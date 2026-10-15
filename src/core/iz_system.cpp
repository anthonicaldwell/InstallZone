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

static std::string s_cached_sudo_password;

bool System::EnsureSudo(const std::string& reason, std::string* out_password) {
    if (IsRoot()) return true;

    // Check if sudo is cached / passwordless
    if (system("sudo -n true 2>/dev/null") == 0) {
        return true;
    }

    if (!s_cached_sudo_password.empty()) {
        std::string test_cmd = "echo \"" + s_cached_sudo_password + "\" | sudo -S -v 2>/dev/null";
        if (system(test_cmd.c_str()) == 0) {
            if (out_password) *out_password = s_cached_sudo_password;
            return true;
        }
    }

    // Graphical prompt on KDE Plasma (kdialog) or Zenity
    std::string prompt = "Administrator privileges are required to " + reason + ".\nPlease enter your password:";
    std::string cmd;
    if (access("/usr/bin/kdialog", X_OK) == 0) {
        cmd = "kdialog --password \"" + prompt + "\" 2>/dev/null";
    } else if (access("/usr/bin/zenity", X_OK) == 0) {
        cmd = "zenity --password --title=\"InstallZone Authentication\" 2>/dev/null";
    }

    if (!cmd.empty()) {
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe) {
            char buf[256] = {0};
            if (fgets(buf, sizeof(buf), pipe)) {
                pclose(pipe);
                size_t len = std::strlen(buf);
                while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
                    buf[--len] = '\0';
                }
                if (len > 0) {
                    std::string test = "echo \"" + std::string(buf) + "\" | sudo -S -v 2>/dev/null";
                    if (system(test.c_str()) == 0) {
                        s_cached_sudo_password = buf;
                        if (out_password) *out_password = s_cached_sudo_password;
                        return true;
                    }
                }
            } else {
                pclose(pipe);
            }
        }
    }

    // Fallback to tty prompt if running from an interactive terminal
    if (isatty(STDIN_FILENO)) {
        if (system("sudo -v") == 0) return true;
    }

    return false;
}

StatusCode System::RunElevatedCommand(const std::string& cmd, const std::string& sudo_password, std::string* output) {
    std::string full_cmd;
    if (IsRoot()) {
        full_cmd = cmd;
    } else {
        std::string pass = !sudo_password.empty() ? sudo_password : s_cached_sudo_password;
        if (!pass.empty()) {
            full_cmd = "echo \"" + pass + "\" | sudo -S " + cmd;
        } else {
            full_cmd = "sudo " + cmd;
        }
    }

    if (output) {
        full_cmd += " 2>&1";
        FILE* pipe = popen(full_cmd.c_str(), "r");
        if (!pipe) return StatusCode::ErrIo;
        char buf[512];
        output->clear();
        while (fgets(buf, sizeof(buf), pipe)) {
            *output += buf;
        }
        int status = pclose(pipe);
        return (status == 0) ? StatusCode::Success : StatusCode::ErrInternal;
    } else {
        int res = system(full_cmd.c_str());
        return (res == 0) ? StatusCode::Success : StatusCode::ErrInternal;
    }
}

bool System::IsDependencySatisfied(const DependencyEntry& dep) {
    // 1. If explicit check command is provided, test it
    if (!dep.check_command.empty()) {
        std::string full_cmd = dep.check_command + " >/dev/null 2>&1";
        if (system(full_cmd.c_str()) == 0) return true;
    }

    // 2. Check dpkg-query for package_name
    if (!dep.package_name.empty()) {
        std::string dpkg_cmd = "dpkg-query -W -f='${Status}' " + dep.package_name + " 2>/dev/null | grep -q 'ok installed'";
        if (system(dpkg_cmd.c_str()) == 0) return true;

        std::string which_cmd = "which " + dep.package_name + " >/dev/null 2>&1";
        if (system(which_cmd.c_str()) == 0) return true;
    }

    // 3. Fallback checks for common tools & libraries
    std::string lower_name = dep.name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    std::string lower_pkg = dep.package_name;
    std::transform(lower_pkg.begin(), lower_pkg.end(), lower_pkg.begin(), ::tolower);

    // pkg-config
    if (lower_name == "pkgconfig" || lower_name == "pkg-config" || lower_pkg == "pkg-config") {
        if (system("which pkg-config >/dev/null 2>&1") == 0) return true;
    }

    // OpenGL / libgl
    if (lower_name == "opengl" || lower_name == "gl" || lower_pkg == "libgl-dev" || lower_pkg == "libgl1-mesa-dev") {
        if (system("dpkg-query -W -f='${Status}' libgl-dev 2>/dev/null | grep -q 'ok installed' || "
                   "dpkg-query -W -f='${Status}' libgl1-mesa-dev 2>/dev/null | grep -q 'ok installed' || "
                   "test -f /usr/include/GL/gl.h || ldconfig -p 2>/dev/null | grep -q 'libGL\\.so'") == 0) {
            return true;
        }
    }

    // SDL2
    if (lower_name == "sdl2" || lower_pkg == "libsdl2-dev") {
        if (system("pkg-config --exists sdl2 >/dev/null 2>&1 || test -f /usr/include/SDL2/SDL.h") == 0) return true;
    }

    // Check if binary named dep.name exists
    if (!dep.name.empty()) {
        std::string which_name = "which " + dep.name + " >/dev/null 2>&1";
        if (system(which_name.c_str()) == 0) return true;

        std::string which_lower = "which " + lower_name + " >/dev/null 2>&1";
        if (system(which_lower.c_str()) == 0) return true;
    }

    return false;
}

StatusCode System::InstallPackageApt(const std::string& package_name, std::string* output) {
    if (package_name.empty()) return StatusCode::ErrInvalidArg;
    return RunElevatedCommand("apt-get install -y " + package_name, "", output);
}

StatusCode System::InstallDesktopEntry(
    const Project& project,
    const std::string& install_dir,
    bool system_wide,
    const std::string& sudo_password)
{
    if (!project.desktop.create_desktop_entry) return StatusCode::Success;

    std::string home = JsEngine::DetectRealUserHome();
    bool is_system_install = (install_dir.rfind("/usr", 0) == 0 || install_dir.rfind("/opt", 0) == 0 || system_wide);

    std::string apps_dir = is_system_install ? "/usr/share/applications" : (home + "/.local/share/applications");
    std::string icons_dir = is_system_install ? "/usr/share/icons/hicolor/scalable/apps" : (home + "/.local/share/icons/hicolor/scalable/apps");
    std::string pixmaps_dir = is_system_install ? "/usr/share/pixmaps" : (home + "/.local/share/pixmaps");

    if (is_system_install && !IsRoot()) {
        RunElevatedCommand("mkdir -p \"" + apps_dir + "\" \"" + icons_dir + "\" \"" + pixmaps_dir + "\"", sudo_password);
    } else {
        IzSystem("mkdir -p \"" + apps_dir + "\"");
        IzSystem("mkdir -p \"" + icons_dir + "\"");
        IzSystem("mkdir -p \"" + pixmaps_dir + "\"");
    }

    std::string tmp_desktop = "/tmp/" + project.app_id + "_" + std::to_string(getpid()) + ".desktop";
    std::ofstream out(tmp_desktop);
    if (!out.is_open()) return StatusCode::ErrIo;

    // Determine executable target path
    std::string exec_target = project.desktop.exec_cmd;
    if (exec_target.rfind("/", 0) != 0) {
        if (install_dir == "/usr/bin" || install_dir == "/usr/local/bin") {
            exec_target = install_dir + "/" + exec_target;
        } else {
            if (access((install_dir + "/" + exec_target).c_str(), X_OK) == 0) {
                exec_target = install_dir + "/" + exec_target;
            } else if (access((install_dir + "/bin/" + exec_target).c_str(), X_OK) == 0) {
                exec_target = install_dir + "/bin/" + exec_target;
            } else {
                exec_target = install_dir + "/" + exec_target;
            }
        }
    }

    // Determine installed icon path
    std::string icon_val = project.app_name;
    if (!project.desktop.icon_source.empty() && access(project.desktop.icon_source.c_str(), R_OK) == 0) {
        std::string ext = ".png";
        size_t dot = project.desktop.icon_source.find_last_of('.');
        if (dot != std::string::npos) ext = project.desktop.icon_source.substr(dot);

        std::string icon_dst_scalable = icons_dir + "/" + project.app_id + ext;
        std::string icon_dst_scalable_name = icons_dir + "/" + project.app_name + ext;
        std::string icon_dst_pixmap = pixmaps_dir + "/" + project.app_id + ext;
        std::string icon_dst_pixmap_name = pixmaps_dir + "/" + project.app_name + ext;

        if (is_system_install && !IsRoot()) {
            RunElevatedCommand("install -m 0644 \"" + project.desktop.icon_source + "\" \"" + icon_dst_scalable + "\"", sudo_password);
            RunElevatedCommand("install -m 0644 \"" + project.desktop.icon_source + "\" \"" + icon_dst_scalable_name + "\"", sudo_password);
            RunElevatedCommand("install -m 0644 \"" + project.desktop.icon_source + "\" \"" + icon_dst_pixmap + "\"", sudo_password);
            RunElevatedCommand("install -m 0644 \"" + project.desktop.icon_source + "\" \"" + icon_dst_pixmap_name + "\"", sudo_password);
        } else {
            IzSystem("install -m 0644 \"" + project.desktop.icon_source + "\" \"" + icon_dst_scalable + "\" 2>/dev/null");
            IzSystem("install -m 0644 \"" + project.desktop.icon_source + "\" \"" + icon_dst_scalable_name + "\" 2>/dev/null");
            IzSystem("install -m 0644 \"" + project.desktop.icon_source + "\" \"" + icon_dst_pixmap + "\" 2>/dev/null");
            IzSystem("install -m 0644 \"" + project.desktop.icon_source + "\" \"" + icon_dst_pixmap_name + "\" 2>/dev/null");
        }

        icon_val = is_system_install ? project.app_name : icon_dst_scalable;
    }

    out << "[Desktop Entry]\n";
    out << "Type=Application\n";
    out << "Version=1.5\n";
    out << "Name=" << project.desktop.name << "\n";
    out << "GenericName=" << project.app_name << "\n";
    out << "Comment=" << project.description << "\n";
    out << "Exec=\"" << exec_target << "\"\n";
    out << "Icon=" << icon_val << "\n";
    out << "Path=" << install_dir << "\n";
    out << "Terminal=" << (project.desktop.terminal ? "true" : "false") << "\n";
    out << "Categories=" << project.desktop.categories << "\n";
    if (!project.desktop.mime_type.empty()) {
        out << "MimeType=" << project.desktop.mime_type << "\n";
    }
    out << "StartupNotify=true\n";
    out.close();

    std::string desktop_path = apps_dir + "/" + project.app_id + ".desktop";
    if (is_system_install && !IsRoot()) {
        RunElevatedCommand("install -m 0644 \"" + tmp_desktop + "\" \"" + desktop_path + "\"", sudo_password);
        unlink(tmp_desktop.c_str());
        RunElevatedCommand("update-desktop-database \"" + apps_dir + "\" 2>/dev/null || true", sudo_password);
        RunElevatedCommand("gtk-update-icon-cache -f -t /usr/share/icons/hicolor 2>/dev/null || true", sudo_password);
    } else {
        IzSystem("install -m 0644 \"" + tmp_desktop + "\" \"" + desktop_path + "\"");
        unlink(tmp_desktop.c_str());
        IzSystem("update-desktop-database \"" + apps_dir + "\" 2>/dev/null || true");
        IzSystem("gtk-update-icon-cache -f -t \"" + home + "/.local/share/icons/hicolor\" 2>/dev/null || true");
    }

    return StatusCode::Success;
}

StatusCode System::RemoveDesktopEntry(const std::string& app_id, bool system_wide) {
    std::string home = JsEngine::DetectRealUserHome();
    std::string apps_dir = system_wide ? "/usr/share/applications" : (home + "/.local/share/applications");
    std::string icons_dir = system_wide ? "/usr/share/icons/hicolor/scalable/apps" : (home + "/.local/share/icons/hicolor/scalable/apps");
    std::string pixmaps_dir = system_wide ? "/usr/share/pixmaps" : (home + "/.local/share/pixmaps");

    std::string desktop_file = apps_dir + "/" + app_id + ".desktop";
    std::string icon_file = icons_dir + "/" + app_id + ".png";
    std::string pixmap_file = pixmaps_dir + "/" + app_id + ".png";

    if (system_wide && !IsRoot()) {
        RunElevatedCommand("rm -f \"" + desktop_file + "\" \"" + icon_file + "\" \"" + pixmap_file + "\"");
        RunElevatedCommand("update-desktop-database \"" + apps_dir + "\" 2>/dev/null || true");
    } else {
        remove(desktop_file.c_str());
        remove(icon_file.c_str());
        remove(pixmap_file.c_str());
        IzSystem("update-desktop-database 2>/dev/null || true");
    }

    return StatusCode::Success;
}

StatusCode System::GenerateUninstaller(
    const std::string& install_dir,
    const Project& project,
    const std::vector<std::string>& installed_files,
    const std::string& sudo_password)
{
    bool is_system_install = (install_dir.rfind("/usr", 0) == 0 || install_dir.rfind("/opt", 0) == 0);
    std::string stage_dir = "/tmp/iz_uninst_" + std::to_string(getpid());
    mkdir(stage_dir.c_str(), 0755);

    std::string uninst_json_name = project.app_id + "-uninstall.json";
    std::string uninst_sh_name = project.app_id + "-uninstall.sh";

    std::string record_path = stage_dir + "/" + uninst_json_name;
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
    j["system_wide"] = is_system_install;

    std::ofstream out(record_path);
    if (!out.is_open()) return StatusCode::ErrIo;
    out << j.dump(2);
    out.close();

    // Create standalone uninstall.sh script inside install directory
    std::string script_path = stage_dir + "/" + uninst_sh_name;
    std::ofstream script(script_path);
    if (script.is_open()) {
        script << "#!/bin/bash\n";
        script << "# InstallZone generated uninstallation script for " << project.app_name << "\n";
        script << "set -e\n";
        script << "DIR=\"$( cd \"$( dirname \"${BASH_SOURCE[0]}\" )\" >/dev/null 2>&1 && pwd )\"\n";
        script << "if [ \"$(id -u)\" -ne 0 ]; then\n";
        script << "    if command -v sudo >/dev/null 2>&1; then SUDO=\"sudo\"; else SUDO=\"\"; fi\n";
        script << "else\n";
        script << "    SUDO=\"\"\n";
        script << "fi\n";
        script << "if command -v installzone-runner >/dev/null 2>&1; then\n";
        script << "    $SUDO installzone-runner --uninstall \"$DIR/" << uninst_json_name << "\"\n";
        script << "else\n";
        script << "    echo \"Removing installed files...\"\n";
        for (const auto& file : installed_files) {
            script << "    $SUDO rm -f \"" << file << "\"\n";
        }
        script << "    $SUDO rm -f \"$DIR/" << uninst_json_name << "\"\n";
        script << "    $SUDO rm -f \"$DIR/" << uninst_sh_name << "\"\n";
        script << "    echo \"Uninstall complete!\"\n";
        script << "fi\n";
        script.close();
        chmod(script_path.c_str(), 0755);
    }

    if (is_system_install && !IsRoot()) {
        RunElevatedCommand("install -m 0644 \"" + record_path + "\" \"" + install_dir + "/" + uninst_json_name + "\"", sudo_password);
        RunElevatedCommand("install -m 0755 \"" + script_path + "\" \"" + install_dir + "/" + uninst_sh_name + "\"", sudo_password);
    } else {
        IzSystem("install -m 0644 \"" + record_path + "\" \"" + install_dir + "/" + uninst_json_name + "\"");
        IzSystem("install -m 0755 \"" + script_path + "\" \"" + install_dir + "/" + uninst_sh_name + "\"");
    }

    unlink(record_path.c_str());
    unlink(script_path.c_str());
    rmdir(stage_dir.c_str());

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

static std::string RunDialogCommand(const std::string& cmd) {
    FILE* p = popen(cmd.c_str(), "r");
    if (!p) return "";
    char buf[kMaxPathLen];
    std::string result;
    if (fgets(buf, sizeof(buf), p) != nullptr) {
        result = buf;
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
            result.pop_back();
        }
    }
    pclose(p);
    return result;
}

std::string System::OpenFileDialog(const std::string& title, const std::string& start_dir, const std::string& filter) {
    std::string dir = start_dir.empty() ? JsEngine::DetectRealUserHome() : start_dir;

    if (system("which kdialog >/dev/null 2>&1") == 0) {
        std::string cmd = "kdialog --title \"" + title + "\" --getopenfilename \"" + dir + "\"";
        if (!filter.empty()) cmd += " \"" + filter + "\"";
        return RunDialogCommand(cmd);
    }
    if (system("which zenity >/dev/null 2>&1") == 0) {
        std::string cmd = "zenity --file-selection --title=\"" + title + "\" --filename=\"" + dir + "/\"";
        return RunDialogCommand(cmd);
    }
    return "";
}

std::string System::OpenDirectoryDialog(const std::string& title, const std::string& start_dir) {
    std::string dir = start_dir.empty() ? JsEngine::DetectRealUserHome() : start_dir;

    if (system("which kdialog >/dev/null 2>&1") == 0) {
        std::string cmd = "kdialog --title \"" + title + "\" --getexistingdirectory \"" + dir + "\"";
        return RunDialogCommand(cmd);
    }
    if (system("which zenity >/dev/null 2>&1") == 0) {
        std::string cmd = "zenity --file-selection --directory --title=\"" + title + "\" --filename=\"" + dir + "/\"";
        return RunDialogCommand(cmd);
    }
    return "";
}

std::string System::SaveFileDialog(const std::string& title, const std::string& start_dir, const std::string& filter) {
    std::string dir = start_dir.empty() ? JsEngine::DetectRealUserHome() : start_dir;

    if (system("which kdialog >/dev/null 2>&1") == 0) {
        std::string cmd = "kdialog --title \"" + title + "\" --getsavefilename \"" + dir + "\"";
        if (!filter.empty()) cmd += " \"" + filter + "\"";
        return RunDialogCommand(cmd);
    }
    if (system("which zenity >/dev/null 2>&1") == 0) {
        std::string cmd = "zenity --file-selection --save --confirm-overwrite --title=\"" + title + "\" --filename=\"" + dir + "/\"";
        return RunDialogCommand(cmd);
    }
    return "";
}

} // namespace iz
