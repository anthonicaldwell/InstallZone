#include "iz_cmake_scanner.h"
#include "iz_assert.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace iz {

static std::string ToLower(const std::string& str) {
    std::string s = str;
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return s;
}

static std::string Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::string CMakeScanner::MapCMakePackageToApt(const std::string& cmake_pkg) {
    std::string lower = ToLower(cmake_pkg);

    static const std::unordered_map<std::string, std::string> s_apt_map = {
        {"sdl2", "libsdl2-dev"},
        {"opengl", "libgl-dev"},
        {"gl", "libgl-dev"},
        {"glew", "libglew-dev"},
        {"glfw", "libglfw3-dev"},
        {"glfw3", "libglfw3-dev"},
        {"vulkan", "libvulkan-dev"},
        {"openssl", "libssl-dev"},
        {"curl", "libcurl4-openssl-dev"},
        {"zlib", "zlib1g-dev"},
        {"png", "libpng-dev"},
        {"jpeg", "libjpeg-dev"},
        {"freetype", "libfreetype-dev"},
        {"fontconfig", "libfontconfig-dev"},
        {"gtk3", "libgtk-3-dev"},
        {"gtk4", "libgtk-4-dev"},
        {"qt5", "qtbase5-dev"},
        {"qt6", "qt6-base-dev"},
        {"boost", "libboost-all-dev"},
        {"sqlite3", "libsqlite3-dev"},
        {"pkgconfig", "pkg-config"},
        {"navi", "navi"}
    };

    auto it = s_apt_map.find(lower);
    if (it != s_apt_map.end()) {
        return it->second;
    }
    return lower;
}

// Tokenizer for CMake arguments
static std::vector<std::string> TokenizeArgs(const std::string& args_str) {
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;
    char quote_char = '\0';

    for (size_t i = 0; i < args_str.size(); ++i) {
        char c = args_str[i];

        if (in_quotes) {
            if (c == quote_char && (i == 0 || args_str[i - 1] != '\\')) {
                in_quotes = false;
                tokens.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        } else {
            if (c == '"' || c == '\'') {
                in_quotes = true;
                quote_char = c;
            } else if (std::isspace(static_cast<unsigned char>(c))) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }
    return tokens;
}

CMakeScanResult CMakeScanner::ScanFile(const std::string& cmake_file_path) {
    CMakeScanResult result;
    std::ifstream in(cmake_file_path);
    if (!in.is_open()) {
        result.scan_log.push_back("Error: Could not open CMake file: " + cmake_file_path);
        return result;
    }

    result.scan_log.push_back("Scanning CMake file: " + cmake_file_path);

    std::string line;
    std::string accumulator;
    bool in_command = false;
    std::string current_cmd_name;

    size_t line_count = 0;
    while (std::getline(in, line) && line_count++ < kMaxLoopIterations) {
        // Strip comments outside quotes
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        line = Trim(line);
        if (line.empty()) continue;

        if (!in_command) {
            size_t paren_open = line.find('(');
            if (paren_open != std::string::npos) {
                current_cmd_name = ToLower(Trim(line.substr(0, paren_open)));
                size_t paren_close = line.find(')', paren_open);
                if (paren_close != std::string::npos) {
                    // Complete single-line command
                    std::string args = line.substr(paren_open + 1, paren_close - (paren_open + 1));
                    auto tokens = TokenizeArgs(args);

                    // Process command
                    if (current_cmd_name == "project" && !tokens.empty()) {
                        result.project_name = tokens[0];
                        for (size_t t = 1; t < tokens.size(); ++t) {
                            std::string upper_tok = tokens[t];
                            std::transform(upper_tok.begin(), upper_tok.end(), upper_tok.begin(), ::toupper);
                            if (upper_tok == "VERSION" && t + 1 < tokens.size()) {
                                result.version = tokens[++t];
                            } else if (upper_tok == "DESCRIPTION" && t + 1 < tokens.size()) {
                                result.description = tokens[++t];
                            } else if (upper_tok == "HOMEPAGE_URL" && t + 1 < tokens.size()) {
                                result.homepage_url = tokens[++t];
                            }
                        }
                        result.scan_log.push_back("Found project: " + result.project_name + " (Version: " + result.version + ")");
                    } else if (current_cmd_name == "add_executable" && !tokens.empty()) {
                        std::string target = tokens[0];
                        if (std::find(result.executable_targets.begin(), result.executable_targets.end(), target) == result.executable_targets.end()) {
                            result.executable_targets.push_back(target);
                            result.scan_log.push_back("Found executable target: " + target);
                        }
                    } else if (current_cmd_name == "find_package" && !tokens.empty()) {
                        std::string pkg = tokens[0];
                        if (ToLower(pkg) != "threads") {
                            DependencyEntry dep;
                            dep.name = pkg;
                            dep.package_name = MapCMakePackageToApt(pkg);
                            dep.check_command = "which " + ToLower(pkg);
                            dep.is_required = false;
                            for (size_t t = 1; t < tokens.size(); ++t) {
                                std::string upper = tokens[t];
                                std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
                                if (upper == "REQUIRED") dep.is_required = true;
                            }
                            result.dependencies.push_back(dep);
                            result.scan_log.push_back("Found dependency (find_package): " + pkg + " -> apt: " + dep.package_name);
                        }
                    } else if (current_cmd_name == "pkg_check_modules" && tokens.size() >= 2) {
                        for (size_t t = 1; t < tokens.size(); ++t) {
                            std::string tok = tokens[t];
                            std::string upper = tok;
                            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
                            if (upper == "REQUIRED" || upper == "QUIET" || upper == "IMPORTED_TARGET") continue;

                            DependencyEntry dep;
                            dep.name = tok;
                            dep.package_name = MapCMakePackageToApt(tok);
                            dep.check_command = "pkg-config --exists " + tok;
                            dep.is_required = true;
                            result.dependencies.push_back(dep);
                            result.scan_log.push_back("Found pkg-config dependency: " + tok + " -> apt: " + dep.package_name);
                        }
                    } else if (current_cmd_name == "install") {
                        // Check install TARGETS or FILES
                        std::string dest = "bin";
                        for (size_t t = 0; t < tokens.size(); ++t) {
                            std::string upper = tokens[t];
                            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
                            if (upper == "DESTINATION" && t + 1 < tokens.size()) {
                                dest = tokens[t + 1];
                                break;
                            }
                        }
                        if (!tokens.empty()) {
                            std::string first_upper = tokens[0];
                            std::transform(first_upper.begin(), first_upper.end(), first_upper.begin(), ::toupper);
                            if (first_upper == "TARGETS" && tokens.size() >= 2) {
                                for (size_t t = 1; t < tokens.size(); ++t) {
                                    std::string u = tokens[t];
                                    std::transform(u.begin(), u.end(), u.begin(), ::toupper);
                                    if (u == "RUNTIME" || u == "DESTINATION" || u == "LIBRARY" || u == "ARCHIVE") break;

                                    FileEntry fe;
                                    fe.source_path = "build/" + tokens[t];
                                    fe.target_subpath = dest + "/" + tokens[t];
                                    fe.unix_permissions = 0755;
                                    result.install_files.push_back(fe);
                                    result.scan_log.push_back("Found install target: " + tokens[t] + " -> " + fe.target_subpath);
                                }
                            } else if (first_upper == "FILES" && tokens.size() >= 2) {
                                for (size_t t = 1; t < tokens.size(); ++t) {
                                    std::string u = tokens[t];
                                    std::transform(u.begin(), u.end(), u.begin(), ::toupper);
                                    if (u == "DESTINATION") break;

                                    FileEntry fe;
                                    fe.source_path = tokens[t];
                                    std::string filename = tokens[t];
                                    size_t s = filename.find_last_of('/');
                                    if (s != std::string::npos) filename = filename.substr(s + 1);

                                    fe.target_subpath = dest + "/" + filename;
                                    fe.unix_permissions = 0644;
                                    result.install_files.push_back(fe);
                                    result.scan_log.push_back("Found install file: " + tokens[t] + " -> " + fe.target_subpath);
                                }
                            }
                        }
                    }
                } else {
                    // Multi-line command begins
                    in_command = true;
                    accumulator = line.substr(paren_open + 1) + " ";
                }
            }
        } else {
            size_t paren_close = line.find(')');
            if (paren_close != std::string::npos) {
                accumulator += line.substr(0, paren_close);
                in_command = false;
                auto tokens = TokenizeArgs(accumulator);

                if (current_cmd_name == "project" && !tokens.empty()) {
                    result.project_name = tokens[0];
                    for (size_t t = 1; t < tokens.size(); ++t) {
                        std::string upper_tok = tokens[t];
                        std::transform(upper_tok.begin(), upper_tok.end(), upper_tok.begin(), ::toupper);
                        if (upper_tok == "VERSION" && t + 1 < tokens.size()) {
                            result.version = tokens[++t];
                        } else if (upper_tok == "DESCRIPTION" && t + 1 < tokens.size()) {
                            result.description = tokens[++t];
                        } else if (upper_tok == "HOMEPAGE_URL" && t + 1 < tokens.size()) {
                            result.homepage_url = tokens[++t];
                        }
                    }
                    result.scan_log.push_back("Found project: " + result.project_name + " (Version: " + result.version + ")");
                } else if (current_cmd_name == "add_executable" && !tokens.empty()) {
                    std::string target = tokens[0];
                    if (std::find(result.executable_targets.begin(), result.executable_targets.end(), target) == result.executable_targets.end()) {
                        result.executable_targets.push_back(target);
                        result.scan_log.push_back("Found executable target: " + target);
                    }
                } else if (current_cmd_name == "find_package" && !tokens.empty()) {
                    std::string pkg = tokens[0];
                    if (ToLower(pkg) != "threads") {
                        DependencyEntry dep;
                        dep.name = pkg;
                        dep.package_name = MapCMakePackageToApt(pkg);
                        dep.check_command = "which " + ToLower(pkg);
                        dep.is_required = false;
                        for (size_t t = 1; t < tokens.size(); ++t) {
                            std::string upper = tokens[t];
                            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
                            if (upper == "REQUIRED") dep.is_required = true;
                        }
                        result.dependencies.push_back(dep);
                        result.scan_log.push_back("Found dependency (find_package): " + pkg + " -> apt: " + dep.package_name);
                    }
                } else if (current_cmd_name == "install") {
                    std::string dest = "bin";
                    for (size_t t = 0; t < tokens.size(); ++t) {
                        std::string upper = tokens[t];
                        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
                        if (upper == "DESTINATION" && t + 1 < tokens.size()) {
                            dest = tokens[t + 1];
                            break;
                        }
                    }
                    if (!tokens.empty()) {
                        std::string first_upper = tokens[0];
                        std::transform(first_upper.begin(), first_upper.end(), first_upper.begin(), ::toupper);
                        if (first_upper == "TARGETS" && tokens.size() >= 2) {
                            for (size_t t = 1; t < tokens.size(); ++t) {
                                std::string u = tokens[t];
                                std::transform(u.begin(), u.end(), u.begin(), ::toupper);
                                if (u == "RUNTIME" || u == "DESTINATION" || u == "LIBRARY" || u == "ARCHIVE") break;

                                FileEntry fe;
                                fe.source_path = "build/" + tokens[t];
                                fe.target_subpath = dest + "/" + tokens[t];
                                fe.unix_permissions = 0755;
                                result.install_files.push_back(fe);
                                result.scan_log.push_back("Found install target: " + tokens[t] + " -> " + fe.target_subpath);
                            }
                        }
                    }
                }
                accumulator.clear();
            } else {
                accumulator += line + " ";
            }
        }
    }

    in.close();

    // If install_files is empty but we have executable_targets, add default bin file entry
    if (result.install_files.empty() && !result.executable_targets.empty()) {
        for (const auto& target : result.executable_targets) {
            FileEntry fe;
            fe.source_path = "build/" + target;
            fe.target_subpath = "bin/" + target;
            fe.unix_permissions = 0755;
            result.install_files.push_back(fe);
        }
    }

    result.success = (!result.project_name.empty() || !result.executable_targets.empty());
    return result;
}

StatusCode CMakeScanner::ApplyToProject(const std::string& cmake_file_path, Project& project, std::vector<std::string>* log_out) {
    CMakeScanResult res = ScanFile(cmake_file_path);
    if (!res.success) {
        if (log_out) {
            *log_out = res.scan_log;
            log_out->push_back("Scan failed: No project or executable targets found.");
        }
        return StatusCode::ErrNotFound;
    }

    if (!res.project_name.empty()) {
        project.app_name = res.project_name;
        project.app_id = "org." + ToLower(res.project_name);
        project.default_prefix = "/usr/bin";
    }
    if (!res.version.empty()) {
        project.version = res.version;
    }
    if (!res.description.empty()) {
        project.description = res.description;
    }

    // Set primary executable and desktop entry
    if (!res.executable_targets.empty()) {
        project.desktop.name = project.app_name;
        project.desktop.exec_cmd = res.executable_targets[0];
    }

    // Merge discovered files
    for (const auto& f : res.install_files) {
        bool exists = false;
        for (const auto& existing : project.files) {
            if (existing.target_subpath == f.target_subpath) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            project.files.push_back(f);
        }
    }

    // Merge discovered dependencies
    for (const auto& dep : res.dependencies) {
        bool exists = false;
        for (const auto& existing : project.dependencies) {
            if (existing.name == dep.name || existing.package_name == dep.package_name) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            project.dependencies.push_back(dep);
        }
    }

    if (log_out) {
        *log_out = res.scan_log;
        log_out->push_back("Successfully applied CMake scan results to project.");
    }

    return StatusCode::Success;
}

} // namespace iz
