#ifndef __SYS_H
#define __SYS_H

#include "stm32f10x.h"

/**
 * BUILD_BOOT_LOADER 宏定义用于控制是否构建引导加载程序
 * 编译程序时请注意更改ROM地址,并修改此宏定义
 * -当定义此宏时，编译器会构建引导加载程序版本
 * -当注释掉此宏时，编译器会构建应用程序版本
 */
// #define BUILD_BOOT_LOADER

// 启动标志地址，用于在内存中设置启动标志以跳转到特定启动模式
#define BOOT_FLAG_ADDR      (0x20003FF0)
// 启动标志值，用于标识有效的启动请求的魔数
#define BOOT_FLAG_VALUE     (0xDEADBEEF)
// 引导程序大小，定义引导程序占用的Flash空间大小为0x4800字节
#define BOOTLOADER_SIZE_SYS 0x4800

// 调试串口波特率设置
#define DEBUG_BAUD          115200

// 设置当前调试模式 0:关闭调试 1:正常调试 2:STM32调试 3:ASRPRO调试 4:BT24-T调试
#define DEBUG_MODE_NONE     0 // 关闭调试
#define DEBUG_MODE_NORMAL   1 // 正常调试
#define DEBUG_MODE_STM32    2 // STM32调试
#define DEBUG_MODE_ASRPRO   3 // ASRPRO调试
#define DEBUG_MODE_BT24     4 // BT24-T调试

// 当前使用的调试模式
#define DEBUG_MODE          DEBUG_MODE_NORMAL

// 日志级别定义，数字越小级别越高
#define LOG_LEVEL_DEBUG     0 // 调试级别，输出最详细的调试信息
#define LOG_LEVEL_INFO      1 // 信息级别，输出一般信息
#define LOG_LEVEL_WARN      2 // 警告级别，输出警告信息
#define LOG_LEVEL_ERROR     3 // 错误级别，只输出错误信息

// 当前调试等级
#define DEBUG_LEVEL         LOG_LEVEL_DEBUG

// 位带操作,实现51类似的GPIO控制功能
// 具体实现思想,参考<<CM3权威指南>>第五章(87页~92页).
// IO口操作宏定义
#define BITBAND(addr, bitnum) \
    ((addr & 0xF0000000) + 0x2000000 + ((addr & 0xFFFFF) << 5) + (bitnum << 2))
#define MEM_ADDR(addr)         *((volatile unsigned long *)(addr))
#define BIT_ADDR(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))
// IO口地址映射
#define GPIOA_ODR_Addr         (GPIOA_BASE + 12) // 0x4001080C
#define GPIOB_ODR_Addr         (GPIOB_BASE + 12) // 0x40010C0C
#define GPIOC_ODR_Addr         (GPIOC_BASE + 12) // 0x4001100C
#define GPIOD_ODR_Addr         (GPIOD_BASE + 12) // 0x4001140C
#define GPIOE_ODR_Addr         (GPIOE_BASE + 12) // 0x4001180C
#define GPIOF_ODR_Addr         (GPIOF_BASE + 12) // 0x40011A0C
#define GPIOG_ODR_Addr         (GPIOG_BASE + 12) // 0x40011E0C

#define GPIOA_IDR_Addr         (GPIOA_BASE + 8) // 0x40010808
#define GPIOB_IDR_Addr         (GPIOB_BASE + 8) // 0x40010C08
#define GPIOC_IDR_Addr         (GPIOC_BASE + 8) // 0x40011008
#define GPIOD_IDR_Addr         (GPIOD_BASE + 8) // 0x40011408
#define GPIOE_IDR_Addr         (GPIOE_BASE + 8) // 0x40011808
#define GPIOF_IDR_Addr         (GPIOF_BASE + 8) // 0x40011A08
#define GPIOG_IDR_Addr         (GPIOG_BASE + 8) // 0x40011E08

// IO口操作,只对单一的IO口!
// 确保n的值小于16!
#define PAout(n)               BIT_ADDR(GPIOA_ODR_Addr, n) // 输出
#define PAin(n)                BIT_ADDR(GPIOA_IDR_Addr, n) // 输入

#define PBout(n)               BIT_ADDR(GPIOB_ODR_Addr, n) // 输出
#define PBin(n)                BIT_ADDR(GPIOB_IDR_Addr, n) // 输入

#define PCout(n)               BIT_ADDR(GPIOC_ODR_Addr, n) // 输出
#define PCin(n)                BIT_ADDR(GPIOC_IDR_Addr, n) // 输入

#define PDout(n)               BIT_ADDR(GPIOD_ODR_Addr, n) // 输出
#define PDin(n)                BIT_ADDR(GPIOD_IDR_Addr, n) // 输入

#define PEout(n)               BIT_ADDR(GPIOE_ODR_Addr, n) // 输出
#define PEin(n)                BIT_ADDR(GPIOE_IDR_Addr, n) // 输入

#define PFout(n)               BIT_ADDR(GPIOF_ODR_Addr, n) // 输出
#define PFin(n)                BIT_ADDR(GPIOF_IDR_Addr, n) // 输入

#define PGout(n)               BIT_ADDR(GPIOG_ODR_Addr, n) // 输出
#define PGin(n)                BIT_ADDR(GPIOG_IDR_Addr, n) // 输入

#endif
