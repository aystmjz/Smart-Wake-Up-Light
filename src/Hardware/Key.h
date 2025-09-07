#ifndef __KEY_H
#define __KEY_H

#include "Buzzer.h"
#include "Delay.h"
#include "sys.h"

// 按键事件
#define KEY_NONE   0 // 无按键事件
#define KEY_CLICK  1 // 单击事件
#define KEY_LONG   2 // 长按事件
#define KEY_DOUBLE 3 // 双击事件

void Timer_Init(void);
void Key_Init(void);
uint8_t Key_Read(void);
uint8_t Key_GetEvent(void);
uint8_t Key_Clear(void);
void Key_Entry(void);

#endif
