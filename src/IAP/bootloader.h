#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#include "sys.h"
#include "W25Q128.h"
#include "stmflash.h"
#include "usart.h"
#include "log.h"
#include "Delay.h"

#define SOH 0x01 // 数据包开始标志
#define EOT 0x04 // 传输结束
#define ACK 0x06 // 确认响应
#define NAK 0x15 // 非确认响应
#define CAN 0x18 // 取消传输

#define XMODEM_PACKET_SIZE 128
#define XMODEM_IAP_BUF_SIZE 1024

#define FLASH_BASE_ADDR STM32_FLASH_BASE                     /*!< Flash基地址 */
#define FLASH_END_ADDR (STM32_FLASH_BASE + STM32_FLASH_SIZE) /*!< Flash结束地址(64KB) */

#define BOOTLOADER_SIZE BOOTLOADER_SIZE_SYS /*!< Bootloader大小 */
#define BOOTLOADER_ADDR FLASH_BASE_ADDR     /*!< Bootloader起始地址 */

#define APP_ADDR (BOOTLOADER_ADDR + BOOTLOADER_SIZE) /*!< APP起始地址 */
#define APP_SIZE (FLASH_END_ADDR - APP_ADDR + 1)     /*!< APP可用空间 */

#define VTOR_ADDR APP_ADDR /* 向量表偏移 */

#define APP_STACK_ADDR (APP_ADDR)             /*!< APP栈指针地址 */
#define APP_RESET_HANDLER_ADDR (APP_ADDR + 4) /*!< APP复位处理函数地址 */

#define OTA_VALID 0x56
#define OTA_FLAG_ADDRESS 0xFFE000
#define APP_NUM 10
#define APP_VALID 0x5A

typedef enum
{
    STATE_IDLE = 0,        // 系统初始化
    STATE_OTA,             // OTA准备就绪
    STATE_MUNU,            // 进入命令行
    STATE_SYSTEM,          // 命令行系统处理
    STATE_JUMP_TO_APP,     // 跳转到APP
    STATE_ERROR            // 错误状态
} BOOTLOADER_STATE;

typedef struct
{
    uint8_t Block_NUM;
    uint16_t APP_Size;
    uint8_t APP_Flag;
} APP_INFO;

typedef struct
{
    uint8_t OTA_FLAG;
    uint8_t OTA_INDEX;
    APP_INFO APP[APP_NUM];
} OTA_INFO;

#endif