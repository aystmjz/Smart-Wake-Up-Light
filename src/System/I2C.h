#ifndef __I2C_H__
#define __I2C_H__


void OLED_I2C_Start(void);
void OLED_I2C_Stop(void);
void OLED_I2C_SendByte(unsigned char Byte);
bit OLED_I2C_ReceiveAck(void);

void DS3231_I2C_Start(void);
void DS3231_I2C_Stop(void);
void DS3231_I2C_SendByte(unsigned char Byte);
unsigned char DS3231_I2C_ReceiveByte(void);
void DS3231_I2C_SendAck(bit AckBit);
bit DS3231_I2C_ReceiveAck(void);

void AT24C02_I2C_Start(void);
void AT24C02_I2C_Stop(void);
void AT24C02_I2C_SendByte(unsigned char Byte);
unsigned char AT24C02_I2C_ReceiveByte(void);
void AT24C02_I2C_SendAck(bit AckBit);
bit AT24C02_I2C_ReceiveAck(void);

#endif
