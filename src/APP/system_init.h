#ifndef __SYSTEM_INIT_H
#define __SYSTEM_INIT_H

#include "ADC.h"
#include "ASRPRO.h"
#include "BT24.h"
#include "Battery.h"
#include "Buzzer.h"
#include "DS3231.h"
#include "EXTI.h"
#include "Encoder.h"
#include "Key.h"
#include "LED.h"
#include "OLED.h"
#include "PWR.h"
#include "SHT30.h"
#include "W25Q128.h"
#include "globals.h"
#include "pwm_application.h"
#include "sys.h"

#define FIRMWARE_VERSION    "2.4.4"  // 固件版本
#define FIRMWARE_BUILD_DATE __DATE__ // 固件构建日期，使用编译器内置宏
#define FIRMWARE_BUILD_TIME __TIME__ // 固件构建时间，使用编译器内置宏

void System_Init();

#endif