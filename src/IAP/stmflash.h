#ifndef __STMFLASH_H
#define __STMFLASH_H

#include "sys.h"
#include "Delay.h"


/* FLASH起始地址 */
#define STM32_FLASH_SIZE        0x10000         /* STM32 FLASH 总大小 */
#define STM32_FLASH_BASE        0x08000000      /* STM32 FLASH 起始地址 */

 /* STM32F103 扇区大小 */
#if STM32_FLASH_SIZE < 256 * 1024
#define STM32_SECTOR_SIZE   1024                /* 容量小于256K的 F103, 扇区大小为1K字节 */
#else
#define STM32_SECTOR_SIZE   2048                /* 容量大于等于于256K的 F103, 扇区大小为2K字节 */
#endif

/* FLASH解锁键值 */
#define STM32_FLASH_KEY1        0X45670123
#define STM32_FLASH_KEY2        0XCDEF89AB

/* 接口函数(外部可调用) */
uint16_t stmflash_read_halfword(uint32_t faddr);                        /* FLASH读半字 */
void stmflash_read(uint32_t raddr, uint16_t *pbuf, uint16_t length);    /* 从指定地址开始读出指定长度的数据 */
void stmflash_write(uint32_t waddr, uint16_t *pbuf, uint16_t length);   /* 在FLASH 指定位置, 写入指定长度的数据(自动擦除) */

#endif


















