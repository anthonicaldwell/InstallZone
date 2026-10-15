#pragma once

#include "jpl_defs.h"
#include <string>
#include <functional>

struct duk_hthread;
typedef struct duk_hthread duk_context;

namespace iz {

using JsLogCallback = std::function<void(const std::string& msg)>;

struct JsContextConfig {
    std::string app_name;
    std::string version;
    std::string install_dir;
    std::string home_dir;
    std::string config_dir;
    bool is_uninstall{false};
    JsLogCallback log_callback;
};

class JsEngine {
public:
    JsEngine();
    ~JsEngine();

    StatusCode Initialize(const JsContextConfig& config);
    void Destroy();

    StatusCode Execute(const std::string& script, std::string* error_out = nullptr);

    // Helper to get real user's home directory even when running under sudo/pkexec
    static std::string DetectRealUserHome();

private:
    duk_context* m_ctx{nullptr};
    JsContextConfig m_config;

    void RegisterBuiltinBindings();

    // Duktape C-function callbacks
    static int NativeLog(duk_context* ctx);
    static int NativeMkdir(duk_context* ctx);
    static int NativeWriteFile(duk_context* ctx);
    static int NativeReadFile(duk_context* ctx);
    static int NativeFileExists(duk_context* ctx);
    static int NativeRemove(duk_context* ctx);
    static int NativeCopy(duk_context* ctx);
    static int NativeExec(duk_context* ctx);
    static int NativeGetEnv(duk_context* ctx);
    static int NativeSetEnv(duk_context* ctx);
};

} // namespace iz
