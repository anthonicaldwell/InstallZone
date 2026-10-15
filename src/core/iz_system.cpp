#include "iz_system.h"
#include "iz_js_engine.h"
#include "iz_assert.h"
#include "nlohmann/json.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace iz {

using json = nlohmann::json;

bool System::IsRoot() {
    return (geteuid() == 0);
}

bool System::PathRequiresElevation(const std::string& path) {
    if (IsRoot()) return false;
    if (path.rfind("/usr", 0) == 0 ||
        path.rfind("/opt", 0) == 0 ||
        path.rfind("/etc", 0) == 0 ||
        path.rfind("/var", 0) == 0 ||
        path.rfind("/bin", 0) == 0 ||
        path.rfind("/sbin", 0) == 0) {
        return true;
    }

    // Check parent directory writability
    size_t last_slash = path.find_last_of('/');
    std::string test_dir = (last_slash != std::string::npos && last_slash > 0) ? path.substr(0, last_slash) : path;
    while (!test_dir.empty()) {
        if (access(test_dir.c_str(), F_OK) == 0) {
            return (access(test_dir.c_str(), W_OK) != 0);
        }
        size_t prev_slash = test_dir.find_last_of('/');
        if (prev_slash == std::string::npos || prev_slash == 0) break;
        test_dir = test_dir.substr(0, prev_slash);
    }
    return false;
}

StatusCode System::ElevateProcess(int argc, char** argv) {
    std::string cmd = "pkexec env ";
    const char* display = std::getenv("DISPLAY");
    const char* xauth = std::getenv("XAUTHORITY");
    const char* wayland = std::getenv("WAYLAND_DISPLAY");
    const char* xdg_runtime = std::getenv("XDG_RUNTIME_DIR");

    if (display) cmd += "DISPLAY=\"" + std::string(display) + "\" ";
    if (xauth) cmd += "XAUTHORITY=\"" + std::string(xauth) + "\" ";
    if (wayland) cmd += "WAYLAND_DISPLAY=\"" + std::string(wayland) + "\" ";
    if (xdg_runtime) cmd += "XDG_RUNTIME_DIR=\"" + std::string(xdg_runtime) + "\" ";

    cmd += "\"/proc/self/exe\"";
    for (int i = 1; i < argc; ++i) {
        cmd += " \"";
        cmd += argv[i];
        cmd += "\"";
    }

    int res = system(cmd.c_str());
    if (res == 0) {
        exit(0); // Successfully ran elevated
    }
    return StatusCode::ErrPermission;
}

bool System::IsDependencySatisfied(const DependencyEntry& dep) {
    if (!dep.check_command.empty()) {
        std::string full_cmd = dep.check_command + " >/dev/null 2>&1";
        return (system(full_cmd.c_str()) == 0);
    }
    if (!dep.package_name.empty()) {
        std::string dpkg_cmd = "dpkg-query -W -f='${Status}' " + dep.package_name + " 2>/dev/null | grep -q 'ok installed'";
        if (system(dpkg_cmd.c_str()) == 0) return true;

        std::string which_cmd = "which " + dep.package_name + " >/dev/null 2>&1";
        return (system(which_cmd.c_str()) == 0);
    }
    return true;
}

StatusCode System::InstallPackageApt(const std::string& package_name, std::string* output) {
    if (package_name.empty()) return StatusCode::ErrInvalidArg;
    std::string cmd = (IsRoot() ? "" : "pkexec ") + std::string("apt-get install -y ") + package_name;
    int res = system(cmd.c_str());
    return (res == 0) ? StatusCode::Success : StatusCode::ErrInternal;
}

StatusCode System::InstallDesktopEntry(const Project& project, const std::string& install_dir, bool system_wide) {
    if (!project.desktop.create_desktop_entry) return StatusCode::Success;

    std::string home = JsEngine::DetectRealUserHome();
    std::string apps_dir = system_wide ? "/usr/local/share/applications" : (home + "/.local/share/applications");
    std::string icons_dir = system_wide ? "/usr/local/share/icons/hicolor/scalable/apps" : (home + "/.local/share/icons/hicolor/scalable/apps");

    system(("mkdir -p \"" + apps_dir + "\"").c_str());
    system(("mkdir -p \"" + icons_dir + "\"").c_str());

    std::string desktop_path = apps_dir + "/" + project.app_id + ".desktop";
    std::ofstream out(desktop_path);
    if (!out.is_open()) return StatusCode::ErrIo;

    std::string exec_target = project.desktop.exec_cmd;
    if (exec_target.rfind("/", 0) != 0) {
        exec_target = install_dir + "/" + exec_target;
    }

    std::string icon_name = project.app_id;
    if (!project.desktop.icon_source.empty()) {
        std::string icon_dst = icons_dir + "/" + project.app_id + ".svg";
        std::string cp_cmd = "cp \"" + project.desktop.icon_source + "\" \"" + icon_dst + "\" 2>/dev/null";
        system(cp_cmd.c_str());
    }

    out << "[Desktop Entry]\n";
    out << "Type=Application\n";
    out << "Version=1.5\n";
    out << "Name=" << project.desktop.name << "\n";
    out << "GenericName=" << project.app_name << "\n";
    out << "Comment=" << project.description << "\n";
    out << "Exec=\"" << exec_target << "\"\n";
    out << "Icon=" << icon_name << "\n";
    out << "Terminal=" << (project.desktop.terminal ? "true" : "false") << "\n";
    out << "Categories=" << project.desktop.categories << "\n";
    if (!project.desktop.mime_type.empty()) {
        out << "MimeType=" << project.desktop.mime_type << "\n";
    }
    out << "StartupNotify=true\n";
    out.close();

    chmod(desktop_path.c_str(), 0644);
    system("update-desktop-database 2>/dev/null");
    system("gtk-update-icon-cache 2>/dev/null");

    return StatusCode::Success;
}

StatusCode System::RemoveDesktopEntry(const std::string& app_id, bool system_wide) {
    std::string home = JsEngine::DetectRealUserHome();
    std::string apps_dir = system_wide ? "/usr/local/share/applications" : (home + "/.local/share/applications");
    std::string icons_dir = system_wide ? "/usr/local/share/icons/hicolor/scalable/apps" : (home + "/.local/share/icons/hicolor/scalable/apps");

    std::string desktop_file = apps_dir + "/" + app_id + ".desktop";
    std::string icon_file = icons_dir + "/" + app_id + ".svg";

    remove(desktop_file.c_str());
    remove(icon_file.c_str());

    system("update-desktop-database 2>/dev/null");
    return StatusCode::Success;
}

StatusCode System::GenerateUninstaller(
    const std::string& install_dir,
    const Project& project,
    const std::vector<std::string>& installed_files)
{
    std::string record_path = install_dir + "/installzone-uninstall.json";
    json j;
    j["app_name"] = project.app_name;
    j["app_id"] = project.app_id;
    j["version"] = project.version;
    j["install_dir"] = install_dir;
    j["installed_files"] = installed_files;
    j["scripts"] = {
        {"pre_uninstall", project.pre_uninstall_js},
        {"post_uninstall", project.post_uninstall_js}
    };
    j["system_wide"] = IsRoot();

    std::ofstream out(record_path);
    if (!out.is_open()) return StatusCode::ErrIo;
    out << j.dump(2);
    out.close();

    // Create standalone uninstall.sh script inside install directory
    std::string script_path = install_dir + "/uninstall.sh";
    std::ofstream script(script_path);
    if (script.is_open()) {
        script << "#!/bin/bash\n";
        script << "# InstallZone generated uninstallation script\n";
        script << "set -e\n";
        script << "DIR=\"$( cd \"$( dirname \"${BASH_SOURCE[0]}\" )\" >/dev/null 2>&1 && pwd )\"\n";
        script << "if [ \"$(id -u)\" -ne 0 ] && [ -w \"/usr/local\" ]; then\n";
        script << "    echo \"Running uninstaller...\"\n";
        script << "fi\n";
        script << "if command -v installzone-runner >/dev/null 2>&1; then\n";
        script << "    installzone-runner --uninstall \"$DIR/installzone-uninstall.json\"\n";
        script << "else\n";
        script << "    echo \"Removing installed files...\"\n";
        for (const auto& file : installed_files) {
            script << "    rm -f \"" << file << "\"\n";
        }
        script << "    rm -f \"$DIR/installzone-uninstall.json\"\n";
        script << "    rm -f \"$DIR/uninstall.sh\"\n";
        script << "    echo \"Uninstall complete!\"\n";
        script << "fi\n";
        script.close();
        chmod(script_path.c_str(), 0755);
    }

    return StatusCode::Success;
}

StatusCode System::PerformUninstall(const std::string& uninstall_json_path, LogCallback log_cb) {
    std::ifstream in(uninstall_json_path);
    if (!in.is_open()) {
        if (log_cb) log_cb("Error: Uninstall manifest not found: " + uninstall_json_path);
        return StatusCode::ErrNotFound;
    }

    json j;
    try {
        in >> j;
    } catch (...) {
        return StatusCode::ErrCorruptData;
    }

    std::string app_name = j.value("app_name", "Application");
    std::string app_id = j.value("app_id", "");
    std::string install_dir = j.value("install_dir", "");
    bool system_wide = j.value("system_wide", false);

    if (log_cb) log_cb("Uninstalling: " + app_name);

    // Run Pre-Uninstall JS
    if (j.contains("scripts") && j["scripts"].contains("pre_uninstall")) {
        std::string pre_js = j["scripts"]["pre_uninstall"].get<std::string>();
        if (!pre_js.empty()) {
            if (log_cb) log_cb("Executing Pre-Uninstall JavaScript hook...");
            JsEngine engine;
            JsContextConfig cfg;
            cfg.app_name = app_name;
            cfg.install_dir = install_dir;
            cfg.is_uninstall = true;
            cfg.log_callback = log_cb;
            engine.Initialize(cfg);
            engine.Execute(pre_js);
        }
    }

    // Remove desktop entry & icons
    if (!app_id.empty()) {
        if (log_cb) log_cb("Removing desktop entry and application icons...");
        RemoveDesktopEntry(app_id, system_wide);
    }

    // Remove files
    if (j.contains("installed_files") && j["installed_files"].is_array()) {
        for (const auto& jf : j["installed_files"]) {
            std::string file_path = jf.get<std::string>();
            if (log_cb) log_cb("Deleting: " + file_path);
            remove(file_path.c_str());
        }
    }

    // Run Post-Uninstall JS
    if (j.contains("scripts") && j["scripts"].contains("post_uninstall")) {
        std::string post_js = j["scripts"]["post_uninstall"].get<std::string>();
        if (!post_js.empty()) {
            if (log_cb) log_cb("Executing Post-Uninstall JavaScript hook...");
            JsEngine engine;
            JsContextConfig cfg;
            cfg.app_name = app_name;
            cfg.install_dir = install_dir;
            cfg.is_uninstall = true;
            cfg.log_callback = log_cb;
            engine.Initialize(cfg);
            engine.Execute(post_js);
        }
    }

    // Delete uninstall manifest and folder
    remove(uninstall_json_path.c_str());
    if (!install_dir.empty()) {
        std::string un_sh = install_dir + "/uninstall.sh";
        remove(un_sh.c_str());
        rmdir(install_dir.c_str());
    }

    if (log_cb) log_cb("Uninstallation completed successfully.");
    return StatusCode::Success;
}

} // namespace iz
