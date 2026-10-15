#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <string>
#include <cstdlib>

/**
 * @file jpl_defs.h
 * @brief JPL Institutional Coding Standard safety definitions and bounded limits.
 *
 * NASA/JPL "Power of 10" Coding Rules compliance:
 * Rule 1: Simple control flow (no goto, setjmp, recursion).
 * Rule 2: Fixed upper bounds on all loops.
 * Rule 3: Bounded memory; no unbounded dynamic growth.
 * Rule 4: Modular, readable functions.
 * Rule 5: Heavy assertion density checking parameters and invariants.
 * Rule 6: Variable declaration at smallest scope.
 * Rule 7: Check return value of every non-void function.
 * Rule 8: Preprocessor limited to include guards and safe macros.
 * Rule 9: Restricted pointer arithmetic and strict bounds checking.
 * Rule 10: Compile with all warnings enabled and zero errors.
 */

namespace iz {

// Bounded limits for safety
constexpr size_t kMaxPathLen        = 4096;
constexpr size_t kMaxNameLen        = 256;
constexpr size_t kMaxDescLen        = 2048;
constexpr size_t kMaxFiles          = 65536;
constexpr size_t kMaxDependencies   = 256;
constexpr size_t kMaxLogLines       = 10000;
constexpr size_t kMaxLogLineLen     = 1024;
constexpr size_t kMaxPayloadSize    = 4ULL * 1024ULL * 1024ULL * 1024ULL; // 4 GB safety cap
constexpr size_t kMaxScriptSize     = 1024 * 1024; // 1 MB script cap
constexpr size_t kMaxLoopIterations = 1000000;

enum class StatusCode : int32_t {
    Success = 0,
    ErrInvalidArg = -1,
    ErrNotFound = -2,
    ErrPermission = -3,
    ErrIo = -4,
    ErrCorruptData = -5,
    ErrOutOfMemory = -6,
    ErrTimeout = -7,
    ErrScriptFailed = -8,
    ErrCancelled = -9,
    ErrInternal = -10
};

inline const char* StatusCodeToString(StatusCode code) {
    switch (code) {
        case StatusCode::Success:         return "Success";
        case StatusCode::ErrInvalidArg:   return "Invalid Argument";
        case StatusCode::ErrNotFound:     return "Not Found";
        case StatusCode::ErrPermission:   return "Permission Denied";
        case StatusCode::ErrIo:           return "I/O Error";
        case StatusCode::ErrCorruptData:  return "Corrupt Data or Hash Mismatch";
        case StatusCode::ErrOutOfMemory:  return "Out of Memory";
        case StatusCode::ErrTimeout:      return "Operation Timed Out";
        case StatusCode::ErrScriptFailed: return "Script Execution Failed";
        case StatusCode::ErrCancelled:    return "Operation Cancelled";
        case StatusCode::ErrInternal:     return "Internal System Error";
        default:                          return "Unknown Error";
    }
}

inline int IzSystem(const std::string& cmd) {
    int ret = std::system(cmd.c_str());
    return ret;
}

} // namespace iz
