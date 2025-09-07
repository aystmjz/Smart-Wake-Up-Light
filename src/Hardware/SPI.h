#ifndef __SPI_H
#define __SPI_H

#include "sys.h"

// W25Q128 SPI片选引脚宏定义
#define W25Q128_SPI_SS_SET PBout(12)

void W25Q128_SPI_Init(void);
void W25Q128_SPI_Start(void);
void W25Q128_SPI_Stop(void);
uint8_t W25Q128_SPI_SwapByte(uint8_t ByteSend);

#endif
