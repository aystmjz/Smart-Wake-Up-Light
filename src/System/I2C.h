#ifndef __I2C_H__
#define __I2C_H__

#include "sys.h"
#include "Delay.h"

#define DS3231_I2C_SCL_SET PAout(12)
#define DS3231_I2C_SDA_SET PAout(11)
#define DS3231_I2C_SDA_READ PAin(11)

#define SHT30_I2C_SCL_SET PCout(14)
#define SHT30_I2C_SDA_SET PCout(15)
#define SHT30_I2C_SDA_READ PCin(15)

void DS3231_I2C_Init(void);
void DS3231_I2C_Start(void);
void DS3231_I2C_Stop(void);
void DS3231_I2C_SendByte(uint8_t Byte);
unsigned char DS3231_I2C_ReceiveByte(void);
void DS3231_I2C_SendAck(uint8_t AckBit);
uint8_t DS3231_I2C_ReceiveAck(void);

void SHT30_I2C_Init(void);
void SHT30_I2C_delay(void);
void SHT30_I2C_Start(void);
void SHT30_I2C_Stop(void);
void SHT30_I2C_SendByte(uint8_t Byte);
uint8_t SHT30_I2C_ReceiveByte(void);
void SHT30_I2C_SendAck(uint8_t AckBit);
uint8_t SHT30_I2C_ReceiveAck(void);

#endif
