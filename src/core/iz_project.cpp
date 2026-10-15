#include "iz_project.h"
#include "iz_assert.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <sys/stat.h>

namespace iz {

using json = nlohmann::json;

Project::Project() {
    ResetToDefaults();
}

void Project::ResetToDefaults() {
    app_name = "MyApplication";
    app_id = "com.example.myapp";
    version = "1.0.0";
    author = "Developer";
    description = "A scriptable cross-platform application.";
    category = "Utility";
    license = "MIT";
    default_prefix = "/usr/bin";

    files.clear();
    dependencies.clear();

    pre_install_js = 
        "// Pre-installation JavaScript hook\n"
        "iz.log('Starting pre-installation tasks for ' + iz.app_name);\n";

    post_install_js = 
        "// Post-installation JavaScript hook\n"
        "iz.log('Configuring application environment...');\n\n"
        "// Example: Setup ~/.config directories or dependency configs (e.g. navi)\n"
        "var userConfigDir = iz.home_dir + '/.config/' + iz.app_name.toLowerCase();\n"
        "iz.mkdir(userConfigDir);\n"
        "iz.write_file(userConfigDir + '/settings.json', '{\"configured\": true}\\n');\n\n"
        "iz.log('Post-installation complete!');\n";

    pre_uninstall_js = 
        "// Pre-uninstallation JavaScript hook\n"
        "iz.log('Preparing to uninstall ' + iz.app_name);\n";

    post_uninstall_js = 
        "// Post-uninstallation JavaScript hook\n"
        "iz.log('Cleanup finished for ' + iz.app_name);\n";

    desktop.create_desktop_entry = true;
    desktop.name = "My Application";
    desktop.exec_cmd = "myapp";
    desktop.icon_source = "";
    desktop.categories = "Utility;Application;";
    desktop.terminal = false;
    desktop.mime_type = "";
}

StatusCode Project::Validate() const {
    if (app_name.empty() || app_name.length() > kMaxNameLen) return StatusCode::ErrInvalidArg;
    if (app_id.empty() || app_id.length() > kMaxNameLen) return StatusCode::ErrInvalidArg;
    if (version.empty() || version.length() > 64) return StatusCode::ErrInvalidArg;
    if (default_prefix.empty() || default_prefix.length() > kMaxPathLen) return StatusCode::ErrInvalidArg;
    if (files.size() > kMaxFiles) return StatusCode::ErrInvalidArg;
    if (dependencies.size() > kMaxDependencies) return StatusCode::ErrInvalidArg;
    return StatusCode::Success;
}

uint64_t Project::CalculateTotalPayloadSize() const {
    uint64_t total = 0;
    for (const auto& f : files) {
        total += f.file_size;
    }
    return total;
}

std::string Project::ToJsonString(int indent) const {
    json j;
    j["app_name"] = app_name;
    j["app_id"] = app_id;
    j["version"] = version;
    j["author"] = author;
    j["description"] = description;
    j["category"] = category;
    j["license"] = license;
    j["default_prefix"] = default_prefix;

    json j_files = json::array();
    for (const auto& f : files) {
        json jf;
        jf["source_path"] = f.source_path;
        jf["target_subpath"] = f.target_subpath;
        jf["permissions"] = f.unix_permissions;
        jf["size"] = f.file_size;
        j_files.push_back(jf);
    }
    j["files"] = j_files;

    json j_deps = json::array();
    for (const auto& d : dependencies) {
        json jd;
        jd["name"] = d.name;
        jd["package_name"] = d.package_name;
        jd["check_command"] = d.check_command;
        jd["is_required"] = d.is_required;
        jd["description"] = d.description;
        j_deps.push_back(jd);
    }
    j["dependencies"] = j_deps;

    j["scripts"] = {
        {"pre_install", pre_install_js},
        {"post_install", post_install_js},
        {"pre_uninstall", pre_uninstall_js},
        {"post_uninstall", post_uninstall_js}
    };

    j["desktop"] = {
        {"create_entry", desktop.create_desktop_entry},
        {"name", desktop.name},
        {"exec", desktop.exec_cmd},
        {"icon", desktop.icon_source},
        {"categories", desktop.categories},
        {"terminal", desktop.terminal},
        {"mime_type", desktop.mime_type}
    };

    return j.dump(indent);
}

StatusCode Project::FromJsonString(const std::string& json_str) {
    try {
        json j = json::parse(json_str);

        if (j.contains("app_name")) app_name = j["app_name"].get<std::string>();
        if (j.contains("app_id")) app_id = j["app_id"].get<std::string>();
        if (j.contains("version")) version = j["version"].get<std::string>();
        if (j.contains("author")) author = j["author"].get<std::string>();
        if (j.contains("description")) description = j["description"].get<std::string>();
        if (j.contains("category")) category = j["category"].get<std::string>();
        if (j.contains("license")) license = j["license"].get<std::string>();
        if (j.contains("default_prefix")) default_prefix = j["default_prefix"].get<std::string>();

        files.clear();
        if (j.contains("files") && j["files"].is_array()) {
            for (const auto& jf : j["files"]) {
                FileEntry fe;
                if (jf.contains("source_path")) fe.source_path = jf["source_path"].get<std::string>();
                if (jf.contains("target_subpath")) fe.target_subpath = jf["target_subpath"].get<std::string>();
                if (jf.contains("permissions")) fe.unix_permissions = jf["permissions"].get<uint32_t>();
                if (jf.contains("size")) fe.file_size = jf["size"].get<uint64_t>();
                files.push_back(fe);
            }
        }

        dependencies.clear();
        if (j.contains("dependencies") && j["dependencies"].is_array()) {
            for (const auto& jd : j["dependencies"]) {
                DependencyEntry de;
                if (jd.contains("name")) de.name = jd["name"].get<std::string>();
                if (jd.contains("package_name")) de.package_name = jd["package_name"].get<std::string>();
                if (jd.contains("check_command")) de.check_command = jd["check_command"].get<std::string>();
                if (jd.contains("is_required")) de.is_required = jd["is_required"].get<bool>();
                if (jd.contains("description")) de.description = jd["description"].get<std::string>();
                dependencies.push_back(de);
            }
        }

        if (j.contains("scripts") && j["scripts"].is_object()) {
            const auto& js = j["scripts"];
            if (js.contains("pre_install")) pre_install_js = js["pre_install"].get<std::string>();
            if (js.contains("post_install")) post_install_js = js["post_install"].get<std::string>();
            if (js.contains("pre_uninstall")) pre_uninstall_js = js["pre_uninstall"].get<std::string>();
            if (js.contains("post_uninstall")) post_uninstall_js = js["post_uninstall"].get<std::string>();
        }

        if (j.contains("desktop") && j["desktop"].is_object()) {
            const auto& jd = j["desktop"];
            if (jd.contains("create_entry")) desktop.create_desktop_entry = jd["create_entry"].get<bool>();
            if (jd.contains("name")) desktop.name = jd["name"].get<std::string>();
            if (jd.contains("exec")) desktop.exec_cmd = jd["exec"].get<std::string>();
            if (jd.contains("icon")) desktop.icon_source = jd["icon"].get<std::string>();
            if (jd.contains("categories")) desktop.categories = jd["categories"].get<std::string>();
            if (jd.contains("terminal")) desktop.terminal = jd["terminal"].get<bool>();
            if (jd.contains("mime_type")) desktop.mime_type = jd["mime_type"].get<std::string>();
        }

        return StatusCode::Success;
    } catch (...) {
        return StatusCode::ErrCorruptData;
    }
}

StatusCode Project::SaveToFile(const std::string& filepath) const {
    IZ_CHECK(!filepath.empty(), StatusCode::ErrInvalidArg);
    std::ofstream out(filepath);
    if (!out.is_open()) {
        return StatusCode::ErrIo;
    }
    out << ToJsonString(2);
    out.close();
    return StatusCode::Success;
}

StatusCode Project::LoadFromFile(const std::string& filepath) {
    IZ_CHECK(!filepath.empty(), StatusCode::ErrInvalidArg);
    std::ifstream in(filepath);
    if (!in.is_open()) {
        return StatusCode::ErrNotFound;
    }
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    in.close();
    return FromJsonString(content);
}

} // namespace iz
