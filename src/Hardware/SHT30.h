#ifndef __SHT30_H
#define __SHT30_H

#include "I2C.h"
#include "sys.h"

// SHT30传感器数据结构体
typedef struct
{
    float Temp;          // 温度值(摄氏度)
    float Hum;           // 湿度值(相对湿度%)
    uint8_t raw_data[6]; // 原始数据缓冲区(温度2字节+CRC+湿度2字节+CRC)
} SHT30TypeDef;

// SHT30设备地址和命令定义
#define SHT30_ADDRESS               0x44 << 1 // SHT30 I2C地址(写)
#define SHT30_MEAS_CLOCKNOSTR_H_CMD 0x2400    // 测量命令: 无时钟拉伸，高重复性
#define SHT30_MEAS_CLOCKNOSTR_M_CMD 0x240B    // 测量命令: 无时钟拉伸，中等重复性
#define SHT30_MEAS_CLOCKNOSTR_L_CMD 0x2416    // 测量命令: 无时钟拉伸，低重复性
#define SHT30_SOFT_RESET_CMD        0X30A2    // 软件复位命令

void SHT30_Init(void);
void SHT30_SoftReset(void);
void SHT30_HardReset(void);
uint8_t SHT30_GetData(SHT30TypeDef *SHT);

#endif
