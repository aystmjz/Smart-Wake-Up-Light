#ifndef __LOG_H__
#define __LOG_H__

#include "UART.h"
#include "sys.h"
#include "usart.h"

// 根据构建目标选择日志输出串口
#ifdef BUILD_BOOT_LOADER

// 启动加载程序使用uart1_printf
#define LOG_PRINTF uart1_printf
#else

// 应用程序使用Debug_printf
#define LOG_PRINTF Debug_printf
#endif

/**
 * @brief 输出DEBUG级别日志
 * @param fmt 格式化字符串
 * @param ... 可变参数
 * @details 只有当DEBUG_LEVEL小于等于LOG_LEVEL_DEBUG时才输出日志
 */
#define LOG_DEBUG(fmt, ...)                            \
    do                                                 \
    {                                                  \
        if (DEBUG_LEVEL <= LOG_LEVEL_DEBUG)            \
        {                                              \
            LOG_PRINTF("[DEBUG] " fmt, ##__VA_ARGS__); \
        }                                              \
    } while (0)

/**
 * @brief 输出INFO级别日志
 * @param fmt 格式化字符串
 * @param ... 可变参数
 * @details 只有当DEBUG_LEVEL小于等于LOG_LEVEL_INFO时才输出日志
 */
#define LOG_INFO(fmt, ...)                            \
    do                                                \
    {                                                 \
        if (DEBUG_LEVEL <= LOG_LEVEL_INFO)            \
        {                                             \
            LOG_PRINTF("[INFO] " fmt, ##__VA_ARGS__); \
        }                                             \
    } while (0)

/**
 * @brief 输出WARN级别日志
 * @param fmt 格式化字符串
 * @param ... 可变参数
 * @details 只有当DEBUG_LEVEL小于等于LOG_LEVEL_WARN时才输出日志
 */
#define LOG_WARN(fmt, ...)                            \
    do                                                \
    {                                                 \
        if (DEBUG_LEVEL <= LOG_LEVEL_WARN)            \
        {                                             \
            LOG_PRINTF("[WARN] " fmt, ##__VA_ARGS__); \
        }                                             \
    } while (0)

/**
 * @brief 输出ERROR级别日志
 * @param fmt 格式化字符串
 * @param ... 可变参数
 * @details 只有当DEBUG_LEVEL小于等于LOG_LEVEL_ERROR时才输出日志
 */
#define LOG_ERROR(fmt, ...)                            \
    do                                                 \
    {                                                  \
        if (DEBUG_LEVEL <= LOG_LEVEL_ERROR)            \
        {                                              \
            LOG_PRINTF("[ERROR] " fmt, ##__VA_ARGS__); \
        }                                              \
    } while (0)

#endif
