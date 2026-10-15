#include "iz_ui_common.h"
#include "iz_i18n.h"
#include "iz_assert.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <unistd.h>
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl2.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace iz {

// WorkerBridge Implementation
void WorkerBridge::AppendLog(const std::string& line) {
    std::lock_guard<std::mutex> lock(data_mutex);
    if (logs.size() >= kMaxLogLines) {
        logs.erase(logs.begin());
    }
    logs.push_back(line);
}

void WorkerBridge::SetOperation(const std::string& op) {
    std::lock_guard<std::mutex> lock(data_mutex);
    current_operation = op;
}

void WorkerBridge::SetProgress(float p) {
    progress.store(p);
}

void WorkerBridge::Finish(bool ok, const std::string& err) {
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        error_message = err;
        if (!err.empty()) {
            logs.push_back("[ERROR] " + err);
        }
    }
    success.store(ok);
    is_running.store(false);
    is_finished.store(true);
}

void WorkerBridge::Reset() {
    std::lock_guard<std::mutex> lock(data_mutex);
    is_running.store(false);
    is_finished.store(false);
    success.store(false);
    progress.store(0.0f);
    current_operation.clear();
    error_message.clear();
    logs.clear();
}

std::string WorkerBridge::GetCurrentOperation() const {
    std::lock_guard<std::mutex> lock(data_mutex);
    return current_operation;
}

std::string WorkerBridge::GetErrorMessage() const {
    std::lock_guard<std::mutex> lock(data_mutex);
    return error_message;
}

void WorkerBridge::CopyLogs(std::vector<std::string>& out_logs) const {
    std::lock_guard<std::mutex> lock(data_mutex);
    out_logs = logs;
}

// UIContext Implementation
UIContext::UIContext() = default;

UIContext::~UIContext() {
    Shutdown();
}

StatusCode UIContext::Initialize(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, "Error: SDL_Init failed: %s\n", SDL_GetError());
        return StatusCode::ErrInternal;
    }

    // Setup OpenGL 2.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );

    m_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        window_flags
    );

    if (!m_window) {
        std::fprintf(stderr, "Error: SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return StatusCode::ErrInternal;
    }

    m_gl_context = SDL_GL_CreateContext(m_window);
    if (!m_gl_context) {
        std::fprintf(stderr, "Error: SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        return StatusCode::ErrInternal;
    }

    SDL_GL_MakeCurrent(m_window, m_gl_context);
    SDL_GL_SetSwapInterval(1); // VSync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ApplyTheme();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_gl_context);
    ImGui_ImplOpenGL2_Init();

    // Load multilingual fonts
    I18n::LoadFonts(16.0f);

    m_initialized = true;
    return StatusCode::Success;
}

void UIContext::Shutdown() {
    if (m_initialized) {
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        if (m_gl_context) {
            SDL_GL_DeleteContext(m_gl_context);
            m_gl_context = nullptr;
        }
        if (m_window) {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }
        if (m_banner_texture) {
            glDeleteTextures(1, &m_banner_texture);
            m_banner_texture = 0;
        }
        SDL_Quit();
        m_initialized = false;
    }
}

bool UIContext::ProcessEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            return false;
        }
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
            event.window.windowID == SDL_GetWindowID(m_window)) {
            return false;
        }
    }
    return true;
}

void UIContext::BeginFrame() {
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void UIContext::EndFrame() {
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(0.12f, 0.14f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(m_window);
}

void UIContext::ApplyTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // KDE Plasma / Breeze Dark inspired modern palette
    colors[ImGuiCol_Text]                  = ImVec4(0.93f, 0.94f, 0.96f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.52f, 0.55f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.14f, 0.16f, 0.20f, 1.00f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.12f, 0.13f, 0.17f, 1.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.16f, 0.18f, 0.23f, 0.98f);
    colors[ImGuiCol_Border]                = ImVec4(0.24f, 0.28f, 0.35f, 0.80f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.19f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.25f, 0.30f, 0.38f, 1.00f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.29f, 0.35f, 0.45f, 1.00f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.12f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.15f, 0.18f, 0.24f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.10f, 0.12f, 0.15f, 0.80f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.15f, 0.17f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.12f, 0.13f, 0.17f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.28f, 0.33f, 0.42f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.35f, 0.42f, 0.54f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.42f, 0.50f, 0.64f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.26f, 0.65f, 0.96f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.26f, 0.65f, 0.96f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.36f, 0.75f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.22f, 0.38f, 0.58f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.28f, 0.46f, 0.70f, 1.00f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.33f, 0.54f, 0.82f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.22f, 0.26f, 0.34f, 1.00f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.28f, 0.34f, 0.44f, 1.00f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.32f, 0.40f, 0.52f, 1.00f);
    colors[ImGuiCol_Separator]             = ImVec4(0.24f, 0.28f, 0.35f, 0.80f);
    colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.35f, 0.42f, 0.54f, 1.00f);
    colors[ImGuiCol_SeparatorActive]       = ImVec4(0.42f, 0.50f, 0.64f, 1.00f);
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.26f, 0.65f, 0.96f, 0.30f);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.65f, 0.96f, 0.70f);
    colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.65f, 0.96f, 1.00f);
    colors[ImGuiCol_Tab]                   = ImVec4(0.17f, 0.20f, 0.26f, 1.00f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.26f, 0.42f, 0.64f, 1.00f);
    colors[ImGuiCol_TabSelected]           = ImVec4(0.22f, 0.38f, 0.58f, 1.00f);
    colors[ImGuiCol_TabDimmed]             = ImVec4(0.14f, 0.16f, 0.21f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected]     = ImVec4(0.18f, 0.24f, 0.34f, 1.00f);
    colors[ImGuiCol_PlotLines]             = ImVec4(0.40f, 0.70f, 0.95f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.50f, 0.80f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]         = ImVec4(0.26f, 0.65f, 0.96f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.36f, 0.75f, 1.00f, 1.00f);

    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 6.0f;
    style.FrameRounding     = 5.0f;
    style.PopupRounding     = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 5.0f;

    style.WindowPadding     = ImVec2(12.0f, 12.0f);
    style.FramePadding      = ImVec2(8.0f, 5.0f);
    style.ItemSpacing       = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing  = ImVec2(6.0f, 4.0f);
    style.ScrollbarSize     = 14.0f;
}

void UIContext::RenderLanguageSelector() {
    Language current = I18n::GetCurrentLanguage();
    const auto& list = I18n::GetSupportedLanguages();

    std::string preview = std::string(I18n::GetLanguageNativeName(current));
    ImGui::SetNextItemWidth(150.0f);
    if (ImGui::BeginCombo("##LanguageSelector", preview.c_str())) {
        for (const auto& item : list) {
            bool is_selected = (item.lang == current);
            std::string label = std::string(item.native_name) + " (" + item.english_name + ")";
            if (ImGui::Selectable(label.c_str(), is_selected)) {
                I18n::SetLanguage(item.lang);
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

unsigned int UIContext::LoadTextureFromFile(const char* filepath, int* out_width, int* out_height) {
    if (!filepath || access(filepath, R_OK) != 0) return 0;

    int w = 0, h = 0, channels = 0;
    unsigned char* data = stbi_load(filepath, &w, &h, &channels, 4);
    if (!data) return 0;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    if (out_width) *out_width = w;
    if (out_height) *out_height = h;
    return tex;
}

unsigned int UIContext::LoadBannerTexture() {
    if (m_banner_texture != 0) return m_banner_texture;

    const char* paths[] = {
        "assets/banner.jpg",
        "assets/banner.png",
        "../assets/banner.jpg",
        "../assets/banner.png",
        "/usr/local/share/installzone/assets/banner.jpg",
        "/usr/local/share/installzone/assets/banner.png"
    };

    const char* home = std::getenv("HOME");
    std::string home_banner = (home ? std::string(home) : "") + "/.local/share/installzone/assets/banner.jpg";

    for (const char* p : paths) {
        if (access(p, R_OK) == 0) {
            m_banner_texture = LoadTextureFromFile(p, &m_banner_width, &m_banner_height);
            if (m_banner_texture) return m_banner_texture;
        }
    }

    if (!home_banner.empty() && access(home_banner.c_str(), R_OK) == 0) {
        m_banner_texture = LoadTextureFromFile(home_banner.c_str(), &m_banner_width, &m_banner_height);
    }

    return m_banner_texture;
}

void UIContext::RenderBanner(unsigned int texture_id, float banner_height) {
    if (!texture_id) {
        texture_id = LoadBannerTexture();
    }
    if (!texture_id) return;

    float avail_width = ImGui::GetContentRegionAvail().x;
    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 p1 = ImVec2(p0.x + avail_width, p0.y + banner_height);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Render image with rounded corners
    draw_list->AddImageRounded(
        static_cast<ImTextureID>(texture_id),
        p0, p1,
        ImVec2(0, 0), ImVec2(1, 1),
        IM_COL32(255, 255, 255, 255), 6.0f
    );

    // Early 2000's GPU box art retro effects:
    // 1. Cyber cyan/electric blue glowing border
    draw_list->AddRect(p0, p1, IM_COL32(0, 220, 255, 180), 6.0f, 0, 2.0f);

    // 2. Scanline effect overlay (vintage CRT scanlines)
    for (float y = p0.y; y < p1.y; y += 4.0f) {
        draw_list->AddLine(ImVec2(p0.x, y), ImVec2(p1.x, y), IM_COL32(0, 0, 0, 35), 1.0f);
    }

    // 3. Vintage CRT phosphor sheen / vignette on edges
    draw_list->AddRectFilledMultiColor(
        p0, ImVec2(p0.x + 35.0f, p1.y),
        IM_COL32(0, 0, 0, 110), IM_COL32(0, 0, 0, 0),
        IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 110)
    );
    draw_list->AddRectFilledMultiColor(
        ImVec2(p1.x - 35.0f, p0.y), p1,
        IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 110),
        IM_COL32(0, 0, 0, 110), IM_COL32(0, 0, 0, 0)
    );

    // Advance cursor past banner
    ImGui::Dummy(ImVec2(avail_width, banner_height));
    ImGui::Spacing();
}

} // namespace iz
