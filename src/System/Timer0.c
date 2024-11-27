#include "STC89C5xRC.h"


void Timer0_Init(void)		//1毫秒@11.0592MHz
{

	TMOD &= 0xF0;		//设置定时器模式
	TMOD |= 0x01;		//设置定时器模式
	TL0 = 0x66;		//设置定时初始值
	TH0 = 0xFC;		//设置定时初始值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
	EA=1;
	
	PT0=0;
}

void Timer2_Init(void)		//50微秒@11.0592MHz
{
	T2MOD = 0;		//初始化模式寄存器
	T2CON = 0;		//初始化控制寄存器
	TL2 = 0xD2;		//设置定时初始值
	TH2 = 0xFF;		//设置定时初始值
	RCAP2L = 0xD2;		//设置定时重载值
	RCAP2H = 0xFF;		//设置定时重载值
	TR2 = 1;		//定时器2开始计时
		ET2=0;
	EA=1;
	
	//PT2H=1;//优先级3
	PT2=1;
}


void Timer_Init(void)
{
	Timer0_Init(); 
	Timer2_Init();	
}



/*
void Timer0_Routine() interrupt 1
{
	TL0 = 0x66;
	TH0 = 0xFC;
	
}
*/
