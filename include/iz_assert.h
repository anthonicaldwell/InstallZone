#pragma once

#include "jpl_defs.h"
#include <cstdio>
#include <cstdlib>

/**
 * @def IZ_ASSERT
 * @brief JPL-compliant assertion macro with file, line, and expression logging.
 */
#ifdef NDEBUG
#define IZ_ASSERT(expr) ((void)0)
#else
#define IZ_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            std::fprintf(stderr, "[IZ_ASSERT_FAIL] %s:%d: assertion failed: %s\n", \
                         __FILE__, __LINE__, #expr); \
            std::abort(); \
        } \
    } while (0)
#endif

/**
 * @def IZ_CHECK
 * @brief Runtime check macro that returns a StatusCode on failure without crashing.
 */
#define IZ_CHECK(expr, code) \
    do { \
        if (!(expr)) { \
            std::fprintf(stderr, "[IZ_CHECK_FAIL] %s:%d: condition '%s' failed, returning %d\n", \
                         __FILE__, __LINE__, #expr, static_cast<int>(code)); \
            return code; \
        } \
    } while (0)

#define IZ_CHECK_VOID(expr) \
    do { \
        if (!(expr)) { \
            std::fprintf(stderr, "[IZ_CHECK_FAIL] %s:%d: condition '%s' failed\n", \
                         __FILE__, __LINE__, #expr); \
            return; \
        } \
    } while (0)
