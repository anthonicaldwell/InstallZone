#pragma once

#include "jpl_defs.h"
#include "iz_project.h"
#include <string>
#include <vector>
#include <functional>

namespace iz {

using ProgressCallback = std::function<void(float progress, const std::string& current_item)>;
using LogCallback = std::function<void(const std::string& message)>;

class Archive {
public:
    static constexpr const char* kStartMarker = "__IZ_PAYLOAD__\0\0";
    static constexpr const char* kFooterMagic = "__INSTALLZONE_V1__";
    static constexpr size_t kStartMarkerLen = 16;
    static constexpr size_t kFooterMagicLen = 16;
    static constexpr size_t kChunkSize = 65536; // 64 KB streaming buffer

    /**
     * @brief Packs runner binary, project manifest, and payload files into standalone .run binary.
     */
    static StatusCode PackStandaloneInstaller(
        const std::string& runner_template_path,
        const Project& project,
        const std::string& output_binary_path,
        ProgressCallback progress_cb = nullptr,
        LogCallback log_cb = nullptr
    );

    /**
     * @brief Checks if executable contains embedded payload.
     */
    static bool HasEmbeddedPayload(const std::string& exe_path, uint64_t* out_payload_offset = nullptr, uint64_t* out_payload_size = nullptr);

    /**
     * @brief Reads project manifest from embedded payload inside executable.
     */
    static StatusCode ReadEmbeddedManifest(
        const std::string& exe_path,
        Project& project_out
    );

    /**
     * @brief Extracts embedded payload files into destination directory.
     */
    static StatusCode ExtractEmbeddedPayload(
        const std::string& exe_path,
        const std::string& destination_dir,
        std::vector<std::string>& installed_files_out,
        ProgressCallback progress_cb = nullptr,
        LogCallback log_cb = nullptr
    );

private:
    static uint32_t CalculateCRC32(const uint8_t* data, size_t length, uint32_t initial_crc = 0);
};

} // namespace iz
