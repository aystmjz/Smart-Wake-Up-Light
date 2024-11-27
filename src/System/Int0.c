#include "STC89C5xRC.h"

void Int0_Init(void)
{
	IT0=1;//启动
	IE0=0;//标志
	
	PX0=1;//优先级
	
	EX0=0;//中断
	EA=1;
}


void Int1_Init(void)
{
	IT1=1;//启动
	IE1=0;//标志
	
	PX1=1;//优先级
	
	EX1=0;//中断
	EA=1;
}
/*
void Int0_Routine(void) interrupt 0
{
	
}

void Int1_Routine(void) interrupt 2
{
	
}
*/