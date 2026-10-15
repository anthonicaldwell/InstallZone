#pragma once

#include "jpl_defs.h"
#include <string>
#include <vector>

namespace iz {

struct FileEntry {
    std::string source_path;
    std::string target_subpath;
    uint32_t unix_permissions{0755};
    uint64_t file_size{0};
};

struct DependencyEntry {
    std::string name;
    std::string package_name;
    std::string check_command;
    bool is_required{true};
    std::string description;
};

struct DesktopConfig {
    bool create_desktop_entry{true};
    std::string name;
    std::string exec_cmd;
    std::string icon_source;
    std::string categories{"Utility;"};
    bool terminal{false};
    std::string mime_type;
};

class Project {
public:
    std::string app_name{"MyApplication"};
    std::string app_id{"com.example.myapp"};
    std::string version{"1.0.0"};
    std::string author{"Developer"};
    std::string description{"A scriptable cross-platform application."};
    std::string category{"Utility"};
    std::string license{"MIT"};
    std::string default_prefix{"/opt/myapp"};

    std::vector<FileEntry> files;
    std::vector<DependencyEntry> dependencies;

    // JavaScript Hooks
    std::string pre_install_js;
    std::string post_install_js;
    std::string pre_uninstall_js;
    std::string post_uninstall_js;

    DesktopConfig desktop;

    Project();
    ~Project() = default;

    void ResetToDefaults();
    StatusCode Validate() const;
    StatusCode SaveToFile(const std::string& filepath) const;
    StatusCode LoadFromFile(const std::string& filepath);

    std::string ToJsonString(int indent = 2) const;
    StatusCode FromJsonString(const std::string& json_str);

    // Helpers
    uint64_t CalculateTotalPayloadSize() const;
};

} // namespace iz
