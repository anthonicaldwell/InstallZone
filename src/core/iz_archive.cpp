#include "iz_archive.h"
#include "iz_assert.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

namespace iz {

// Standard IEEE 802.3 CRC32 lookup table
static const uint32_t s_crc32_table[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t Archive::CalculateCRC32(const uint8_t* data, size_t length, uint32_t initial_crc) {
    uint32_t crc = ~initial_crc;
    for (size_t i = 0; i < length; ++i) {
        crc = (crc >> 8) ^ s_crc32_table[(crc & 0xFF) ^ data[i]];
    }
    return ~crc;
}

StatusCode Archive::PackStandaloneInstaller(
    const std::string& runner_template_path,
    const Project& project,
    const std::string& output_binary_path,
    ProgressCallback progress_cb,
    LogCallback log_cb)
{
    IZ_CHECK(!runner_template_path.empty(), StatusCode::ErrInvalidArg);
    IZ_CHECK(!output_binary_path.empty(), StatusCode::ErrInvalidArg);

    if (log_cb) log_cb("Opening runner template: " + runner_template_path);
    std::ifstream template_in(runner_template_path, std::ios::binary);
    if (!template_in.is_open()) {
        if (log_cb) log_cb("Error: Runner template not found: " + runner_template_path);
        return StatusCode::ErrNotFound;
    }

    std::ofstream out(output_binary_path, std::ios::binary);
    if (!out.is_open()) {
        if (log_cb) log_cb("Error: Could not open output path for writing: " + output_binary_path);
        return StatusCode::ErrIo;
    }

    // 1. Copy base runner binary
    if (log_cb) log_cb("Copying base executable...");
    std::vector<char> buffer(kChunkSize);
    size_t loop_count = 0;
    while (template_in.read(buffer.data(), buffer.size()) || template_in.gcount() > 0) {
        out.write(buffer.data(), template_in.gcount());
        if (++loop_count > kMaxLoopIterations) break; // JPL bounded
    }
    template_in.close();

    // 2. Prepare payload
    std::streampos payload_start_pos = out.tellp();
    uint32_t running_crc = 0;

    // Write Start Marker (16 bytes)
    out.write(kStartMarker, kStartMarkerLen);
    running_crc = CalculateCRC32(reinterpret_cast<const uint8_t*>(kStartMarker), kStartMarkerLen, running_crc);

    // Write Header Version
    uint32_t version = 1;
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));
    running_crc = CalculateCRC32(reinterpret_cast<const uint8_t*>(&version), sizeof(version), running_crc);

    // Write Project JSON Manifest
    std::string manifest_str = project.ToJsonString();
    uint32_t manifest_len = static_cast<uint32_t>(manifest_str.size());
    out.write(reinterpret_cast<const char*>(&manifest_len), sizeof(manifest_len));
    running_crc = CalculateCRC32(reinterpret_cast<const uint8_t*>(&manifest_len), sizeof(manifest_len), running_crc);

    out.write(manifest_str.data(), manifest_len);
    running_crc = CalculateCRC32(reinterpret_cast<const uint8_t*>(manifest_str.data()), manifest_len, running_crc);

    // Write File Entries
    uint32_t num_files = static_cast<uint32_t>(project.files.size());
    out.write(reinterpret_cast<const char*>(&num_files), sizeof(num_files));
    running_crc = CalculateCRC32(reinterpret_cast<const uint8_t*>(&num_files), sizeof(num_files), running_crc);

    if (log_cb) log_cb("Packing " + std::to_string(num_files) + " payload files...");

    for (size_t i = 0; i < project.files.size(); ++i) {
        const auto& f = project.files[i];
        if (progress_cb) {
            float p = static_cast<float>(i) / static_cast<float>(num_files > 0 ? num_files : 1);
            progress_cb(p, f.target_subpath);
        }

        // Subpath
        uint16_t path_len = static_cast<uint16_t>(f.target_subpath.size());
        out.write(reinterpret_cast<const char*>(&path_len), sizeof(path_len));
        running_crc = CalculateCRC32(reinterpret_cast<const uint8_t*>(&path_len), sizeof(path_len), running_crc);

        out.write(f.target_subpath.data(), path_len);
        running_crc = CalculateCRC32(reinterpret_cast<const uint8_t*>(f.target_subpath.data()), path_len, running_crc);

        // Permissions
        uint32_t perms = f.unix_permissions;
        out.write(reinterpret_cast<const char*>(&perms), sizeof(perms));
        running_crc = CalculateCRC32(reinterpret_cast<const uint8_t*>(&perms), sizeof(perms), running_crc);

        // Open source file
        std::ifstream src_file(f.source_path, std::ios::binary | std::ios::ate);
        if (!src_file.is_open()) {
            if (log_cb) log_cb("Warning: Source file not found: " + f.source_path + " (skipping)");
            uint64_t zero_size = 0;
            out.write(reinterpret_cast<const char*>(&zero_size), sizeof(zero_size));
            running_crc = CalculateCRC32(reinterpret_cast<const uint8_t*>(&zero_size), sizeof(zero_size), running_crc);
            continue;
        }

        uint64_t file_size = src_file.tellg();
        src_file.seekg(0, std::ios::beg);

        out.write(reinterpret_cast<const char*>(&file_size), sizeof(file_size));
        running_crc = CalculateCRC32(reinterpret_cast<const uint8_t*>(&file_size), sizeof(file_size), running_crc);

        // Stream file contents
        uint64_t remaining = file_size;
        size_t file_loop = 0;
        while (remaining > 0 && file_loop++ < kMaxLoopIterations) {
            size_t to_read = (remaining < kChunkSize) ? remaining : kChunkSize;
            src_file.read(buffer.data(), to_read);
            size_t read_bytes = src_file.gcount();
            if (read_bytes == 0) break;

            out.write(buffer.data(), read_bytes);
            running_crc = CalculateCRC32(reinterpret_cast<const uint8_t*>(buffer.data()), read_bytes, running_crc);
            remaining -= read_bytes;
        }
        src_file.close();
    }

    std::streampos payload_end_pos = out.tellp();
    uint64_t total_payload_size = static_cast<uint64_t>(payload_end_pos - payload_start_pos);

    // 3. Write Footer
    // Total Payload Size (8 bytes)
    out.write(reinterpret_cast<const char*>(&total_payload_size), sizeof(total_payload_size));

    // CRC32 (4 bytes)
    out.write(reinterpret_cast<const char*>(&running_crc), sizeof(running_crc));

    // Footer Magic (16 bytes)
    out.write(kFooterMagic, kFooterMagicLen);

    out.close();

    // 4. Make executable (chmod 0755)
    chmod(output_binary_path.c_str(), 0755);

    if (progress_cb) progress_cb(1.0f, "Completed");
    if (log_cb) log_cb("Standalone installer successfully generated: " + output_binary_path);

    return StatusCode::Success;
}

bool Archive::HasEmbeddedPayload(const std::string& exe_path, uint64_t* out_payload_offset, uint64_t* out_payload_size) {
    std::ifstream in(exe_path, std::ios::binary | std::ios::ate);
    if (!in.is_open()) return false;

    uint64_t file_size = in.tellg();
    constexpr size_t min_tail = kFooterMagicLen + sizeof(uint32_t) + sizeof(uint64_t);
    if (file_size < min_tail + kStartMarkerLen) return false;

    // Check footer magic
    in.seekg(file_size - kFooterMagicLen);
    char magic[kFooterMagicLen + 1] = {0};
    in.read(magic, kFooterMagicLen);
    if (std::memcmp(magic, kFooterMagic, kFooterMagicLen) != 0) {
        return false;
    }

    // Read CRC and total_payload_size
    in.seekg(file_size - min_tail);
    uint64_t total_payload_size = 0;
    uint32_t stored_crc = 0;
    in.read(reinterpret_cast<char*>(&total_payload_size), sizeof(total_payload_size));
    in.read(reinterpret_cast<char*>(&stored_crc), sizeof(stored_crc));

    if (total_payload_size > file_size - min_tail) return false;

    uint64_t payload_offset = file_size - min_tail - total_payload_size;
    in.seekg(payload_offset);
    char start_marker[kStartMarkerLen + 1] = {0};
    in.read(start_marker, kStartMarkerLen);

    if (std::memcmp(start_marker, kStartMarker, kStartMarkerLen) != 0) {
        return false;
    }

    if (out_payload_offset) *out_payload_offset = payload_offset;
    if (out_payload_size) *out_payload_size = total_payload_size;
    return true;
}

StatusCode Archive::ReadEmbeddedManifest(const std::string& exe_path, Project& project_out) {
    uint64_t payload_offset = 0;
    uint64_t payload_size = 0;
    if (!HasEmbeddedPayload(exe_path, &payload_offset, &payload_size)) {
        return StatusCode::ErrNotFound;
    }

    std::ifstream in(exe_path, std::ios::binary);
    if (!in.is_open()) return StatusCode::ErrIo;

    in.seekg(payload_offset + kStartMarkerLen);
    uint32_t version = 0;
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version != 1) return StatusCode::ErrCorruptData;

    uint32_t manifest_len = 0;
    in.read(reinterpret_cast<char*>(&manifest_len), sizeof(manifest_len));
    if (manifest_len > kMaxScriptSize * 10) return StatusCode::ErrCorruptData;

    std::string manifest_str(manifest_len, '\0');
    in.read(&manifest_str[0], manifest_len);

    return project_out.FromJsonString(manifest_str);
}

StatusCode Archive::ExtractEmbeddedPayload(
    const std::string& exe_path,
    const std::string& destination_dir,
    std::vector<std::string>& installed_files_out,
    ProgressCallback progress_cb,
    LogCallback log_cb)
{
    uint64_t payload_offset = 0;
    uint64_t payload_size = 0;
    if (!HasEmbeddedPayload(exe_path, &payload_offset, &payload_size)) {
        return StatusCode::ErrNotFound;
    }

    std::ifstream in(exe_path, std::ios::binary);
    if (!in.is_open()) return StatusCode::ErrIo;

    in.seekg(payload_offset + kStartMarkerLen + sizeof(uint32_t)); // Skip start marker & version

    uint32_t manifest_len = 0;
    in.read(reinterpret_cast<char*>(&manifest_len), sizeof(manifest_len));
    in.seekg(manifest_len, std::ios::cur); // Skip manifest string

    uint32_t num_files = 0;
    in.read(reinterpret_cast<char*>(&num_files), sizeof(num_files));
    if (num_files > kMaxFiles) return StatusCode::ErrCorruptData;

    if (log_cb) log_cb("Extracting " + std::to_string(num_files) + " files to: " + destination_dir);

    // Create destination directory
    mkdir(destination_dir.c_str(), 0755);

    std::vector<char> buffer(kChunkSize);
    installed_files_out.clear();

    for (uint32_t i = 0; i < num_files; ++i) {
        uint16_t path_len = 0;
        in.read(reinterpret_cast<char*>(&path_len), sizeof(path_len));
        if (path_len == 0 || path_len > kMaxPathLen) return StatusCode::ErrCorruptData;

        std::string subpath(path_len, '\0');
        in.read(&subpath[0], path_len);

        uint32_t perms = 0755;
        in.read(reinterpret_cast<char*>(&perms), sizeof(perms));

        uint64_t file_size = 0;
        in.read(reinterpret_cast<char*>(&file_size), sizeof(file_size));

        std::string target_full_path = destination_dir + "/" + subpath;

        if (progress_cb) {
            float p = static_cast<float>(i) / static_cast<float>(num_files > 0 ? num_files : 1);
            progress_cb(p, subpath);
        }

        // Ensure target parent directory exists
        size_t last_slash = target_full_path.find_last_of('/');
        if (last_slash != std::string::npos) {
            std::string parent_dir = target_full_path.substr(0, last_slash);
            char tmp[kMaxPathLen];
            std::strncpy(tmp, parent_dir.c_str(), sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';
            for (char* p = tmp + 1; *p; ++p) {
                if (*p == '/') {
                    *p = '\0';
                    mkdir(tmp, 0755);
                    *p = '/';
                }
            }
            mkdir(tmp, 0755);
        }

        std::ofstream target_out(target_full_path, std::ios::binary);
        if (!target_out.is_open()) {
            if (log_cb) log_cb("Error: Could not open for writing: " + target_full_path);
            return StatusCode::ErrIo;
        }

        uint64_t remaining = file_size;
        size_t file_loop = 0;
        while (remaining > 0 && file_loop++ < kMaxLoopIterations) {
            size_t to_read = (remaining < kChunkSize) ? remaining : kChunkSize;
            in.read(buffer.data(), to_read);
            size_t bytes_read = in.gcount();
            if (bytes_read == 0) break;

            target_out.write(buffer.data(), bytes_read);
            remaining -= bytes_read;
        }
        target_out.close();

        // Set unix permissions
        chmod(target_full_path.c_str(), perms);

        installed_files_out.push_back(target_full_path);
    }

    if (progress_cb) progress_cb(1.0f, "Completed");
    if (log_cb) log_cb("All payload files successfully extracted.");

    return StatusCode::Success;
}

} // namespace iz
