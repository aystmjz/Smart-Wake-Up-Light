#ifndef __I2C_H__
#define __I2C_H__

#include "Delay.h"
#include "sys.h"

// DS3231 I2C引脚定义
#define DS3231_I2C_SCL_SET  PAout(12) // DS3231 I2C时钟线输出控制，PA12
#define DS3231_I2C_SDA_SET  PAout(11) // DS3231 I2C数据线输出控制，PA11
#define DS3231_I2C_SDA_READ PAin(11)  // DS3231 I2C数据线输入读取，PA11

// SHT30 I2C引脚定义
#define SHT30_I2C_SCL_SET   PCout(14) // SHT30 I2C时钟线输出控制，PC14
#define SHT30_I2C_SDA_SET   PCout(15) // SHT30 I2C数据线输出控制，PC15
#define SHT30_I2C_SDA_READ  PCin(15)  // SHT30 I2C数据线输入读取，PC15

void DS3231_I2C_Init(void);
void DS3231_I2C_Start(void);
void DS3231_I2C_Stop(void);
void DS3231_I2C_SendByte(uint8_t Byte);
unsigned char DS3231_I2C_ReceiveByte(void);
void DS3231_I2C_SendAck(uint8_t AckBit);
uint8_t DS3231_I2C_ReceiveAck(void);

void SHT30_I2C_Init(void);
void SHT30_I2C_Start(void);
void SHT30_I2C_Stop(void);
void SHT30_I2C_SendByte(uint8_t Byte);
uint8_t SHT30_I2C_ReceiveByte(void);
void SHT30_I2C_SendAck(uint8_t AckBit);
uint8_t SHT30_I2C_ReceiveAck(void);

#endif
