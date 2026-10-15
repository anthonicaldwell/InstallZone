#pragma once

#include "jpl_defs.h"
#include "iz_project.h"
#include <string>
#include <vector>

namespace iz {

struct CMakeScanResult {
    bool success{false};
    std::string project_name;
    std::string version;
    std::string description;
    std::string homepage_url;
    std::vector<std::string> executable_targets;
    std::vector<FileEntry> install_files;
    std::vector<DependencyEntry> dependencies;
    std::vector<std::string> scan_log;
};

class CMakeScanner {
public:
    /**
     * @brief Scans a CMakeLists.txt file and extracts project metadata, executables, install targets, and dependencies.
     */
    static CMakeScanResult ScanFile(const std::string& cmake_file_path);

    /**
     * @brief Scans CMakeLists.txt and applies discovered information directly to a Project object.
     */
    static StatusCode ApplyToProject(const std::string& cmake_file_path, Project& project, std::vector<std::string>* log_out = nullptr);

    /**
     * @brief Maps CMake package name to standard Debian/Ubuntu Apt package name.
     */
    static std::string MapCMakePackageToApt(const std::string& cmake_pkg);
};

} // namespace iz
