#pragma once

#include "jpl_defs.h"
#include "iz_project.h"
#include <string>
#include <vector>
#include <functional>

namespace iz {

using LogCallback = std::function<void(const std::string& message)>;

class System {
public:
    static bool IsRoot();
    static bool PathRequiresElevation(const std::string& path);
    static StatusCode ElevateProcess(int argc, char** argv);

    static bool EnsureSudo(const std::string& reason, std::string* out_password = nullptr);
    static StatusCode RunElevatedCommand(const std::string& cmd, const std::string& sudo_password = "", std::string* output = nullptr);

    static bool DetectExistingInstallation(
        const Project& project,
        std::string* out_uninstall_manifest = nullptr,
        std::string* out_installed_prefix = nullptr
    );

    // Dependency verification
    static bool IsDependencySatisfied(const DependencyEntry& dep);
    static StatusCode InstallPackageApt(const std::string& package_name, std::string* output = nullptr);

    // Desktop integration
    static StatusCode InstallDesktopEntry(
        const Project& project,
        const std::string& install_dir,
        bool system_wide,
        const std::string& sudo_password = ""
    );

    static StatusCode RemoveDesktopEntry(
        const std::string& app_id,
        bool system_wide,
        const std::string& sudo_password = ""
    );

    // Uninstaller generation
    static StatusCode GenerateUninstaller(
        const std::string& install_dir,
        const Project& project,
        const std::vector<std::string>& installed_files,
        const std::string& sudo_password = ""
    );

    static StatusCode PerformUninstall(
        const std::string& uninstall_json_path,
        LogCallback log_cb = nullptr,
        const std::string& sudo_password = ""
    );

    // Native Desktop File & Directory Dialogs (KDE Plasma kdialog / Zenity)
    static std::string OpenFileDialog(const std::string& title, const std::string& start_dir = "", const std::string& filter = "");
    static std::string OpenDirectoryDialog(const std::string& title, const std::string& start_dir = "");
    static std::string SaveFileDialog(const std::string& title, const std::string& start_dir = "", const std::string& filter = "");
};

} // namespace iz
