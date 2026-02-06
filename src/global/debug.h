#ifndef DEBUG_H
#define DEBUG_H
// Mystic C sorceries, I love those

#include "global/global.h"

typedef enum LogLevel {
	ERROR,
	WARNING,
	INFO,
	DEBUG
} LogLevel;

// Should be defined elsewhere. Sets the minimum debug level of logging code to execute
extern LogLevel LOG_LEVEL;

#define LOG(level, fmt, ...) \
    do { \
        if ((level) <= LOG_LEVEL) { \
            fprintf(stderr, "[%s] In %s | " fmt "\n", #level, __func__, ##__VA_ARGS__); \
        } \
    } while(0)

typedef enum DebugSystem {
    DEBUG_NONE = 0,
    DEBUG_SYSTEMS = 1 << 0,
    DEBUG_COLLISIONS = 1 << 1,
    DEBUG_UI = 1 << 2,
    DEBUG_ALL = 0xFFFFFFFF
} DebugSystem;

// Should be defined elsewhere. Sets what types of debug logs are visible
extern Uint32 DEBUG_MASK;

#define SYS_DEBUG(system, fmt, ...) \
    do { \
        if (DEBUG_MASK & (system)) { \
            LOG(DEBUG, fmt, ##__VA_ARGS__); \
        } \
    } while(0)

#define ASSERT(expr, fmt, ...) \
    do { \
        if (!(expr)) { \
            LOG(ERROR, "Assertion failed: %s | " fmt, #expr, ##__VA_ARGS__); \
            abort(); \
        } \
    } while(0)

#endif
