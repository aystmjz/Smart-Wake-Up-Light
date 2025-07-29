#ifndef __LOG_H__
#define __LOG_H__

#include "sys.h"
#include "UART.h"
#include "usart.h"

#ifdef BUILD_BOOT_LOADER
#define LOG_PRINTF uart1_printf
#else
#define LOG_PRINTF Debug_printf
#endif

#define LOG_DEBUG(fmt, ...) do { \
    if (DEBUG_LEVEL <= LOG_LEVEL_DEBUG) { \
        LOG_PRINTF("[DEBUG] " fmt, ##__VA_ARGS__); \
    } \
} while(0)

#define LOG_INFO(fmt, ...) do { \
    if (DEBUG_LEVEL <= LOG_LEVEL_INFO) { \
        LOG_PRINTF("[INFO] " fmt, ##__VA_ARGS__); \
    } \
} while(0)

#define LOG_WARN(fmt, ...) do { \
    if (DEBUG_LEVEL <= LOG_LEVEL_WARN) { \
        LOG_PRINTF("[WARN] " fmt, ##__VA_ARGS__); \
    } \
} while(0)

#define LOG_ERROR(fmt, ...) do { \
    if (DEBUG_LEVEL <= LOG_LEVEL_ERROR) { \
        LOG_PRINTF("[ERROR] " fmt, ##__VA_ARGS__); \
    } \
} while(0)

#endif
