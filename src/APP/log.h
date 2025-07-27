#ifndef __LOG_H__
#define __LOG_H__

#include "sys.h"
#include "UART.h"

#define LOG_DEBUG(fmt, ...) do { \
    if (DEBUG_LEVEL <= LOG_LEVEL_DEBUG) { \
        Debug_printf("[DEBUG] " fmt, ##__VA_ARGS__); \
    } \
} while(0)

#define LOG_INFO(fmt, ...) do { \
    if (DEBUG_LEVEL <= LOG_LEVEL_INFO) { \
        Debug_printf("[INFO] " fmt, ##__VA_ARGS__); \
    } \
} while(0)

#define LOG_WARN(fmt, ...) do { \
    if (DEBUG_LEVEL <= LOG_LEVEL_WARN) { \
        Debug_printf("[WARN] " fmt, ##__VA_ARGS__); \
    } \
} while(0)

#define LOG_ERROR(fmt, ...) do { \
    if (DEBUG_LEVEL <= LOG_LEVEL_ERROR) { \
        Debug_printf("[ERROR] " fmt, ##__VA_ARGS__); \
    } \
} while(0)

#endif
