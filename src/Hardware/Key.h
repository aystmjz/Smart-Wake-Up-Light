#ifndef __KEY_H
#define __KEY_H

#include "sys.h"
#include "Delay.h"
#include "Buzzer.h"

#define KEY_NONE    0  // 无按键事件
#define KEY_CLICK   1  // 单击事件
#define KEY_LONG    2  // 长按事件
#define KEY_DOUBLE  3  // 双击事件

void Timer_Init(void);
void Key_Init(void);
uint8_t Key_Read(void);
uint8_t Key_GetNumber(void);
uint8_t Key_Clear(void);
uint8_t Key_GetState(void);
void Key_Entry(void);

#endif
