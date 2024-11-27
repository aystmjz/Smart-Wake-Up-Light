#include <REGX52.H>
#include "I2C.h"

#define AT24C02_ADDRESS 0xA0

unsigned char Light_Date[181];

void AT24C02_WriteByte(unsigned char WordAddress,Data)
{
	AT24C02_I2C_Start();
	AT24C02_I2C_SendByte(AT24C02_ADDRESS);
	AT24C02_I2C_ReceiveAck();
	AT24C02_I2C_SendByte(WordAddress);
	AT24C02_I2C_ReceiveAck();
	AT24C02_I2C_SendByte(Data);
	AT24C02_I2C_ReceiveAck();
	AT24C02_I2C_Stop();
}

unsigned char AT24C02_ReadByte(unsigned char WordAddress)
{
	unsigned char Data;
	AT24C02_I2C_Start();
	AT24C02_I2C_SendByte(AT24C02_ADDRESS);
	AT24C02_I2C_ReceiveAck();
	AT24C02_I2C_SendByte(WordAddress);
	AT24C02_I2C_ReceiveAck();
	AT24C02_I2C_Start();
	AT24C02_I2C_SendByte(AT24C02_ADDRESS|0x01);
	AT24C02_I2C_ReceiveAck();
	Data=AT24C02_I2C_ReceiveByte();
	AT24C02_I2C_SendAck(1);
	AT24C02_I2C_Stop();
	return Data;
}

void AT24C02_Write_Light(void)
{
	unsigned char i,Temp=0x00;
	for(i=0;i<=180;i++)
	{
		AT24C02_WriteByte(Temp,Light_Date[i]);
		Temp++;
	}
}

void AT24C02_Init(void)
{
	unsigned char i,Temp=0x00;
	for(i=0;i<=180;i++)
	{
		Light_Date[i]=AT24C02_ReadByte(Temp);
		Temp++;
	}
}