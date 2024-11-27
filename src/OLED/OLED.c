#include "OLEDFONT.h"  	 
#include "I2C.h"
#include "AT24C02.h"	
	
#define OLED_ADDRESS 	0x78
#define Mode_Data	0x40
#define Mode_CMD	0x00
	   
extern unsigned char Light_Date[181];

void OLED_WriteCMD(unsigned char Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(OLED_ADDRESS);
	OLED_I2C_ReceiveAck();
	OLED_I2C_SendByte(Mode_CMD);
	OLED_I2C_ReceiveAck();
	OLED_I2C_SendByte(Data);
	OLED_I2C_ReceiveAck();
	OLED_I2C_Stop();
}

void OLED_WriteData(unsigned char Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(OLED_ADDRESS);
	OLED_I2C_ReceiveAck();
	OLED_I2C_SendByte(Mode_Data);
	OLED_I2C_ReceiveAck();
	OLED_I2C_SendByte(Data);
	OLED_I2C_ReceiveAck();
	OLED_I2C_Stop();
}


void OLED_SetPointer(unsigned char x, unsigned char y) 
{ 
	OLED_WriteCMD(0xb0+y);
	OLED_WriteCMD(((x&0xf0)>>4)|0x10);
	OLED_WriteCMD((x&0x0f));
}   	  
   
   			
void OLED_Clear(void)  
{  
	unsigned char i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WriteCMD (0xb0+i);
		OLED_WriteCMD (0x00); 
		OLED_WriteCMD (0x10);    
		for(n=0;n<128;n++)OLED_WriteData(0); 
	} 
}


void OLED_ShowChar(unsigned char x,unsigned char y,unsigned char Char,unsigned char Char_Size)
{      	
	unsigned char i,j,CharNum;
	CharNum=Char-' ';
	if(Char_Size==16)
	{
		for(i=0;i<2;i++)
		{
			OLED_SetPointer(x,y+i);
			for(j=0;j<8;j++)
			{
				 OLED_WriteData(ascii_8_16[CharNum][j+i*8]);//8x16
			}
		}
	}
	else if(Char_Size==8)
	{
		OLED_SetPointer(x,y);
		for(i=0;i<6;i++)
			{
				 OLED_WriteData(ascii_6_8[CharNum][i]);//6X8
			}
	}
}

unsigned int Pow(unsigned int m,unsigned int n)
{
	unsigned int result=1;	 
	while(n--)result*=m;    
	return result;
}				  

void OLED_ShowNum(unsigned char x,unsigned char y,unsigned int Num,unsigned char Num_Lim,unsigned char Char_Size)
{         	
	unsigned char i,Temp,Enshow=0;
	for(i=0;i<Num_Lim;i++)
	{
		Temp=(Num/Pow(10,Num_Lim-i-1))%10;
		if(Enshow==0&&i<(Num_Lim-1))
		{
			if(Temp==0)
			{
				if(Char_Size==16)OLED_ShowChar(x+8*i,y,' ',Char_Size);
				else if(Char_Size==8)OLED_ShowChar(x+6*i,y,' ',Char_Size);
				continue;
			}else Enshow=1;
		}
	 	if(Char_Size==16)OLED_ShowChar(x+8*i,y,Temp+'0',Char_Size);
		else if(Char_Size==8)OLED_ShowChar(x+6*i,y,Temp+'0',Char_Size);
	}
}

void OLED_ShowString(unsigned char x,unsigned char y,unsigned char *Char,unsigned char Char_Size)
{
	unsigned char i=0;
	while (Char[i]!='\0')
	{		
		OLED_ShowChar(x,y,Char[i++],Char_Size);
		if(Char_Size==16)x+=8;
		else if(Char_Size==8)x+=6;
	}
}

void OLED_ShowSymbol(unsigned char x,unsigned char y,unsigned char Symbol_Num,unsigned char Symbol_Size)
{
	unsigned char i,j;
	if(Symbol_Size==16)
	{
		for(i=0;i<2;i++)
		{
			OLED_SetPointer(x,y+i);
			for(j=0;j<16;j++)
			{
				 OLED_WriteData(Symbol_16_16[Symbol_Num][j+i*16]);//16x16
			}
		}
	}
	else if(Symbol_Size==8)
	{
		OLED_SetPointer(x,y);
		for(i=0;i<6;i++)
			{
				 OLED_WriteData(Symbol_16_16[Symbol_Num][i]);//6X8
			}
	}
			
}

void OLED_ShowSentence(unsigned char n,unsigned char Sentence_Num)
{
	unsigned char i,j,l,x,y;
	x=0;y=n*2;
	for(l=1;l<=Sentence_Date[Sentence_Num][0];l++)
	{
			for(i=0;i<2;i++)
			{
				OLED_SetPointer(x,y+i);
				for(j=0;j<16;j++)
				{
					 OLED_WriteData(Sentence_16_16[Sentence_Date[Sentence_Num][l]][j+i*16]);//16x16
				}
			}
			x+=16;
	}
	
}

void OLED_DrawLight(void)
{ 	
  unsigned char i,j,a,b;
	for(i=0;i<8;i++)
	{
		OLED_SetPointer(0,0+i);
		for(j=0;j<128;j++)
		{
			a=(Light_Date[j+1]/2)/8;
			b=(Light_Date[j+1]/2)%8;
			if(i==(7-a))
			{
					switch(b)
				{	
					case 0: OLED_WriteData(0x80);break;
					case 1: OLED_WriteData(0x40);break;
					case 2: OLED_WriteData(0x20);break;
					case 3: OLED_WriteData(0x10);break;
					case 4: OLED_WriteData(0x08);break;
					case 5: OLED_WriteData(0x04);break;
					case 6: OLED_WriteData(0x02);break;
					case 7: OLED_WriteData(0x01);break;
				};
			}
			else
			OLED_WriteData(0x00);
		}
	}
} 

void OLED_DrawBMP(unsigned char BMP_Num)
{ 	
  	unsigned char i,j;
	for(i=0;i<8;i++)
	{
		OLED_SetPointer(0,0+i);
		for(j=0;j<128;j++)
		{
			if(BMP_Num==1) 
			OLED_WriteData(BMP_Name[i][j]);
			else if(BMP_Num==2)
			OLED_WriteData(BMP_GetUP[i][j]);
		}
	}
} 


		    
void OLED_Init(void)
{

	OLED_WriteCMD(0xAE);
	OLED_WriteCMD(0x00);
	OLED_WriteCMD(0x10);
	OLED_WriteCMD(0x40);
	OLED_WriteCMD(0x81);
	OLED_WriteCMD(0xCF);
	OLED_WriteCMD(0xA1);
	OLED_WriteCMD(0xC8);
	OLED_WriteCMD(0xA6);
	OLED_WriteCMD(0xA8);
	OLED_WriteCMD(0x3f);
	OLED_WriteCMD(0xD3);
	OLED_WriteCMD(0x00);
	OLED_WriteCMD(0xd5);
	OLED_WriteCMD(0x80);
	OLED_WriteCMD(0xD9);
	OLED_WriteCMD(0xF1);
	OLED_WriteCMD(0xDA);
	OLED_WriteCMD(0x12);
	OLED_WriteCMD(0xDB);
	OLED_WriteCMD(0x40);
	OLED_WriteCMD(0x20); 
	OLED_WriteCMD(0x02); 
	OLED_WriteCMD(0x8D);
	OLED_WriteCMD(0x14);
	OLED_WriteCMD(0xA4);
	OLED_WriteCMD(0xA6);
	OLED_WriteCMD(0xC8);
	OLED_WriteCMD(0xA1);
	OLED_WriteCMD(0xA6);
	OLED_WriteCMD(0xAF); 
	OLED_DrawBMP(1);
	OLED_Clear();
}
