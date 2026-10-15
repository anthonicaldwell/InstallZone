#pragma once

#include "jpl_defs.h"
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>

struct SDL_Window;
typedef void* SDL_GLContext;

namespace iz {

/**
 * @struct WorkerBridge
 * @brief Thread-safe communication bridge between UI thread and background logic thread.
 * Guarantees zero ImGui context loss by strictly confining ImGui to the main UI thread.
 */
struct WorkerBridge {
    std::atomic<bool> is_running{false};
    std::atomic<bool> is_finished{false};
    std::atomic<bool> success{false};
    std::atomic<float> progress{0.0f};

    mutable std::mutex data_mutex;
    std::string current_operation;
    std::string error_message;
    std::vector<std::string> logs;

    void AppendLog(const std::string& line);
    void SetOperation(const std::string& op);
    void SetProgress(float p);
    void Finish(bool ok, const std::string& err = "");
    void Reset();

    std::string GetCurrentOperation() const;
    std::string GetErrorMessage() const;
    void CopyLogs(std::vector<std::string>& out_logs) const;
};

class UIContext {
public:
    UIContext();
    ~UIContext();

    StatusCode Initialize(const char* title, int width, int height);
    void Shutdown();

    bool ProcessEvents(); // Returns false if exit requested
    void BeginFrame();
    void EndFrame();

    void ApplyTheme();
    void RenderLanguageSelector();
    void RenderBanner(unsigned int texture_id, float banner_height = 115.0f, bool preserve_aspect = true);

    unsigned int LoadBannerTexture(const char* custom_path = nullptr);
    static unsigned int LoadTextureFromFile(const char* filepath, int* out_width = nullptr, int* out_height = nullptr);

    SDL_Window* GetWindow() const { return m_window; }

private:
    SDL_Window* m_window{nullptr};
    SDL_GLContext m_gl_context{nullptr};
    unsigned int m_banner_texture{0};
    int m_banner_width{0};
    int m_banner_height{0};
    bool m_initialized{false};
};

} // namespace iz
