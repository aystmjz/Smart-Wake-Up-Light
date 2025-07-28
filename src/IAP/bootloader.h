#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#include "sys.h"
#include "log.h"
#include "W25Q128.h"
#include "stmflash.h"

/* Bootloader和APP地址定义 */
#define FLASH_BASE_ADDR 0x08000000UL /*!< Flash基地址 */
#define FLASH_END_ADDR 0x0800FFFFUL  /*!< Flash结束地址(64KB) */

/* Bootloader相关定义 */
#define BOOTLOADER_SIZE 0x1C00          /*!< Bootloader大小 */
#define BOOTLOADER_ADDR FLASH_BASE_ADDR /*!< Bootloader起始地址 */

/* APP相关定义 */
#define APP_ADDR (BOOTLOADER_ADDR + BOOTLOADER_SIZE) /*!< APP起始地址 */
#define APP_SIZE (FLASH_END_ADDR - APP_ADDR + 1)     /*!< APP可用空间 */

/* 向量表偏移 */
#define VTOR_ADDR APP_ADDR /*!< 向量表地址 */

/* 栈指针和复位处理函数地址 */
#define APP_STACK_ADDR (APP_ADDR)             /*!< APP栈指针地址 */
#define APP_RESET_HANDLER_ADDR (APP_ADDR + 4) /*!< APP复位处理函数地址 */

#define OTA_SET_FLAG 0x56
#define OTA_FLAG_ADDRESS 0xFFE000
#define APP_NUM 10

typedef struct
{
    uint8_t Block_NUM;
    uint16_t APP_Size;
} APP_INFO;

typedef struct
{
    uint8_t OTA_FLAG;
    uint8_t OTA_INDEX;
    APP_INFO APP[APP_NUM];
} OTA_INFO;

void bootloader_main(void);

#endif