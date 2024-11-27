#include "I2C.h"

#define GY302_ADDRESS 0x46
#define GY302_CMD_H 0x10
#define GY302_CMD_H2 0x11
#define GY302_CMD_PowerOn 0x01
#define GY302_CMD_PowerDown 0x00


extern unsigned int lx=0;


void GY302_SendCMD(unsigned char CMD_Num)
{
	GY302_I2C_Start();
	GY302_I2C_SendByte(GY302_ADDRESS);
	GY302_I2C_ReceiveAck();
	switch(CMD_Num)
			{	
				case 1: GY302_I2C_SendByte(GY302_CMD_H);break;
				case 2: GY302_I2C_SendByte(GY302_CMD_H2);break;
				case 3: GY302_I2C_SendByte(GY302_CMD_PowerOn);break;
				case 4: GY302_I2C_SendByte(GY302_CMD_PowerDown);break;
			}
	GY302_I2C_ReceiveAck();
	GY302_I2C_Stop();
}


void GY302_Read_lx(void)
{
	unsigned char HighByte,LowByte;
	unsigned long Temp;
	GY302_I2C_Start();
	GY302_I2C_SendByte(GY302_ADDRESS|0x01);
	GY302_I2C_ReceiveAck();
	HighByte=GY302_I2C_ReceiveByte();
	GY302_I2C_SendAck(0);
	LowByte=GY302_I2C_ReceiveByte();
	GY302_I2C_SendAck(1);
	GY302_I2C_Stop();
	Temp=HighByte;
	Temp<<=8;
	Temp+=LowByte;
	Temp*=10;
	lx=Temp/12;
}	


void GY302_Init()
{
	GY302_SendCMD(3);
	GY302_SendCMD(1);
}

