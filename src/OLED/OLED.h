#ifndef __OLED_H
#define __OLED_H

#include "sys.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "Delay.h"
#include "LED.h"
#include "OLED_Data.h"

//-----------------OLED端口定义----------------

#define OLED_SCL_Clr() GPIO_ResetBits(GPIOA, GPIO_Pin_5) // SCL
#define OLED_SCL_Set() GPIO_SetBits(GPIOA, GPIO_Pin_5)

#define OLED_SDA_Clr() GPIO_ResetBits(GPIOA, GPIO_Pin_7) // SDA
#define OLED_SDA_Set() GPIO_SetBits(GPIOA, GPIO_Pin_7)

#define OLED_RES_Clr() GPIO_ResetBits(GPIOB, GPIO_Pin_3) // RES
#define OLED_RES_Set() GPIO_SetBits(GPIOB, GPIO_Pin_3)

#define OLED_DC_Clr() GPIO_ResetBits(GPIOA, GPIO_Pin_15) // DC
#define OLED_DC_Set() GPIO_SetBits(GPIOA, GPIO_Pin_15)

#define OLED_CS_Clr() GPIO_ResetBits(GPIOA, GPIO_Pin_4) // CS
#define OLED_CS_Set() GPIO_SetBits(GPIOA, GPIO_Pin_4)

#define OLED_BUSY() GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4)

#define OLED_W 296
#define OLED_H 128

#define LINE_END 256

typedef struct
{
    u8 *Image;
    u16 Width;
    u16 Height;
    u16 WidthMemory;
    u16 HeightMemory;
    u16 Color;
    u16 Rotate;
    u16 WidthByte;
    u16 HeightByte;
} PAINT;

extern PAINT Paint;
extern u8 Image_BW[OLED_W * OLED_H / 8];

#define OLED_6X8 8
#define OLED_6X12 12
#define OLED_8X16 16
#define OLED_12X24 24
#define OLED_52X104 104

#define ROTATE_0 0     // 屏幕正向显示
#define ROTATE_90 90   // 屏幕旋转90度显示
#define ROTATE_180 180 // 屏幕旋转180度显示
#define ROTATE_270 270 // 屏幕旋转270度显示

#define WHITE 0XFF // 显示白色
#define BLACK 0X00 // 显示黑色

#define All 1
#define Part 0

void OLED_WR_Bus(u8 dat);   // 模拟SPI时序
void OLED_WR_REG(u8 reg);   // 写入一个命令
void OLED_WR_DATA8(u8 dat); // 写入一个字节

void Epaper_READBUSY(void);
void EPD_WhiteScreen_White(void); // 清屏白色
void EPD_Dis_PartAll(u8 *Image);  // 用局刷的方式刷新全屏
void EPD_DeepSleep(void);         // 深度睡眠
void EPD_WeakUp(void);

void Paint_NewImage(u8 *image, u16 Width, u16 Height, u16 Rotate, u16 Color);            // 创建画布控制显示方向
void OLED_Clear(u16 Color);                                                              // 清屏函数
void OLED_DrawPoint(u16 Xpoint, u16 Ypoint, u16 Color);                                  // 画点
void OLED_DrawLine(u16 Xstart, u16 Ystart, u16 Xend, u16 Yend, u16 Color);               // 画线
void OLED_DrawRectangle(u16 Xstart, u16 Ystart, u16 Xend, u16 Yend, u16 Color, u8 mode); // 画矩形
void OLED_DrawCircle(u16 X_Center, u16 Y_Center, u16 Radius, u16 Color, u8 mode);        // 画圆
void OLED_DrawChart(u16 Xstart, u16 Ystart,u16 Width, u16 Height, u8* Data,u8 Num, u16 Color);

void OLED_ShowChar(u16 X, u16 Y, u8 Char, u8 Size, u16 Color);                       // 显示字符
void OLED_ShowNum(u16 X, u16 Y, u32 Num, u16 Len, u8 Size, u16 Color);               // 显示数字
void OLED_ShowChinese(u16 X, u16 Y, u8 *Hanzi, u8 Size, u16 Color);                  // 显示中文
void OLED_ShowString(u16 X, u16 Y, u8 *String, u8 Size, u16 Color);                  // 显示字符串
void OLED_ShowImage(u16 X, u16 Y, u16 Sizex, u16 Sizey, const u8 *Image, u16 Color); // 显示图片

void OLED_Printf(u16 X, u16 Y, u8 Size, u16 Color, const char *format, ...);
void OLED_Display(u8 *Image, u8 Mode); // 更新到屏幕(局刷/全刷)
void OLED_Init(void);

#endif