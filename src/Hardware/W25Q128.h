#ifndef __W25Q128_H
#define __W25Q128_H

#include "SPI.h"
#include "sys.h"

// Flash 总容量：128Mbit = 16MB
#define W25Q128_FLASH_SIZE_BYTES                 (16UL * 1024 * 1024) // 16MB = 16777216 Bytes

// 页（Page）结构：每页 256 字节
#define W25Q128_PAGE_SIZE                        (256U) // 256 Bytes/Page
#define W25Q128_TOTAL_PAGE_COUNT                 (W25Q128_FLASH_SIZE_BYTES / W25Q128_PAGE_SIZE) // 65536 pages

// 扇区（Sector）结构：每扇区 4KB
#define W25Q128_SECTOR_SIZE                      (4U * 1024U) // 4096 Bytes/Sector
#define W25Q128_TOTAL_SECTOR_COUNT               (W25Q128_FLASH_SIZE_BYTES / W25Q128_SECTOR_SIZE) // 4096 sectors

// 块（Block）结构：每块 64KB = 16 个扇区
#define W25Q128_BLOCK_SIZE                       (64U * 1024U) // 65536 Bytes/Block
#define W25Q128_TOTAL_BLOCK_COUNT                (W25Q128_FLASH_SIZE_BYTES / W25Q128_BLOCK_SIZE) // 256 blocks

// 最大索引
#define W25Q128_MAX_PAGE_INDEX                   (W25Q128_TOTAL_PAGE_COUNT - 1U)
#define W25Q128_MAX_SECTOR_INDEX                 (W25Q128_TOTAL_SECTOR_COUNT - 1U)
#define W25Q128_MAX_BLOCK_INDEX                  (W25Q128_TOTAL_BLOCK_COUNT - 1U)

// W25Q128指令集定义
#define W25Q128_WRITE_ENABLE                     0x06 // 写使能指令
#define W25Q128_WRITE_DISABLE                    0x04 // 写禁止指令
#define W25Q128_READ_STATUS_REGISTER_1           0x05 // 读状态寄存器1指令
#define W25Q128_READ_STATUS_REGISTER_2           0x35 // 读状态寄存器2指令
#define W25Q128_WRITE_STATUS_REGISTER            0x01 // 写状态寄存器指令
#define W25Q128_PAGE_PROGRAM                     0x02 // 页编程指令
#define W25Q128_QUAD_PAGE_PROGRAM                0x32 // 四线页编程指令
#define W25Q128_BLOCK_ERASE_64KB                 0xD8 // 64KB块擦除指令
#define W25Q128_BLOCK_ERASE_32KB                 0x52 // 32KB块擦除指令
#define W25Q128_SECTOR_ERASE_4KB                 0x20 // 4KB扇区擦除指令
#define W25Q128_CHIP_ERASE                       0xC7 // 芯片全擦除指令
#define W25Q128_ERASE_SUSPEND                    0x75 // 擦除暂停指令
#define W25Q128_ERASE_RESUME                     0x7A // 擦除恢复指令
#define W25Q128_POWER_DOWN                       0xB9 // 掉电模式指令
#define W25Q128_HIGH_PERFORMANCE_MODE            0xA3 // 高性能模式指令
#define W25Q128_CONTINUOUS_READ_MODE_RESET       0xFF // 连续读模式复位指令
#define W25Q128_RELEASE_POWER_DOWN_HPM_DEVICE_ID 0xAB // 释放掉电/高性能模式/设备ID指令
#define W25Q128_MANUFACTURER_DEVICE_ID           0x90 // 读取制造商/设备ID指令
#define W25Q128_READ_UNIQUE_ID                   0x4B // 读取唯一ID指令
#define W25Q128_JEDEC_ID                         0x9F // JEDEC ID指令
#define W25Q128_READ_DATA                        0x03 // 读数据指令
#define W25Q128_FAST_READ                        0x0B // 快速读指令
#define W25Q128_FAST_READ_DUAL_OUTPUT            0x3B // 双线快速读输出指令
#define W25Q128_FAST_READ_DUAL_IO                0xBB // 双线快速读I/O指令
#define W25Q128_FAST_READ_QUAD_OUTPUT            0x6B // 四线快速读输出指令
#define W25Q128_FAST_READ_QUAD_IO                0xEB // 四线快速读I/O指令
#define W25Q128_OCTAL_WORD_READ_QUAD_IO          0xE3 // 八字快速读四I/O指令
#define W25Q128_DUMMY_BYTE                       0xFF // 虚拟字节

// 设置参数存储地址
#define SETTING_ADDRESS                          0xFFF000

// 设置参数结构体
typedef struct
{
    uint8_t PwmMod;        // PWM模式设置
    uint8_t MuzicEnable;   // 音乐使能设置
    uint8_t BuzzerEnable;  // 蜂鸣器使能设置
    uint8_t VoiceEnable;   // 语音使能设置
    uint8_t WeekEnable[7]; // 星期使能设置数组(周一到周日)
    char DeviceName[32];   // 设备名称字符串
} SettingTypeDef;

void W25Q128_Init(void);
void W25Q128_ReadID(uint8_t *MID, uint16_t *DID);
void W25Q128_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count);
void W25Q128_SectorErase(uint32_t Address);
void W25Q128_BlockErase(uint32_t Address);
void W25Q128_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count);
void W25Q128_ReadSetting(SettingTypeDef *Set);
void W25Q128_WriteSetting(SettingTypeDef *Set);

#endif
