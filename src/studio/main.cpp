#include "iz_ui_common.h"
#include "iz_project.h"
#include "iz_archive.h"
#include "iz_system.h"
#include "iz_js_engine.h"
#include "iz_cmake_scanner.h"
#include "iz_i18n.h"
#include "iz_assert.h"

#include "imgui.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

namespace iz {

class StudioApp {
public:
    StudioApp() {
        SetupUserConfigDirectory();
        m_project.ResetToDefaults();
        FindDefaultRunnerTemplate();
        m_output_installer_path = "dist/" + m_project.app_name + "-Installer.run";
    }

    static void SetupUserConfigDirectory() {
        const char* home = std::getenv("HOME");
        if (!home) return;

        std::string base_dir = std::string(home) + "/.config/InstallZone";
        std::string tpl_dir = base_dir + "/templates";
        std::string ast_dir = base_dir + "/assets";

        mkdir(base_dir.c_str(), 0755);
        mkdir(tpl_dir.c_str(), 0755);
        mkdir(ast_dir.c_str(), 0755);

        // 1. Copy runner executable to templates/ if missing or if a source binary is newer
        std::string target_runner = tpl_dir + "/installzone-runner";
        struct stat target_st;
        bool target_exists = (stat(target_runner.c_str(), &target_st) == 0);

        const char* runner_candidates[] = {
            "build/installzone-runner",
            "../build/installzone-runner",
            "./installzone-runner",
            "/usr/bin/installzone-runner",
            "/usr/share/installzone/templates/installzone-runner",
            "/usr/local/bin/installzone-runner",
            "/usr/local/share/installzone/templates/installzone-runner"
        };
        for (const char* src : runner_candidates) {
            struct stat src_st;
            if (stat(src, &src_st) == 0) {
                if (!target_exists || src_st.st_mtime > target_st.st_mtime) {
                    std::string cp_cmd = "cp \"" + std::string(src) + "\" \"" + target_runner + "\" && chmod 0755 \"" + target_runner + "\"";
                    IzSystem(cp_cmd);
                    break;
                }
            }
        }

        // 2. Copy assets (banner, chiptune, studio banner) to assets/ if missing or newer
        const char* asset_files[] = { "chiptune.wav", "banner.jpg", "banner.png", "banner_studio.png" };
        for (const char* f : asset_files) {
            std::string target_file = ast_dir + "/" + f;
            struct stat ast_target_st;
            bool ast_target_exists = (stat(target_file.c_str(), &ast_target_st) == 0);

            std::vector<std::string> candidates = {
                std::string("assets/") + f,
                std::string("../assets/") + f,
                std::string("/usr/share/installzone/assets/") + f,
                std::string("/usr/local/share/installzone/assets/") + f
            };
            for (const auto& src : candidates) {
                struct stat src_st;
                if (stat(src.c_str(), &src_st) == 0) {
                    if (!ast_target_exists || src_st.st_mtime > ast_target_st.st_mtime) {
                        std::string cp_cmd = "cp \"" + src + "\" \"" + target_file + "\"";
                        IzSystem(cp_cmd);
                        break;
                    }
                }
            }
        }
    }

    void Run() {
        I18n::Initialize();

        if (m_ui.Initialize(_("app_title_studio"), 1060, 800) != StatusCode::Success) {
            std::cerr << "Failed to initialize Studio UI.\n";
            return;
        }

        // Load Studio ribbon banner (assets/banner_studio.png)
        const char* home = std::getenv("HOME");
        std::vector<std::string> studio_banners = {
            (home ? std::string(home) + "/.config/InstallZone/assets/banner_studio.png" : ""),
            "assets/banner_studio.png",
            "../assets/banner_studio.png",
            "/usr/share/installzone/assets/banner_studio.png",
            "/usr/local/share/installzone/assets/banner_studio.png"
        };
        for (const auto& sb : studio_banners) {
            if (!sb.empty() && access(sb.c_str(), R_OK) == 0) {
                m_ui.LoadBannerTexture(sb.c_str());
                break;
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
        m_ui.Shutdown();
    }

private:
    UIContext m_ui;
    Project m_project;
    std::string m_current_project_file;
    std::string m_runner_template_path;
    std::string m_output_installer_path;

    // UI state buffers
    char m_buf_app_name[kMaxNameLen] = {0};
    char m_buf_app_id[kMaxNameLen] = {0};
    char m_buf_version[64] = {0};
    char m_buf_author[kMaxNameLen] = {0};
    char m_buf_category[kMaxNameLen] = {0};
    char m_buf_license[64] = {0};
    char m_buf_prefix[kMaxPathLen] = {0};
    char m_buf_desc[kMaxDescLen] = {0};

    char m_buf_new_src[kMaxPathLen] = {0};
    char m_buf_new_dst[kMaxPathLen] = {0};
    int m_new_perms{0755};

    char m_buf_dep_name[kMaxNameLen] = {0};
    char m_buf_dep_pkg[kMaxNameLen] = {0};
    char m_buf_dep_cmd[kMaxPathLen] = {0};
    bool m_dep_required{true};

    char m_buf_desktop_name[kMaxNameLen] = {0};
    char m_buf_desktop_exec[kMaxPathLen] = {0};
    char m_buf_desktop_icon[kMaxPathLen] = {0};
    char m_buf_desktop_cats[kMaxNameLen] = {0};

    // Script buffers
    std::string m_script_pre_install;
    std::string m_script_post_install;
    std::string m_script_pre_uninstall;
    std::string m_script_post_uninstall;

    // Worker thread & synchronization
    WorkerBridge m_worker_bridge;
    std::thread m_worker_thread;

    bool m_show_about_dialog{false};
    char m_buf_cmake_path[kMaxPathLen] = "CMakeLists.txt";
    bool m_show_cmake_modal{false};
    std::vector<std::string> m_cmake_scan_log;

    void ScanCMakeFile(const std::string& path) {
        m_cmake_scan_log.clear();
        StatusCode res = CMakeScanner::ApplyToProject(path, m_project, &m_cmake_scan_log);
        if (res == StatusCode::Success) {
            SyncProjectToBuffers();
        }
        m_show_cmake_modal = true;
    }

    void SyncProjectToBuffers() {
        std::snprintf(m_buf_app_name, sizeof(m_buf_app_name), "%s", m_project.app_name.c_str());
        std::snprintf(m_buf_app_id, sizeof(m_buf_app_id), "%s", m_project.app_id.c_str());
        std::snprintf(m_buf_version, sizeof(m_buf_version), "%s", m_project.version.c_str());
        std::snprintf(m_buf_author, sizeof(m_buf_author), "%s", m_project.author.c_str());
        std::snprintf(m_buf_category, sizeof(m_buf_category), "%s", m_project.category.c_str());
        std::snprintf(m_buf_license, sizeof(m_buf_license), "%s", m_project.license.c_str());
        std::snprintf(m_buf_prefix, sizeof(m_buf_prefix), "%s", m_project.default_prefix.c_str());
        std::snprintf(m_buf_desc, sizeof(m_buf_desc), "%s", m_project.description.c_str());

        std::snprintf(m_buf_desktop_name, sizeof(m_buf_desktop_name), "%s", m_project.desktop.name.c_str());
        std::snprintf(m_buf_desktop_exec, sizeof(m_buf_desktop_exec), "%s", m_project.desktop.exec_cmd.c_str());
        std::snprintf(m_buf_desktop_icon, sizeof(m_buf_desktop_icon), "%s", m_project.desktop.icon_source.c_str());
        std::snprintf(m_buf_desktop_cats, sizeof(m_buf_desktop_cats), "%s", m_project.desktop.categories.c_str());

        m_script_pre_install = m_project.pre_install_js;
        m_script_post_install = m_project.post_install_js;
        m_script_pre_uninstall = m_project.pre_uninstall_js;
        m_script_post_uninstall = m_project.post_uninstall_js;
    }

    void SyncBuffersToProject() {
        m_project.app_name = m_buf_app_name;
        m_project.app_id = m_buf_app_id;
        m_project.version = m_buf_version;
        m_project.author = m_buf_author;
        m_project.category = m_buf_category;
        m_project.license = m_buf_license;
        m_project.default_prefix = m_buf_prefix;
        m_project.description = m_buf_desc;

        m_project.desktop.name = m_buf_desktop_name;
        m_project.desktop.exec_cmd = m_buf_desktop_exec;
        m_project.desktop.icon_source = m_buf_desktop_icon;
        m_project.desktop.categories = m_buf_desktop_cats;

        m_project.pre_install_js = m_script_pre_install;
        m_project.post_install_js = m_script_post_install;
        m_project.pre_uninstall_js = m_script_pre_uninstall;
        m_project.post_uninstall_js = m_script_post_uninstall;
    }

    void FindDefaultRunnerTemplate() {
        const char* home = std::getenv("HOME");
        std::vector<std::string> search_paths = {
            (home ? std::string(home) + "/.config/InstallZone/templates/installzone-runner" : ""),
            "/usr/share/installzone/templates/installzone-runner",
            "/usr/bin/installzone-runner",
            "/usr/local/share/installzone/templates/installzone-runner",
            "/usr/local/bin/installzone-runner",
            "build/installzone-runner",
            "../build/installzone-runner",
            "./installzone-runner"
        };
        for (const auto& p : search_paths) {
            if (!p.empty() && access(p.c_str(), R_OK) == 0) {
                m_runner_template_path = p;
                return;
            }
        }
        m_runner_template_path = "build/installzone-runner";
    }

    void RenderUI() {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::Begin("StudioMain", nullptr, window_flags);
        ImGui::PopStyleVar();

        RenderMenuBar();

        // Retro 2000's GPU Box Art Header Banner (Preserve Studio aspect ratio)
        m_ui.RenderBanner(0, 110.0f, true);
        ImGui::Spacing();

        if (ImGui::BeginTabBar("StudioTabs", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem(_("tab_general"))) {
                RenderGeneralTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(_("tab_files"))) {
                RenderFilesTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(_("tab_dependencies"))) {
                RenderDependenciesTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(_("tab_scripts"))) {
                RenderScriptsTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(_("tab_desktop"))) {
                RenderDesktopTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(_("tab_pack"))) {
                RenderPackTab();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        RenderAboutModal();
        RenderCMakeModal();
        ImGui::End();
    }

    void RenderMenuBar() {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem(_("btn_new"), "Ctrl+N")) {
                    m_project.ResetToDefaults();
                    SyncProjectToBuffers();
                    m_current_project_file.clear();
                }
                if (ImGui::MenuItem(_("btn_open"), "Ctrl+O")) {
                    std::string sel = System::OpenFileDialog("Open InstallZone Project", "", "*.izproj|InstallZone Projects (*.izproj)");
                    if (!sel.empty()) {
                        if (m_project.LoadFromFile(sel) == StatusCode::Success) {
                            m_current_project_file = sel;
                            SyncProjectToBuffers();
                        }
                    }
                }
                if (ImGui::MenuItem(_("btn_save"), "Ctrl+S")) {
                    SyncBuffersToProject();
                    std::string save_path = m_current_project_file.empty() ? (m_project.app_name + ".izproj") : m_current_project_file;
                    m_project.SaveToFile(save_path);
                    m_current_project_file = save_path;
                }
                if (ImGui::MenuItem(_("btn_save_as"))) {
                    SyncBuffersToProject();
                    std::string sel = System::SaveFileDialog("Save Project As", "", "*.izproj|InstallZone Projects (*.izproj)");
                    if (!sel.empty()) {
                        m_project.SaveToFile(sel);
                        m_current_project_file = sel;
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Scan CMakeLists.txt...", "Ctrl+M")) {
                    ScanCMakeFile(m_buf_cmake_path);
                }
                ImGui::Separator();
                if (ImGui::MenuItem(_("btn_exit"), "Alt+F4")) {
                    exit(0);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu(_("lbl_help"))) {
                if (ImGui::MenuItem(_("lbl_about"))) {
                    m_show_about_dialog = true;
                }
                ImGui::EndMenu();
            }

            // Language Selector on the right side of the menu bar
            float combo_width = 160.0f;
            float avail = ImGui::GetContentRegionAvail().x;
            if (avail > combo_width) {
                ImGui::SameLine(ImGui::GetWindowWidth() - combo_width - 15.0f);
                m_ui.RenderLanguageSelector();
            }

            ImGui::EndMenuBar();
        }
    }

    void RenderGeneralTab() {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "%s", _("tab_general"));
        ImGui::Separator();
        ImGui::Spacing();

        // CMake Auto-Scan Card
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.16f, 0.20f, 0.27f, 0.85f));
        if (ImGui::BeginChild("CMakeScanCard", ImVec2(-1, 80), true)) {
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "CMake Project Scanner & Auto-Populate:");
            ImGui::SameLine();
            ImGui::TextDisabled("(Extracts project name, version, targets, install rules, and apt dependencies)");

            ImGui::SetNextItemWidth(300.0f);
            ImGui::InputTextWithHint("##CMakeScanPath", "Path to CMakeLists.txt", m_buf_cmake_path, sizeof(m_buf_cmake_path));
            ImGui::SameLine();
            if (ImGui::Button("Browse...##CMakeBrowse")) {
                std::string sel = System::OpenFileDialog("Select CMakeLists.txt", "", "*.txt|CMakeLists.txt (CMakeLists.txt)");
                if (!sel.empty()) {
                    std::snprintf(m_buf_cmake_path, sizeof(m_buf_cmake_path), "%s", sel.c_str());
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Scan CMakeLists.txt")) {
                ScanCMakeFile(m_buf_cmake_path);
            }
            ImGui::SameLine();
            if (ImGui::Button("Quick Scan src/CMakeLists.txt")) {
                std::snprintf(m_buf_cmake_path, sizeof(m_buf_cmake_path), "src/CMakeLists.txt");
                ScanCMakeFile(m_buf_cmake_path);
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::Columns(2, "GeneralCols", false);
        ImGui::SetColumnWidth(0, 220.0f);

        ImGui::Text("%s", _("lbl_app_name"));
        ImGui::NextColumn();
        ImGui::InputText("##AppName", m_buf_app_name, sizeof(m_buf_app_name));
        ImGui::NextColumn();

        ImGui::Text("%s", _("lbl_app_id"));
        ImGui::NextColumn();
        ImGui::InputText("##AppId", m_buf_app_id, sizeof(m_buf_app_id));
        ImGui::NextColumn();

        ImGui::Text("%s", _("lbl_version"));
        ImGui::NextColumn();
        ImGui::InputText("##Version", m_buf_version, sizeof(m_buf_version));
        ImGui::NextColumn();

        ImGui::Text("%s", _("lbl_author"));
        ImGui::NextColumn();
        ImGui::InputText("##Author", m_buf_author, sizeof(m_buf_author));
        ImGui::NextColumn();

        ImGui::Text("%s", _("lbl_category"));
        ImGui::NextColumn();
        ImGui::InputText("##Category", m_buf_category, sizeof(m_buf_category));
        ImGui::NextColumn();

        ImGui::Text("%s", _("lbl_license"));
        ImGui::NextColumn();
        ImGui::InputText("##License", m_buf_license, sizeof(m_buf_license));
        ImGui::NextColumn();

        ImGui::Text("%s", _("lbl_install_prefix"));
        ImGui::NextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 100.0f);
        ImGui::InputText("##Prefix", m_buf_prefix, sizeof(m_buf_prefix));
        ImGui::SameLine();
        if (ImGui::Button("Browse...##PrefixBrowse", ImVec2(90, 0))) {
            std::string sel = System::OpenDirectoryDialog("Select Default Installation Directory", m_buf_prefix);
            if (!sel.empty()) {
                std::snprintf(m_buf_prefix, sizeof(m_buf_prefix), "%s", sel.c_str());
            }
        }
        ImGui::NextColumn();

        ImGui::Text("%s", _("lbl_description"));
        ImGui::NextColumn();
        ImGui::InputTextMultiline("##Desc", m_buf_desc, sizeof(m_buf_desc), ImVec2(-1, 80));
        ImGui::NextColumn();

        ImGui::Columns(1);
    }

    void RenderFilesTab() {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "%s", _("tab_files"));
        ImGui::Separator();
        ImGui::Spacing();

        // Top quick-add action buttons
        if (ImGui::Button("📁 Add File(s)...", ImVec2(160, 32))) {
            std::string sel = System::OpenFileDialog("Select File to Add to Installer Payload");
            if (!sel.empty()) {
                FileEntry fe;
                fe.source_path = sel;
                size_t slash = sel.find_last_of('/');
                fe.target_subpath = (slash != std::string::npos) ? sel.substr(slash + 1) : sel;
                struct stat st;
                if (stat(sel.c_str(), &st) == 0) {
                    fe.file_size = st.st_size;
                    fe.unix_permissions = (st.st_mode & 0111) ? 0755 : 0644;
                } else {
                    fe.file_size = 0;
                    fe.unix_permissions = 0755;
                }
                m_project.files.push_back(fe);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("📂 Add Directory...", ImVec2(170, 32))) {
            std::string sel = System::OpenDirectoryDialog("Select Directory to Add to Installer Payload");
            if (!sel.empty()) {
                size_t slash = sel.find_last_of('/');
                std::string dir_name = (slash != std::string::npos) ? sel.substr(slash + 1) : sel;
                DIR* d = opendir(sel.c_str());
                if (d) {
                    struct dirent* ent;
                    while ((ent = readdir(d)) != nullptr) {
                        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
                        std::string fpath = sel + "/" + ent->d_name;
                        struct stat st;
                        if (stat(fpath.c_str(), &st) == 0 && !S_ISDIR(st.st_mode)) {
                            FileEntry fe;
                            fe.source_path = fpath;
                            fe.target_subpath = dir_name + "/" + ent->d_name;
                            fe.file_size = st.st_size;
                            fe.unix_permissions = (st.st_mode & 0111) ? 0755 : 0644;
                            m_project.files.push_back(fe);
                        }
                    }
                    closedir(d);
                }
            }
        }

        ImGui::Spacing();

        // Custom manual file entry card
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.16f, 0.20f, 0.27f, 0.50f));
        if (ImGui::BeginChild("ManualAddCard", ImVec2(-1, 115), true)) {
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Custom File / Destination Rule:");

            // Row 1: Source file / dir with clear browse buttons
            float btn_w = 95.0f;
            float input_w = ImGui::GetContentRegionAvail().x - (btn_w * 2.0f) - 20.0f;
            ImGui::SetNextItemWidth(input_w);
            ImGui::InputTextWithHint("##SrcPath", "Source file or directory on development machine", m_buf_new_src, sizeof(m_buf_new_src));
            ImGui::SameLine();
            if (ImGui::Button("File...##FileBrowse", ImVec2(btn_w, 0))) {
                std::string sel = System::OpenFileDialog("Select File for Payload");
                if (!sel.empty()) {
                    std::snprintf(m_buf_new_src, sizeof(m_buf_new_src), "%s", sel.c_str());
                    if (m_buf_new_dst[0] == '\0') {
                        size_t slash = sel.find_last_of('/');
                        std::string base = (slash != std::string::npos) ? sel.substr(slash + 1) : sel;
                        std::snprintf(m_buf_new_dst, sizeof(m_buf_new_dst), "%s", base.c_str());
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Dir...##DirBrowse", ImVec2(btn_w, 0))) {
                std::string sel = System::OpenDirectoryDialog("Select Directory for Payload");
                if (!sel.empty()) {
                    std::snprintf(m_buf_new_src, sizeof(m_buf_new_src), "%s", sel.c_str());
                    if (m_buf_new_dst[0] == '\0') {
                        size_t slash = sel.find_last_of('/');
                        std::string base = (slash != std::string::npos) ? sel.substr(slash + 1) : sel;
                        std::snprintf(m_buf_new_dst, sizeof(m_buf_new_dst), "%s", base.c_str());
                    }
                }
            }

            // Row 2: Target subpath & permissions
            ImGui::SetNextItemWidth(input_w * 0.55f);
            ImGui::InputTextWithHint("##DstPath", "Target subpath in install directory (e.g. bin/my-app)", m_buf_new_dst, sizeof(m_buf_new_dst));
            ImGui::SameLine();

            const char* perms_items[] = { "0755 (Exec/Dir)", "0644 (Data/Config)" };
            static int perms_idx = 0;
            ImGui::SetNextItemWidth(140.0f);
            if (ImGui::Combo("##PermsCombo", &perms_idx, perms_items, IM_ARRAYSIZE(perms_items))) {
                m_new_perms = (perms_idx == 0) ? 0755 : 0644;
            }
            ImGui::SameLine();

            if (ImGui::Button(_("btn_add_file"), ImVec2(130, 0))) {
                if (m_buf_new_src[0] != '\0' && m_buf_new_dst[0] != '\0') {
                    FileEntry fe;
                    fe.source_path = m_buf_new_src;
                    fe.target_subpath = m_buf_new_dst;
                    fe.unix_permissions = m_new_perms;

                    struct stat st;
                    if (stat(fe.source_path.c_str(), &st) == 0) {
                        fe.file_size = st.st_size;
                    } else {
                        fe.file_size = 0;
                    }
                    m_project.files.push_back(fe);

                    m_buf_new_src[0] = '\0';
                    m_buf_new_dst[0] = '\0';
                }
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();

        // Files Table with inline Browse buttons
        ImVec2 size = ImVec2(-1, 300);
        if (ImGui::BeginTable("FilesTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, size)) {
            ImGui::TableSetupColumn(_("lbl_source_path"), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(_("lbl_target_path"), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(_("lbl_permissions"), ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableSetupColumn("Browse", ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 75.0f);
            ImGui::TableHeadersRow();

            int to_remove = -1;
            for (size_t i = 0; i < m_project.files.size(); ++i) {
                auto& f = m_project.files[i];
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(f.source_path.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(f.target_subpath.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("0%o", f.unix_permissions);

                ImGui::TableSetColumnIndex(3);
                ImGui::PushID(static_cast<int>(i * 2));
                if (ImGui::SmallButton("Browse...")) {
                    std::string sel = System::OpenFileDialog("Replace Source File", f.source_path);
                    if (!sel.empty()) {
                        f.source_path = sel;
                        struct stat st;
                        if (stat(sel.c_str(), &st) == 0) {
                            f.file_size = st.st_size;
                        }
                    }
                }
                ImGui::PopID();

                ImGui::TableSetColumnIndex(4);
                ImGui::PushID(static_cast<int>(i * 2 + 1));
                if (ImGui::SmallButton("Delete")) {
                    to_remove = static_cast<int>(i);
                }
                ImGui::PopID();
            }

            if (to_remove >= 0 && to_remove < static_cast<int>(m_project.files.size())) {
                m_project.files.erase(m_project.files.begin() + to_remove);
            }

            ImGui::EndTable();
        }

        ImGui::Text("Total Payload Size: %.2f MB (%zu files)",
                    static_cast<double>(m_project.CalculateTotalPayloadSize()) / (1024.0 * 1024.0),
                    m_project.files.size());
    }

    void RenderDependenciesTab() {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "%s", _("tab_dependencies"));
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Add Required System / Apt Dependency:");
        ImGui::SetNextItemWidth(180.0f);
        ImGui::InputTextWithHint("##DepName", _("lbl_dep_name"), m_buf_dep_name, sizeof(m_buf_dep_name));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(180.0f);
        ImGui::InputTextWithHint("##DepPkg", _("lbl_dep_package"), m_buf_dep_pkg, sizeof(m_buf_dep_pkg));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(220.0f);
        ImGui::InputTextWithHint("##DepCmd", _("lbl_dep_check_cmd"), m_buf_dep_cmd, sizeof(m_buf_dep_cmd));
        ImGui::SameLine();
        ImGui::Checkbox(_("lbl_dep_required"), &m_dep_required);
        ImGui::SameLine();

        if (ImGui::Button(_("btn_add_dep"))) {
            if (m_buf_dep_name[0] != '\0') {
                DependencyEntry de;
                de.name = m_buf_dep_name;
                de.package_name = m_buf_dep_pkg;
                de.check_command = m_buf_dep_cmd;
                de.is_required = m_dep_required;
                m_project.dependencies.push_back(de);

                m_buf_dep_name[0] = '\0';
                m_buf_dep_pkg[0] = '\0';
                m_buf_dep_cmd[0] = '\0';
            }
        }

        ImGui::Spacing();
        ImGui::Separator();

        // Dependencies Table
        ImVec2 size = ImVec2(-1, 300);
        if (ImGui::BeginTable("DepsTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, size)) {
            ImGui::TableSetupColumn(_("lbl_dep_name"), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(_("lbl_dep_package"), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn(_("lbl_dep_check_cmd"), ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 160.0f);
            ImGui::TableHeadersRow();

            int to_remove = -1;
            for (size_t i = 0; i < m_project.dependencies.size(); ++i) {
                const auto& d = m_project.dependencies[i];
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(d.name.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(d.package_name.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(d.check_command.c_str());

                ImGui::TableSetColumnIndex(3);
                bool ok = System::IsDependencySatisfied(d);
                if (ok) {
                    ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "Installed");
                } else {
                    ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.3f, 1.0f), "Missing");
                }

                ImGui::TableSetColumnIndex(4);
                ImGui::PushID(static_cast<int>(i));
                if (ImGui::SmallButton("Test")) {
                    bool res = System::IsDependencySatisfied(d);
                    std::cout << "Dependency test " << d.name << ": " << (res ? "OK" : "Missing") << "\n";
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Delete")) {
                    to_remove = static_cast<int>(i);
                }
                ImGui::PopID();
            }

            if (to_remove >= 0 && to_remove < static_cast<int>(m_project.dependencies.size())) {
                m_project.dependencies.erase(m_project.dependencies.begin() + to_remove);
            }

            ImGui::EndTable();
        }
    }

    void RenderScriptsTab() {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "%s", _("tab_scripts"));
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTabBar("ScriptSubTabs")) {
            if (ImGui::BeginTabItem(_("lbl_pre_install_js"))) {
                ImGui::Text("Executes before payload extraction (e.g. system check, stopping services):");
                static char pre_buf[65536] = {0};
                if (pre_buf[0] == '\0') std::snprintf(pre_buf, sizeof(pre_buf), "%s", m_script_pre_install.c_str());
                if (ImGui::InputTextMultiline("##PreJs", pre_buf, sizeof(pre_buf), ImVec2(-1, 260))) {
                    m_script_pre_install = pre_buf;
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(_("lbl_post_install_js"))) {
                ImGui::Text("Executes after payload extraction (e.g. creating ~/.config, configuring navi, symlinks):");

                // Quick snippet buttons!
                if (ImGui::Button(_("lbl_sample_mkdir"))) {
                    m_script_post_install += "\n// Create ~/.config directory\nvar myConfig = iz.config_dir + '/' + iz.app_name.toLowerCase();\niz.mkdir(myConfig);\niz.write_file(myConfig + '/config.json', '{\"configured\": true}\\n');\n";
                }
                ImGui::SameLine();
                if (ImGui::Button(_("lbl_sample_navi"))) {
                    m_script_post_install += "\n// Configure Navi cheatsheet\nvar naviCheats = iz.home_dir + '/.config/navi/cheats';\niz.mkdir(naviCheats);\niz.write_file(naviCheats + '/' + iz.app_name.toLowerCase() + '.cheat', '% ' + iz.app_name + '\\n# Run application\\n' + iz.app_name.toLowerCase() + ' --help\\n');\niz.log('Configured navi cheatsheet in ' + naviCheats);\n";
                }

                static char post_buf[65536] = {0};
                std::snprintf(post_buf, sizeof(post_buf), "%s", m_script_post_install.c_str());
                if (ImGui::InputTextMultiline("##PostJs", post_buf, sizeof(post_buf), ImVec2(-1, 260))) {
                    m_script_post_install = post_buf;
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(_("lbl_pre_uninstall_js"))) {
                static char pre_un_buf[65536] = {0};
                std::snprintf(pre_un_buf, sizeof(pre_un_buf), "%s", m_script_pre_uninstall.c_str());
                if (ImGui::InputTextMultiline("##PreUnJs", pre_un_buf, sizeof(pre_un_buf), ImVec2(-1, 260))) {
                    m_script_pre_uninstall = pre_un_buf;
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(_("lbl_post_uninstall_js"))) {
                static char post_un_buf[65536] = {0};
                std::snprintf(post_un_buf, sizeof(post_un_buf), "%s", m_script_post_uninstall.c_str());
                if (ImGui::InputTextMultiline("##PostUnJs", post_un_buf, sizeof(post_un_buf), ImVec2(-1, 260))) {
                    m_script_post_uninstall = post_un_buf;
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::Spacing();
        if (ImGui::Button(_("btn_test_script"))) {
            JsEngine engine;
            JsContextConfig cfg;
            cfg.app_name = m_buf_app_name;
            cfg.version = m_buf_version;
            cfg.install_dir = "/tmp/installzone_sandbox";
            cfg.log_callback = [this](const std::string& msg) {
                m_worker_bridge.AppendLog(msg);
            };
            engine.Initialize(cfg);
            std::string err;
            engine.Execute(m_script_post_install, &err);
        }

        // Script output console
        ImGui::Spacing();
        ImGui::Text("Script Sandbox Output:");
        std::vector<std::string> logs;
        m_worker_bridge.CopyLogs(logs);
        if (ImGui::BeginChild("ScriptLogBox", ImVec2(-1, 120), true)) {
            for (const auto& line : logs) {
                ImGui::TextUnformatted(line.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
        }
        ImGui::EndChild();
    }

    void RenderDesktopTab() {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "%s", _("tab_desktop"));
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Checkbox("Generate XDG Desktop Launcher Entry (.desktop)", &m_project.desktop.create_desktop_entry);
        ImGui::Spacing();

        if (m_project.desktop.create_desktop_entry) {
            ImGui::Columns(2, "DesktopCols", false);
            ImGui::SetColumnWidth(0, 220.0f);

            ImGui::Text("%s", _("lbl_desktop_name"));
            ImGui::NextColumn();
            ImGui::InputText("##DesktopName", m_buf_desktop_name, sizeof(m_buf_desktop_name));
            ImGui::NextColumn();

            ImGui::Text("%s", _("lbl_desktop_exec"));
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 100.0f);
            ImGui::InputText("##DesktopExec", m_buf_desktop_exec, sizeof(m_buf_desktop_exec));
            ImGui::SameLine();
            if (ImGui::Button("Browse...##ExecBrowse", ImVec2(90, 0))) {
                std::string sel = System::OpenFileDialog("Select Executable Binary", m_buf_prefix);
                if (!sel.empty()) {
                    size_t slash = sel.find_last_of('/');
                    std::string base = (slash != std::string::npos) ? sel.substr(slash + 1) : sel;
                    std::snprintf(m_buf_desktop_exec, sizeof(m_buf_desktop_exec), "%s", base.c_str());
                }
            }
            ImGui::NextColumn();

            ImGui::Text("%s", _("lbl_desktop_icon"));
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 100.0f);
            ImGui::InputText("##DesktopIcon", m_buf_desktop_icon, sizeof(m_buf_desktop_icon));
            ImGui::SameLine();
            if (ImGui::Button("Browse...##IconBrowse", ImVec2(90, 0))) {
                std::string sel = System::OpenFileDialog("Select Icon File", "", "*.png *.svg *.ico *.xpm|Icon Files (*.png *.svg *.ico)");
                if (!sel.empty()) {
                    std::snprintf(m_buf_desktop_icon, sizeof(m_buf_desktop_icon), "%s", sel.c_str());
                }
            }
            ImGui::NextColumn();

            ImGui::Text("%s", _("lbl_desktop_categories"));
            ImGui::NextColumn();
            ImGui::InputText("##DesktopCats", m_buf_desktop_cats, sizeof(m_buf_desktop_cats));
            ImGui::NextColumn();

            ImGui::Text("%s", _("lbl_desktop_terminal"));
            ImGui::NextColumn();
            ImGui::Checkbox("##DesktopTerminal", &m_project.desktop.terminal);
            ImGui::NextColumn();

            ImGui::Columns(1);
        }
    }

    void RenderPackTab() {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "%s", _("tab_pack"));
        ImGui::Separator();
        ImGui::Spacing();

        static char template_buf[kMaxPathLen] = {0};
        if (template_buf[0] == '\0') std::snprintf(template_buf, sizeof(template_buf), "%s", m_runner_template_path.c_str());
        ImGui::Text("Runner Base Executable Path:");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 100.0f);
        if (ImGui::InputText("##RunnerTemplate", template_buf, sizeof(template_buf))) {
            m_runner_template_path = template_buf;
        }
        ImGui::SameLine();
        if (ImGui::Button("Browse...##TemplateBrowse", ImVec2(90, 0))) {
            std::string sel = System::OpenFileDialog("Select Runner Base Executable", "", "installzone-runner*|Executables");
            if (!sel.empty()) {
                m_runner_template_path = sel;
                std::snprintf(template_buf, sizeof(template_buf), "%s", sel.c_str());
            }
        }

        static char output_buf[kMaxPathLen] = {0};
        if (output_buf[0] == '\0') std::snprintf(output_buf, sizeof(output_buf), "%s", m_output_installer_path.c_str());
        ImGui::Text("%s", _("lbl_select_output"));
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 100.0f);
        if (ImGui::InputText("##OutputInstaller", output_buf, sizeof(output_buf))) {
            m_output_installer_path = output_buf;
        }
        ImGui::SameLine();
        if (ImGui::Button("Browse...##OutputBrowse", ImVec2(90, 0))) {
            std::string sel = System::SaveFileDialog("Save Standalone Installer As", "", "*.run|Installers (*.run)");
            if (!sel.empty()) {
                m_output_installer_path = sel;
                std::snprintf(output_buf, sizeof(output_buf), "%s", sel.c_str());
            }
        }

        ImGui::Spacing();

        bool is_packing = m_worker_bridge.is_running.load();
        if (is_packing) {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button(_("btn_pack_installer"), ImVec2(240, 42))) {
            SyncBuffersToProject();
            StartPacking();
        }

        if (is_packing) {
            ImGui::EndDisabled();
        }

        ImGui::Spacing();
        float prog = m_worker_bridge.progress.load();
        ImGui::ProgressBar(prog, ImVec2(-1, 24));
        ImGui::Text("%s %s", _("lbl_status"), m_worker_bridge.GetCurrentOperation().c_str());

        ImGui::Spacing();
        ImGui::Text("Packing Log:");
        std::vector<std::string> logs;
        m_worker_bridge.CopyLogs(logs);
        if (ImGui::BeginChild("PackLogBox", ImVec2(-1, 220), true)) {
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
                ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "Standalone Installer Successfully Generated!");
                ImGui::SameLine();
                if (ImGui::Button("Run Installer Now")) {
                    std::string target_path = m_output_installer_path;
                    if (!target_path.empty()) {
                        if (target_path.rfind("/", 0) != 0) {
                            char cwd[kMaxPathLen] = {0};
                            if (getcwd(cwd, sizeof(cwd))) {
                                target_path = std::string(cwd) + "/" + target_path;
                            }
                        }
                        chmod(target_path.c_str(), 0755);
                        m_worker_bridge.AppendLog("Launching installer: " + target_path);
                        std::string run_cmd = "\"" + target_path + "\" &";
                        IzSystem(run_cmd);
                    }
                }
            } else {
                ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "Packing failed: %s", m_worker_bridge.GetErrorMessage().c_str());
            }
        }
    }

    void StartPacking() {
        if (m_worker_thread.joinable()) {
            m_worker_thread.join();
        }
        m_worker_bridge.Reset();
        m_worker_bridge.is_running.store(true);
        m_worker_bridge.SetOperation(_("lbl_packing"));

        // Ensure output parent directory exists
        size_t slash = m_output_installer_path.find_last_of('/');
        if (slash != std::string::npos) {
            std::string parent = m_output_installer_path.substr(0, slash);
            IzSystem("mkdir -p \"" + parent + "\"");
        }

        // Launch background logic thread (completely separate from UI thread)
        m_worker_thread = std::thread([this]() {
            StatusCode res = Archive::PackStandaloneInstaller(
                m_runner_template_path,
                m_project,
                m_output_installer_path,
                [this](float p, const std::string& item) {
                    m_worker_bridge.SetProgress(p);
                    m_worker_bridge.SetOperation(item);
                },
                [this](const std::string& msg) {
                    m_worker_bridge.AppendLog(msg);
                }
            );

            if (res == StatusCode::Success) {
                m_worker_bridge.Finish(true);
            } else {
                m_worker_bridge.Finish(false, StatusCodeToString(res));
            }
        });
    }

    void RenderAboutModal() {
        if (m_show_about_dialog) {
            ImGui::OpenPopup(_("lbl_about"));
        }

        if (ImGui::BeginPopupModal(_("lbl_about"), &m_show_about_dialog, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "InstallZone Studio");
            ImGui::Text("Version 1.0.0 (KDE Plasma Edition)");
            ImGui::Separator();
            ImGui::TextUnformatted(_("lbl_about_desc"));
            ImGui::Spacing();
            if (ImGui::Button("Close", ImVec2(120, 0))) {
                m_show_about_dialog = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void RenderCMakeModal() {
        if (m_show_cmake_modal) {
            ImGui::OpenPopup("CMake Scanner Results");
        }

        if (ImGui::BeginPopupModal("CMake Scanner Results", &m_show_cmake_modal, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "CMakeLists.txt Scan Report:");
            ImGui::Separator();
            if (ImGui::BeginChild("CMakeLogBox", ImVec2(560, 220), true)) {
                for (const auto& line : m_cmake_scan_log) {
                    ImGui::TextUnformatted(line.c_str());
                }
            }
            ImGui::EndChild();
            ImGui::Spacing();
            if (ImGui::Button("Close", ImVec2(120, 0))) {
                m_show_cmake_modal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
};

} // namespace iz

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    iz::StudioApp app;
    app.Run();
    return 0;
}
