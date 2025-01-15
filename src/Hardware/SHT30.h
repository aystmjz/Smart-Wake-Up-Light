#ifndef __SHT30_H
#define __SHT30_H

#include "stm32f10x.h"
#include "sys.h"
#include "I2C.h"

typedef struct
{
    float Temp;
    float Hum;
    uint8_t raw_data[6];
} SHT30_DataTypeDef;

extern SHT30_DataTypeDef SHT;

#define SHT30_ADDRESS 0x44 << 1
#define SHT30_MEAS_CLOCKNOSTR_H_CMD 0x2400 // measurement: clock no stretching, high repeatability
#define SHT30_MEAS_CLOCKNOSTR_M_CMD 0x240B // measurement: clock no stretching, medium repeatability
#define SHT30_MEAS_CLOCKNOSTR_L_CMD 0x2416 // measurement: clock no stretching, low repeatability
#define SHT30_SOFT_RESET_CMD 0X30A2        // software reset

void SHT30_WriteCommand(uint16_t Command);
void SHT30_MultiReadByte(uint8_t *Data_p, uint8_t Length);
void SHT30_Init(void);
void SHT30_Reset(void);
uint8_t SHT30_GetData(void);

#endif
