#include "iz_js_engine.h"
#include "duktape.h"
#include "iz_assert.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace iz {

static std::string ExpandPath(const std::string& path, const std::string& home_dir) {
    if (path.rfind("~/", 0) == 0) {
        return home_dir + path.substr(1);
    }
    return path;
}

static void MakeDirectoryRecursive(const std::string& path, uid_t uid, gid_t gid, bool change_owner) {
    if (path.empty()) return;
    char tmp[kMaxPathLen];
    std::strncpy(tmp, path.c_str(), sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    size_t len = std::strlen(tmp);
    if (tmp[len - 1] == '/') tmp[len - 1] = '\0';

    for (char* p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0755);
            if (change_owner) {
                (void)!chown(tmp, uid, gid);
            }
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
    if (change_owner) {
        (void)!chown(tmp, uid, gid);
    }
}

std::string JsEngine::DetectRealUserHome() {
    const char* sudo_user = std::getenv("SUDO_USER");
    if (sudo_user && std::strlen(sudo_user) > 0) {
        struct passwd* pw = getpwnam(sudo_user);
        if (pw && pw->pw_dir) {
            return std::string(pw->pw_dir);
        }
    }

    const char* pkexec_uid = std::getenv("PKEXEC_UID");
    if (pkexec_uid && std::strlen(pkexec_uid) > 0) {
        uid_t u = static_cast<uid_t>(std::atoi(pkexec_uid));
        struct passwd* pw = getpwuid(u);
        if (pw && pw->pw_dir) {
            return std::string(pw->pw_dir);
        }
    }

    const char* home = std::getenv("HOME");
    if (home && std::strlen(home) > 0) {
        return std::string(home);
    }

    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        return std::string(pw->pw_dir);
    }

    return "/tmp";
}

static uid_t GetRealUserId() {
    const char* sudo_user = std::getenv("SUDO_USER");
    if (sudo_user && std::strlen(sudo_user) > 0) {
        struct passwd* pw = getpwnam(sudo_user);
        if (pw) return pw->pw_uid;
    }
    const char* pkexec_uid = std::getenv("PKEXEC_UID");
    if (pkexec_uid && std::strlen(pkexec_uid) > 0) {
        return static_cast<uid_t>(std::atoi(pkexec_uid));
    }
    return getuid();
}

static gid_t GetRealGroupId() {
    const char* sudo_user = std::getenv("SUDO_USER");
    if (sudo_user && std::strlen(sudo_user) > 0) {
        struct passwd* pw = getpwnam(sudo_user);
        if (pw) return pw->pw_gid;
    }
    const char* pkexec_uid = std::getenv("PKEXEC_UID");
    if (pkexec_uid && std::strlen(pkexec_uid) > 0) {
        struct passwd* pw = getpwuid(static_cast<uid_t>(std::atoi(pkexec_uid)));
        if (pw) return pw->pw_gid;
    }
    return getgid();
}

JsEngine::JsEngine() = default;

JsEngine::~JsEngine() {
    Destroy();
}

StatusCode JsEngine::Initialize(const JsContextConfig& config) {
    Destroy();
    m_config = config;

    if (m_config.home_dir.empty()) {
        m_config.home_dir = DetectRealUserHome();
    }
    if (m_config.config_dir.empty()) {
        m_config.config_dir = m_config.home_dir + "/.config";
    }

    m_ctx = duk_create_heap_default();
    IZ_CHECK(m_ctx != nullptr, StatusCode::ErrOutOfMemory);

    // Save self pointer in global stash
    duk_push_global_stash(m_ctx);
    duk_push_pointer(m_ctx, this);
    duk_put_prop_string(m_ctx, -2, "__iz_engine_ptr__");
    duk_pop(m_ctx);

    RegisterBuiltinBindings();
    return StatusCode::Success;
}

void JsEngine::Destroy() {
    if (m_ctx) {
        duk_destroy_heap(m_ctx);
        m_ctx = nullptr;
    }
}

static JsEngine* GetEngineInstance(duk_context* ctx) {
    duk_push_global_stash(ctx);
    duk_get_prop_string(ctx, -1, "__iz_engine_ptr__");
    JsEngine* self = static_cast<JsEngine*>(duk_get_pointer(ctx, -1));
    duk_pop_2(ctx);
    return self;
}

void JsEngine::RegisterBuiltinBindings() {
    IZ_ASSERT(m_ctx != nullptr);

    // Create global object 'iz'
    duk_push_object(m_ctx);

    // String properties
    duk_push_string(m_ctx, m_config.app_name.c_str());
    duk_put_prop_string(m_ctx, -2, "app_name");

    duk_push_string(m_ctx, m_config.version.c_str());
    duk_put_prop_string(m_ctx, -2, "version");

    duk_push_string(m_ctx, m_config.install_dir.c_str());
    duk_put_prop_string(m_ctx, -2, "install_dir");

    duk_push_string(m_ctx, m_config.home_dir.c_str());
    duk_put_prop_string(m_ctx, -2, "home_dir");

    duk_push_string(m_ctx, m_config.config_dir.c_str());
    duk_put_prop_string(m_ctx, -2, "config_dir");

    duk_push_boolean(m_ctx, m_config.is_uninstall);
    duk_put_prop_string(m_ctx, -2, "is_uninstall");

    // Native methods
    duk_push_c_function(m_ctx, NativeLog, 1);
    duk_put_prop_string(m_ctx, -2, "log");

    duk_push_c_function(m_ctx, NativeMkdir, 1);
    duk_put_prop_string(m_ctx, -2, "mkdir");

    duk_push_c_function(m_ctx, NativeWriteFile, 2);
    duk_put_prop_string(m_ctx, -2, "write_file");

    duk_push_c_function(m_ctx, NativeReadFile, 1);
    duk_put_prop_string(m_ctx, -2, "read_file");

    duk_push_c_function(m_ctx, NativeFileExists, 1);
    duk_put_prop_string(m_ctx, -2, "file_exists");

    duk_push_c_function(m_ctx, NativeRemove, 1);
    duk_put_prop_string(m_ctx, -2, "remove");

    duk_push_c_function(m_ctx, NativeCopy, 2);
    duk_put_prop_string(m_ctx, -2, "copy");

    duk_push_c_function(m_ctx, NativeExec, 1);
    duk_put_prop_string(m_ctx, -2, "exec");

    duk_push_c_function(m_ctx, NativeGetEnv, 1);
    duk_put_prop_string(m_ctx, -2, "get_env");

    duk_push_c_function(m_ctx, NativeSetEnv, 2);
    duk_put_prop_string(m_ctx, -2, "set_env");

    // Set as global 'iz'
    duk_put_global_string(m_ctx, "iz");
}

StatusCode JsEngine::Execute(const std::string& script, std::string* error_out) {
    if (!m_ctx) return StatusCode::ErrInternal;
    if (script.empty()) return StatusCode::Success;

    if (duk_peval_string(m_ctx, script.c_str()) != 0) {
        const char* err_str = duk_safe_to_string(m_ctx, -1);
        if (error_out) {
            *error_out = err_str ? err_str : "Unknown JavaScript execution error";
        }
        if (m_config.log_callback) {
            m_config.log_callback(std::string("[JS ERROR] ") + (err_str ? err_str : "Unknown"));
        }
        duk_pop(m_ctx);
        return StatusCode::ErrScriptFailed;
    }
    duk_pop(m_ctx);
    return StatusCode::Success;
}

int JsEngine::NativeLog(duk_context* ctx) {
    JsEngine* self = GetEngineInstance(ctx);
    if (!self) return 0;
    const char* str = duk_safe_to_string(ctx, 0);
    if (str && self->m_config.log_callback) {
        self->m_config.log_callback(str);
    }
    return 0;
}

int JsEngine::NativeMkdir(duk_context* ctx) {
    JsEngine* self = GetEngineInstance(ctx);
    if (!self) return 0;
    const char* path_raw = duk_safe_to_string(ctx, 0);
    if (!path_raw) {
        duk_push_boolean(ctx, 0);
        return 1;
    }

    std::string path = ExpandPath(path_raw, self->m_config.home_dir);
    bool is_root = (geteuid() == 0);
    uid_t uid = GetRealUserId();
    gid_t gid = GetRealGroupId();

    MakeDirectoryRecursive(path, uid, gid, is_root);

    duk_push_boolean(ctx, 1);
    return 1;
}

int JsEngine::NativeWriteFile(duk_context* ctx) {
    JsEngine* self = GetEngineInstance(ctx);
    if (!self) return 0;
    const char* path_raw = duk_safe_to_string(ctx, 0);
    const char* content = duk_safe_to_string(ctx, 1);
    if (!path_raw || !content) {
        duk_push_boolean(ctx, 0);
        return 1;
    }

    std::string path = ExpandPath(path_raw, self->m_config.home_dir);

    // Ensure parent directory exists
    size_t last_slash = path.find_last_of('/');
    if (last_slash != std::string::npos) {
        std::string parent = path.substr(0, last_slash);
        bool is_root = (geteuid() == 0);
        MakeDirectoryRecursive(parent, GetRealUserId(), GetRealGroupId(), is_root);
    }

    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) {
        duk_push_boolean(ctx, 0);
        return 1;
    }
    out << content;
    out.close();

    if (geteuid() == 0) {
        (void)!chown(path.c_str(), GetRealUserId(), GetRealGroupId());
    }

    duk_push_boolean(ctx, 1);
    return 1;
}

int JsEngine::NativeReadFile(duk_context* ctx) {
    JsEngine* self = GetEngineInstance(ctx);
    if (!self) return 0;
    const char* path_raw = duk_safe_to_string(ctx, 0);
    if (!path_raw) {
        duk_push_null(ctx);
        return 1;
    }

    std::string path = ExpandPath(path_raw, self->m_config.home_dir);
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        duk_push_null(ctx);
        return 1;
    }

    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    in.close();

    duk_push_string(ctx, content.c_str());
    return 1;
}

int JsEngine::NativeFileExists(duk_context* ctx) {
    JsEngine* self = GetEngineInstance(ctx);
    if (!self) return 0;
    const char* path_raw = duk_safe_to_string(ctx, 0);
    if (!path_raw) {
        duk_push_boolean(ctx, 0);
        return 1;
    }
    std::string path = ExpandPath(path_raw, self->m_config.home_dir);
    bool exists = (access(path.c_str(), F_OK) == 0);
    duk_push_boolean(ctx, exists ? 1 : 0);
    return 1;
}

int JsEngine::NativeRemove(duk_context* ctx) {
    JsEngine* self = GetEngineInstance(ctx);
    if (!self) return 0;
    const char* path_raw = duk_safe_to_string(ctx, 0);
    if (!path_raw) {
        duk_push_boolean(ctx, 0);
        return 1;
    }
    std::string path = ExpandPath(path_raw, self->m_config.home_dir);
    int res = ::remove(path.c_str());
    duk_push_boolean(ctx, res == 0 ? 1 : 0);
    return 1;
}

int JsEngine::NativeCopy(duk_context* ctx) {
    JsEngine* self = GetEngineInstance(ctx);
    if (!self) return 0;
    const char* src_raw = duk_safe_to_string(ctx, 0);
    const char* dst_raw = duk_safe_to_string(ctx, 1);
    if (!src_raw || !dst_raw) {
        duk_push_boolean(ctx, 0);
        return 1;
    }

    std::string src = ExpandPath(src_raw, self->m_config.home_dir);
    std::string dst = ExpandPath(dst_raw, self->m_config.home_dir);

    std::ifstream src_file(src, std::ios::binary);
    if (!src_file.is_open()) {
        duk_push_boolean(ctx, 0);
        return 1;
    }
    std::ofstream dst_file(dst, std::ios::binary);
    if (!dst_file.is_open()) {
        duk_push_boolean(ctx, 0);
        return 1;
    }
    dst_file << src_file.rdbuf();
    src_file.close();
    dst_file.close();

    if (geteuid() == 0) {
        (void)!chown(dst.c_str(), GetRealUserId(), GetRealGroupId());
    }

    duk_push_boolean(ctx, 1);
    return 1;
}

int JsEngine::NativeExec(duk_context* ctx) {
    const char* cmd = duk_safe_to_string(ctx, 0);
    if (!cmd) {
        duk_push_object(ctx);
        duk_push_int(ctx, -1);
        duk_put_prop_string(ctx, -2, "exit_code");
        duk_push_string(ctx, "");
        duk_put_prop_string(ctx, -2, "stdout");
        return 1;
    }

    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        duk_push_object(ctx);
        duk_push_int(ctx, -1);
        duk_put_prop_string(ctx, -2, "exit_code");
        duk_push_string(ctx, "");
        duk_put_prop_string(ctx, -2, "stdout");
        return 1;
    }

    char buffer[512];
    std::string output;
    size_t iterations = 0;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr && iterations++ < kMaxLoopIterations) {
        output += buffer;
        if (output.length() > kMaxScriptSize) break; // bounded
    }
    int status = pclose(pipe);
    int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

    duk_push_object(ctx);
    duk_push_int(ctx, exit_code);
    duk_put_prop_string(ctx, -2, "exit_code");
    duk_push_string(ctx, output.c_str());
    duk_put_prop_string(ctx, -2, "stdout");
    return 1;
}

int JsEngine::NativeGetEnv(duk_context* ctx) {
    const char* key = duk_safe_to_string(ctx, 0);
    if (!key) {
        duk_push_string(ctx, "");
        return 1;
    }
    const char* val = std::getenv(key);
    duk_push_string(ctx, val ? val : "");
    return 1;
}

int JsEngine::NativeSetEnv(duk_context* ctx) {
    const char* key = duk_safe_to_string(ctx, 0);
    const char* val = duk_safe_to_string(ctx, 1);
    if (key && val) {
        setenv(key, val, 1);
        duk_push_boolean(ctx, 1);
    } else {
        duk_push_boolean(ctx, 0);
    }
    return 1;
}

} // namespace iz
