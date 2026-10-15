#include "iz_ui_common.h"
#include "iz_project.h"
#include "iz_archive.h"
#include "iz_system.h"
#include "iz_js_engine.h"
#include "iz_i18n.h"
#include "iz_audio.h"
#include "iz_assert.h"

#include "imgui.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <cstdlib>
#include <cmath>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

namespace iz {

enum class WizardPage {
    Welcome = 0,
    Destination,
    Dependencies,
    Installing,
    Completed
};

class RunnerApp {
public:
    RunnerApp(int argc, char** argv)
        : m_argc(argc), m_argv(argv)
    {
        ParseCommandLine();
        DetectPayload();
    }

    ~RunnerApp() {
        if (!m_temp_assets_dir.empty()) {
            std::string rm_cmd = "rm -rf \"" + m_temp_assets_dir + "\" 2>/dev/null";
            IzSystem(rm_cmd);
        }
    }

    void Run() {
        I18n::Initialize();

        if (m_is_uninstall_mode) {
            RunUninstall();
            return;
        }

        // Initialize and play background chiptune (default 50% volume)
        AudioManager::Initialize();

        std::string chiptune_override = "";
        if (!m_temp_assets_dir.empty()) {
            std::string candidate = m_temp_assets_dir + "/chiptune.wav";
            if (access(candidate.c_str(), R_OK) == 0) {
                chiptune_override = candidate;
            }
        }
        AudioManager::StartDefaultChiptune(chiptune_override);

        std::string win_title = std::string(_("app_title_runner")) + " - " + m_project.app_name;
        if (m_ui.Initialize(win_title.c_str(), 760, 780) != StatusCode::Success) {
            std::cerr << "Failed to initialize Runner UI.\n";
            AudioManager::Shutdown();
            return;
        }

        if (!m_temp_assets_dir.empty()) {
            std::string banner_override = m_temp_assets_dir + "/banner.jpg";
            if (access(banner_override.c_str(), R_OK) == 0) {
                m_ui.LoadBannerTexture(banner_override.c_str());
            }
        }

        while (m_ui.ProcessEvents()) {
            m_ui.BeginFrame();
            RenderUI();
            m_ui.EndFrame();
        }

        if (m_worker_thread.joinable()) {
            m_worker_thread.join();
        }
        AudioManager::Shutdown();
        m_ui.Shutdown();
    }

private:
    int m_argc;
    char** m_argv;
    UIContext m_ui;
    Project m_project;

    std::string m_exe_path;
    std::string m_temp_assets_dir;
    bool m_has_embedded_payload{false};
    bool m_is_uninstall_mode{false};
    std::string m_uninstall_manifest_path;

    WizardPage m_current_page{WizardPage::Welcome};
    char m_buf_destination[kMaxPathLen] = {0};
    bool m_create_desktop_entry{true};

    WorkerBridge m_worker_bridge;
    std::thread m_worker_thread;
    std::vector<std::string> m_installed_files;

    void ParseCommandLine() {
        m_exe_path = "/proc/self/exe";
        for (int i = 1; i < m_argc; ++i) {
            std::string arg = m_argv[i];
            if (arg == "--uninstall") {
                m_is_uninstall_mode = true;
                if (i + 1 < m_argc) {
                    m_uninstall_manifest_path = m_argv[++i];
                }
            } else if (arg == "--prefix" && i + 1 < m_argc) {
                m_project.default_prefix = m_argv[++i];
            } else if (arg == "--project" && i + 1 < m_argc) {
                m_project.LoadFromFile(m_argv[++i]);
            }
        }
    }

    void DetectPayload() {
        if (Archive::HasEmbeddedPayload(m_exe_path)) {
            m_has_embedded_payload = true;
            Archive::ReadEmbeddedManifest(m_exe_path, m_project);

            // Extract internal assets (banner, chiptune, icon) to per-process temp directory
            m_temp_assets_dir = "/tmp/iz_runner_" + std::to_string(getpid());
            Archive::ExtractInternalAssets(m_exe_path, m_temp_assets_dir);

            // Check if internal app icon was extracted
            const char* icon_exts[] = { ".png", ".svg", ".ico", ".xpm" };
            for (const char* ext : icon_exts) {
                std::string icon_candidate = m_temp_assets_dir + "/app_icon" + std::string(ext);
                if (access(icon_candidate.c_str(), R_OK) == 0) {
                    m_project.desktop.icon_source = icon_candidate;
                    break;
                }
            }
        } else {
            // Check if project manifest exists in current working directory
            if (access("manifest.json", R_OK) == 0) {
                m_project.LoadFromFile("manifest.json");
            }
        }
        std::snprintf(m_buf_destination, sizeof(m_buf_destination), "%s", m_project.default_prefix.c_str());
        m_create_desktop_entry = m_project.desktop.create_desktop_entry;
    }

    void RunUninstall() {
        if (m_uninstall_manifest_path.empty()) {
            m_uninstall_manifest_path = m_project.default_prefix + "/installzone-uninstall.json";
        }
        System::PerformUninstall(m_uninstall_manifest_path, [](const std::string& msg) {
            std::cout << msg << "\n";
        });
    }

    void RenderUI() {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::Begin("RunnerMain", nullptr, window_flags);
        ImGui::PopStyleVar();

        // Top bar with Language Selector and Chiptune Music Toggle
        float combo_width = 150.0f;
        ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "%s %s", m_project.app_name.c_str(), m_project.version.c_str());
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
        bool music_enabled = !AudioManager::IsMuted();
        if (ImGui::Checkbox("🎵 Music", &music_enabled)) {
            AudioManager::SetMuted(!music_enabled);
        }
        ImGui::SameLine();
        int vol_pct = static_cast<int>(std::round(AudioManager::GetVolume() * 100.0f));
        ImGui::SetNextItemWidth(90.0f);
        if (ImGui::SliderInt("##MusicVol", &vol_pct, 0, 100, "Vol: %d%%")) {
            AudioManager::SetVolume(static_cast<float>(vol_pct) / 100.0f);
        }
        ImGui::SameLine(ImGui::GetWindowWidth() - combo_width - 15.0f);
        m_ui.RenderLanguageSelector();

        ImGui::Separator();
        ImGui::Spacing();

        // Top banner with early 2000's GPU box art retro aesthetic (aspect ratio preserved)
        m_ui.RenderBanner(0, 200.0f, true);
        ImGui::Spacing();

        switch (m_current_page) {
            case WizardPage::Welcome:      RenderWelcomePage(); break;
            case WizardPage::Destination:  RenderDestinationPage(); break;
            case WizardPage::Dependencies: RenderDependenciesPage(); break;
            case WizardPage::Installing:   RenderInstallingPage(); break;
            case WizardPage::Completed:    RenderCompletedPage(); break;
        }

        ImGui::End();
    }

    void RenderWelcomePage() {
        ImGui::TextColored(ImVec4(0.9f, 0.95f, 1.0f, 1.0f), "%s", _("lbl_welcome"));
        ImGui::Spacing();
        ImGui::Text("Application: %s", m_project.app_name.c_str());
        ImGui::Text("Version: %s", m_project.version.c_str());
        ImGui::Text("Author: %s", m_project.author.c_str());
        ImGui::Text("License: %s", m_project.license.c_str());
        ImGui::Spacing();

        ImGui::Text("Description:");
        if (ImGui::BeginChild("DescBox", ImVec2(-1, 200), true)) {
            ImGui::TextWrapped("%s", m_project.description.c_str());
        }
        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button(_("btn_cancel"), ImVec2(100, 32))) {
            exit(0);
        }
        ImGui::SameLine(ImGui::GetWindowWidth() - 120.0f);
        if (ImGui::Button(_("btn_next"), ImVec2(100, 32))) {
            m_current_page = WizardPage::Destination;
        }
    }

    void RenderDestinationPage() {
        ImGui::TextColored(ImVec4(0.9f, 0.95f, 1.0f, 1.0f), "%s", _("lbl_destination"));
        ImGui::Spacing();

        float btn_w = 90.0f;
        float input_w = ImGui::GetContentRegionAvail().x - btn_w - 10.0f;
        ImGui::SetNextItemWidth(input_w);
        ImGui::InputText("##DestPath", m_buf_destination, sizeof(m_buf_destination));
        ImGui::SameLine();
        if (ImGui::Button("Browse...##DestBrowse", ImVec2(btn_w, 0))) {
            std::string chosen = System::OpenDirectoryDialog("Select Installation Directory", m_buf_destination);
            if (!chosen.empty()) {
                std::snprintf(m_buf_destination, sizeof(m_buf_destination), "%s", chosen.c_str());
            }
        }
        ImGui::Spacing();

        // Disk space info
        struct statvfs vfs;
        if (statvfs("/", &vfs) == 0) {
            double free_gb = (static_cast<double>(vfs.f_bavail) * vfs.f_frsize) / (1024.0 * 1024.0 * 1024.0);
            double req_mb = static_cast<double>(m_project.CalculateTotalPayloadSize()) / (1024.0 * 1024.0);
            ImGui::Text("Required Space: %.2f MB", req_mb);
            ImGui::Text("Available Disk Space: %.2f GB", free_gb);
        }

        ImGui::Spacing();
        ImGui::Checkbox("Create Desktop Shortcut & Application Menu Entry", &m_create_desktop_entry);

        ImGui::Spacing();
        bool needs_elevation = System::PathRequiresElevation(m_buf_destination);
        if (needs_elevation) {
            ImGui::TextDisabled("Note: Destination path requires administrator privileges (will prompt on Install).");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button(_("btn_back"), ImVec2(100, 32))) {
            m_current_page = WizardPage::Welcome;
        }
        ImGui::SameLine(ImGui::GetWindowWidth() - 120.0f);
        if (ImGui::Button(_("btn_next"), ImVec2(100, 32))) {
            m_project.default_prefix = m_buf_destination;
            m_project.desktop.create_desktop_entry = m_create_desktop_entry;
            m_current_page = WizardPage::Dependencies;
        }
    }

    void RenderDependenciesPage() {
        ImGui::TextColored(ImVec4(0.9f, 0.95f, 1.0f, 1.0f), "%s", _("lbl_deps_checking"));
        ImGui::Spacing();

        if (m_project.dependencies.empty()) {
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "No external dependencies required.");
        } else {
            for (const auto& dep : m_project.dependencies) {
                bool ok = System::IsDependencySatisfied(dep);
                if (ok) {
                    ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "[OK] %s (%s) - Installed", dep.name.c_str(), dep.package_name.c_str());
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), "[PENDING] %s (%s) - Will be installed via apt", dep.name.c_str(), dep.package_name.c_str());
                }
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button(_("btn_back"), ImVec2(100, 32))) {
            m_current_page = WizardPage::Destination;
        }
        ImGui::SameLine(ImGui::GetWindowWidth() - 140.0f);

        if (ImGui::Button("Install Now", ImVec2(120, 32))) {
            m_current_page = WizardPage::Installing;
            StartInstallation();
        }
    }

    void RenderInstallingPage() {
        ImGui::TextColored(ImVec4(0.9f, 0.95f, 1.0f, 1.0f), "%s", _("lbl_installing"));
        ImGui::Spacing();

        float p = m_worker_bridge.progress.load();
        ImGui::ProgressBar(p, ImVec2(-1, 24));
        ImGui::Text("%s %s", _("lbl_status"), m_worker_bridge.GetCurrentOperation().c_str());

        ImGui::Spacing();
        std::vector<std::string> logs;
        m_worker_bridge.CopyLogs(logs);
        if (ImGui::BeginChild("InstallLogBox", ImVec2(-1, 260), true)) {
            for (const auto& l : logs) {
                ImGui::TextUnformatted(l.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
        }
        ImGui::EndChild();

        if (m_worker_bridge.is_finished.load()) {
            if (m_worker_bridge.success.load()) {
                m_current_page = WizardPage::Completed;
            } else {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Installation Error: %s", m_worker_bridge.GetErrorMessage().c_str());
            }
        }
    }

    void RenderCompletedPage() {
        ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "%s", _("lbl_installed"));
        ImGui::Spacing();
        ImGui::Text("%s", _("lbl_install_complete"));
        ImGui::Spacing();
        ImGui::Text("Installed to: %s", m_project.default_prefix.c_str());
        ImGui::Text("Total Files: %zu", m_installed_files.size());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        std::string exec_target = m_project.desktop.exec_cmd;
        if (!exec_target.empty()) {
            if (ImGui::Button(_("lbl_launch_app"), ImVec2(160, 36))) {
                if (exec_target.rfind("/", 0) != 0) {
                    exec_target = m_project.default_prefix + "/" + exec_target;
                }
                std::string launch_cmd = "\"" + exec_target + "\" &";
                IzSystem(launch_cmd);
                exit(0);
            }
            ImGui::SameLine();
        }

        if (ImGui::Button(_("btn_finish"), ImVec2(120, 36))) {
            exit(0);
        }
    }

    void StartInstallation() {
        if (m_worker_thread.joinable()) {
            m_worker_thread.join();
        }
        m_worker_bridge.Reset();
        m_worker_bridge.is_running.store(true);
        m_worker_bridge.SetOperation("Initializing installation...");

        m_worker_thread = std::thread([this]() {
            // Step 0: Check missing dependencies & elevation requirement
            std::vector<std::string> missing_pkgs;
            for (const auto& dep : m_project.dependencies) {
                if (!System::IsDependencySatisfied(dep) && !dep.package_name.empty()) {
                    missing_pkgs.push_back(dep.package_name);
                }
            }

            bool is_system_install = (m_project.default_prefix.rfind("/usr", 0) == 0 ||
                                      m_project.default_prefix.rfind("/opt", 0) == 0 ||
                                      System::PathRequiresElevation(m_project.default_prefix));

            bool needs_sudo = (!System::IsRoot()) && (is_system_install || !missing_pkgs.empty() || m_create_desktop_entry);

            std::string sudo_pass;
            if (needs_sudo) {
                m_worker_bridge.SetOperation("Requesting administrator privileges...");
                m_worker_bridge.AppendLog("Prompting for administrator elevation (sudo)...");

                std::string reason = "install " + m_project.app_name;
                if (!missing_pkgs.empty()) reason += " and missing system dependencies";

                if (!System::EnsureSudo(reason, &sudo_pass)) {
                    m_worker_bridge.Finish(false, "Administrator authentication was cancelled or failed.");
                    return;
                }
                m_worker_bridge.AppendLog("Administrator privileges confirmed.");
            }

            // Step 1: Install any missing system dependencies via Apt
            if (!missing_pkgs.empty()) {
                m_worker_bridge.SetOperation("Installing system dependencies...");
                for (const auto& pkg : missing_pkgs) {
                    m_worker_bridge.AppendLog("Installing dependency via apt: " + pkg + "...");
                    std::string apt_cmd = "apt-get install -y " + pkg;
                    std::string apt_out;
                    StatusCode ares = System::RunElevatedCommand(apt_cmd, sudo_pass, &apt_out);
                    if (ares != StatusCode::Success) {
                        m_worker_bridge.AppendLog("Warning: apt-get install " + pkg + " failed: " + apt_out);
                    } else {
                        m_worker_bridge.AppendLog("Successfully installed " + pkg + ".");
                    }
                }
            }

            // Step 2: Run Pre-Install JS Hook
            if (!m_project.pre_install_js.empty()) {
                m_worker_bridge.SetOperation("Running Pre-Install Script...");
                m_worker_bridge.AppendLog("Executing Pre-Install JavaScript hook...");

                JsEngine engine;
                JsContextConfig cfg;
                cfg.app_name = m_project.app_name;
                cfg.version = m_project.version;
                cfg.install_dir = m_project.default_prefix;
                cfg.log_callback = [this](const std::string& msg) {
                    m_worker_bridge.AppendLog(msg);
                };

                engine.Initialize(cfg);
                std::string err;
                if (engine.Execute(m_project.pre_install_js, &err) != StatusCode::Success) {
                    m_worker_bridge.Finish(false, "Pre-Install JS failed: " + err);
                    return;
                }
            }

            // Step 3: Extract payload files
            m_worker_bridge.SetOperation("Extracting application files...");

            // If system install as non-root, extract to temporary staging folder and copy with sudo
            std::string stage_dir = "";
            std::string extract_target = m_project.default_prefix;
            bool use_staging = (is_system_install && !System::IsRoot());

            if (use_staging) {
                stage_dir = "/tmp/iz_stage_" + std::to_string(getpid());
                extract_target = stage_dir;
            }

            StatusCode res = Archive::ExtractEmbeddedPayload(
                m_exe_path,
                extract_target,
                m_installed_files,
                [this](float p, const std::string& item) {
                    m_worker_bridge.SetProgress(p * 0.7f);
                    m_worker_bridge.SetOperation(item);
                },
                [this](const std::string& msg) {
                    m_worker_bridge.AppendLog(msg);
                }
            );

            if (res != StatusCode::Success && m_has_embedded_payload) {
                m_worker_bridge.Finish(false, "File extraction failed: " + std::string(StatusCodeToString(res)));
                return;
            }

            if (use_staging) {
                m_worker_bridge.SetOperation("Installing files to destination...");
                m_worker_bridge.AppendLog("Copying files to " + m_project.default_prefix + " with administrator privileges...");

                System::RunElevatedCommand("mkdir -p \"" + m_project.default_prefix + "\"", sudo_pass);

                bool prefix_is_bin = (m_project.default_prefix.length() >= 4 &&
                                      m_project.default_prefix.rfind("/bin") == m_project.default_prefix.length() - 4);
                std::string stage_bin = stage_dir + "/bin";
                struct stat st_bin;
                bool stage_has_bin = (stat(stage_bin.c_str(), &st_bin) == 0 && S_ISDIR(st_bin.st_mode));

                if (prefix_is_bin && stage_has_bin) {
                    // Copy files from stage_dir/bin/* directly into prefix (e.g. /usr/bin)
                    System::RunElevatedCommand("cp -a \"" + stage_bin + "/\". \"" + m_project.default_prefix + "/\"", sudo_pass);
                    System::RunElevatedCommand("rm -rf \"" + stage_bin + "\"", sudo_pass);
                    System::RunElevatedCommand("cp -a \"" + stage_dir + "/\". \"" + m_project.default_prefix + "/\"", sudo_pass);
                } else {
                    System::RunElevatedCommand("cp -a \"" + stage_dir + "/\". \"" + m_project.default_prefix + "/\"", sudo_pass);
                }

                // Update m_installed_files paths to reflect final prefix
                for (auto& f : m_installed_files) {
                    if (prefix_is_bin && stage_has_bin && f.rfind(stage_bin, 0) == 0) {
                        f = m_project.default_prefix + f.substr(stage_bin.length());
                    } else if (f.rfind(stage_dir, 0) == 0) {
                        f = m_project.default_prefix + f.substr(stage_dir.length());
                    }
                }
                IzSystem("rm -rf \"" + stage_dir + "\" 2>/dev/null");
            }

            // Step 4: Desktop Entry Integration
            if (m_create_desktop_entry) {
                m_worker_bridge.SetOperation("Creating desktop launcher and menu items...");
                m_worker_bridge.AppendLog("Creating FreeDesktop XDG shortcut...");
                System::InstallDesktopEntry(m_project, m_project.default_prefix, is_system_install, sudo_pass);
            }

            // Step 5: Run Post-Install JS Hook
            if (!m_project.post_install_js.empty()) {
                m_worker_bridge.SetOperation("Running Post-Install Script...");
                m_worker_bridge.AppendLog("Executing Post-Install JavaScript hook...");

                JsEngine engine;
                JsContextConfig cfg;
                cfg.app_name = m_project.app_name;
                cfg.version = m_project.version;
                cfg.install_dir = m_project.default_prefix;
                cfg.log_callback = [this](const std::string& msg) {
                    m_worker_bridge.AppendLog(msg);
                };

                engine.Initialize(cfg);
                std::string err;
                if (engine.Execute(m_project.post_install_js, &err) != StatusCode::Success) {
                    m_worker_bridge.Finish(false, "Post-Install JS failed: " + err);
                    return;
                }
            }

            // Step 6: Generate Uninstaller Records
            m_worker_bridge.SetOperation("Generating uninstaller records...");
            System::GenerateUninstaller(m_project.default_prefix, m_project, m_installed_files, sudo_pass);

            m_worker_bridge.SetProgress(1.0f);
            m_worker_bridge.SetOperation("Done");
            m_worker_bridge.AppendLog("Installation finished successfully.");
            m_worker_bridge.Finish(true);
        });
    }
};

} // namespace iz

int main(int argc, char** argv) {
    iz::RunnerApp app(argc, argv);
    app.Run();
    return 0;
}
