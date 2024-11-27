#include "STC89C5xRC.h"

sbit Key_1=P0^7;
sbit Key_2=P0^6;
sbit Key_3=P0^5;
sbit Key_4=P0^2;

unsigned char Key_KeyNumber;

unsigned char Get_KeyNumber(void)
{
	unsigned char Temp=0;
	Temp=Key_KeyNumber;
	Key_KeyNumber=0;

	return Temp;
}

unsigned char Key_GetState()
{
	unsigned char KeyNumber=0;
	
	if(Key_1==0){KeyNumber=1;}
	if(Key_2==0){KeyNumber=2;}
	if(Key_3==0){KeyNumber=3;}
	if(Key_4==0){KeyNumber=4;}
	
	return KeyNumber;
}

void Key_Entry()
{
	static unsigned char NowState,LastState;
	static unsigned int HoldTimer;
	LastState=NowState;
	NowState=Key_GetState();
	if(LastState==0&&NowState==1){Key_KeyNumber=1;HoldTimer=0;}
	else if(LastState==1&&NowState==1){HoldTimer++;if(HoldTimer>20)Key_KeyNumber=1;}
		
	if(LastState==0&&NowState==2){Key_KeyNumber=2;HoldTimer=0;}
	else if(LastState==2&&NowState==2){HoldTimer++;if(HoldTimer>20)Key_KeyNumber=2;}
		
	if(LastState==0&&NowState==3){Key_KeyNumber=3;HoldTimer=0;}
	else if(LastState==3&&NowState==3){HoldTimer++;if(HoldTimer>20)Key_KeyNumber=3;}
		
	if(LastState==0&&NowState==4 ){Key_KeyNumber=4;HoldTimer=0;}
	else if(LastState==4&&NowState==4){HoldTimer++;if(HoldTimer>20)Key_KeyNumber=4;}

}