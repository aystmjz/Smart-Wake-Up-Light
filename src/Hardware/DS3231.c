#include "STC89C5xRC.h"
#include "I2C.h"

#define DS3231_ADDRESS 0xD0
#define DS3231_CONTROL 0x0E
#define DS3231_STATUS  0x0F
#define DS3231_Temp	   0x0B
#define DS3231_Temp2   0x0C

unsigned char code RTC_CLOCK_ADDR[7]={0x00,0x01,0x02,0x04,0x05,0x03,0x06};//秒分时日月周年
unsigned char code RTC_ALARM1_ADDR[4]={0x07,0x08,0x09,0x0A};//秒分时日月周年
unsigned char code RTC_ALARM2_ADDR[3]={0x0B,0x0C,0x0D};//秒分时日月周年
extern char TIME[7] = {0, 0, 0x16, 0x1C, 0x06, 0x01, 0x17};//秒分时日月周年
extern unsigned char Alarm_Set[8]={0,1,1,1,1,1,0,1};
extern char Alarm_Date[2]={8,0};
extern char Alarm_Date_Temp[2]={8,0};
extern unsigned char Alarm_Status[2]={0,0};
extern unsigned char Alarm_Enable=1;
extern char PWM_Mod=0;

void DS3231_WriteByte(unsigned char WordAddress,unsigned char Data)
{
	DS3231_I2C_Start();
	DS3231_I2C_SendByte(DS3231_ADDRESS);
	DS3231_I2C_ReceiveAck();
	DS3231_I2C_SendByte(WordAddress);
	DS3231_I2C_ReceiveAck();
	DS3231_I2C_SendByte(Data);
	DS3231_I2C_ReceiveAck();
	DS3231_I2C_Stop();
}

unsigned char DS3231_ReadByte(unsigned char WordAddress)
{
	unsigned char Data;
	DS3231_I2C_Start();
	DS3231_I2C_SendByte(DS3231_ADDRESS);
	DS3231_I2C_ReceiveAck();
	DS3231_I2C_SendByte(WordAddress);
	DS3231_I2C_ReceiveAck();
	DS3231_I2C_Stop();
	DS3231_I2C_Start();
	DS3231_I2C_SendByte(DS3231_ADDRESS|0x01);
	DS3231_I2C_ReceiveAck();
	Data=DS3231_I2C_ReceiveByte();
	DS3231_I2C_SendAck(1);
	DS3231_I2C_Stop();
	return Data;
}

void DS3231_ReadTime()
{
    unsigned char i,Temp;         
    for(i=0;i<7;i++)
    {
      if(i<2){Alarm_Status[i]=(DS3231_ReadByte(DS3231_STATUS)&(0x01<<i));}
			Temp = DS3231_ReadByte(RTC_CLOCK_ADDR[i]);
      TIME[i]=(Temp/16)*10+(Temp%16);
    }  
}

void DS3231_WriteTime()
{
    unsigned char i;
    for(i=0;i<7;i++)
    {
        DS3231_WriteByte(RTC_CLOCK_ADDR[i],(TIME[i]/10)*16+TIME[i]%10);
    }
}

void DS3231_WriteAlarm()
{
	unsigned char i,Temp=0;
	DS3231_WriteByte(RTC_ALARM1_ADDR[0],0);
	DS3231_WriteByte(RTC_ALARM1_ADDR[3],0x80);
	for(i=0;i<8;i++)
	{
		if(i==1||i==2)DS3231_WriteByte(RTC_ALARM1_ADDR[i],((Alarm_Date_Temp[2-i]/10)<<4)+(Alarm_Date_Temp[2-i]%10));
		Temp+=Alarm_Set[7-i];
		if(i!=7)Temp<<=1;
	}
	DS3231_WriteByte(DS3231_CONTROL,0x04+Alarm_Enable);
	DS3231_WriteByte(DS3231_Temp2,PWM_Mod);
	DS3231_WriteByte(DS3231_Temp,Temp);
}

void DS3231_Init()
{
	unsigned char i,Temp;
	DS3231_WriteByte(DS3231_STATUS,0x00);
	DS3231_ReadTime();
	
	PWM_Mod=DS3231_ReadByte(DS3231_Temp2);
	
	Temp=DS3231_ReadByte(DS3231_CONTROL);
	Alarm_Enable=!(!(Temp&0x01));

	Temp=DS3231_ReadByte(DS3231_Temp);
	for(i=0;i<8;i++)
	{	
		Alarm_Set[i]=!(!(Temp&(0x01<<i)));
	}

	for(i=1;i<=2;i++)
	{
		Temp=DS3231_ReadByte(RTC_ALARM1_ADDR[i]);
		Alarm_Date_Temp[2-i]=(Temp>>4)*10+(Temp&0x0F);
	}
	
	switch(PWM_Mod)
	{	
		case 0: Alarm_Date[0]=(Alarm_Date_Temp[0]*60+Alarm_Date_Temp[1]+5)/60;
				Alarm_Date[1]=(Alarm_Date_Temp[0]*60+Alarm_Date_Temp[1]+5)%60;break;
		case 1: Alarm_Date[0]=(Alarm_Date_Temp[0]*60+Alarm_Date_Temp[1]+10)/60;
				Alarm_Date[1]=(Alarm_Date_Temp[0]*60+Alarm_Date_Temp[1]+10)%60;break;
		case 2: Alarm_Date[0]=(Alarm_Date_Temp[0]*60+Alarm_Date_Temp[1]+15)/60;
				Alarm_Date[1]=(Alarm_Date_Temp[0]*60+Alarm_Date_Temp[1]+15)%60;break;
		case 3: Alarm_Date[0]=(Alarm_Date_Temp[0]*60+Alarm_Date_Temp[1]+20)/60;
				Alarm_Date[1]=(Alarm_Date_Temp[0]*60+Alarm_Date_Temp[1]+20)%60;break;
		case 4: Alarm_Date[0]=(Alarm_Date_Temp[0]*60+Alarm_Date_Temp[1]+30)/60;
				Alarm_Date[1]=(Alarm_Date_Temp[0]*60+Alarm_Date_Temp[1]+30)%60;break;
		case 5: Alarm_Date[0]=(Alarm_Date_Temp[0]*60+Alarm_Date_Temp[1]+40)/60;
				Alarm_Date[1]=(Alarm_Date_Temp[0]*60+Alarm_Date_Temp[1]+40)%60;break;
		case 6: Alarm_Date[0]=(Alarm_Date_Temp[0]*60+Alarm_Date_Temp[1]+60)/60;
				Alarm_Date[1]=(Alarm_Date_Temp[0]*60+Alarm_Date_Temp[1]+60)%60;break;
	}
	
	
}
