#include "STC89C5xRC.h"
#include "UART.h"

void BlueTooth_Init()
{
	UART_Init();
	ES=1;//ÔÊÐí´®¿ÚÖÐ¶Ï	
}

void BlueTooth_SendByte(unsigned char Byte)
{
	UART_SendByte(Byte);
}


void BlueTooth_SendString(unsigned char *P_String)
{
	while(*P_String!='\0')
	{
		UART_SendByte(*P_String);
		P_String++;
	}
}
	