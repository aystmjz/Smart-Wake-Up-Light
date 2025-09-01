#ifndef __SYSTEM_INIT_H
#define __SYSTEM_INIT_H

#include "sys.h"
#include "Key.h"
#include "Encoder.h"
#include "OLED.h"
#include "DS3231.h"
#include "Buzzer.h"
#include "ASRPRO.h"
#include "LED.h"
#include "SHT30.h"
#include "EXTI.h"
#include "W25Q128.h"
#include "PWR.h"
#include "BT24.h"
#include "ADC.h"
#include "Battery.h"
#include "globals.h"
#include "pwm_application.h"

#define FIRMWARE_VERSION "2.4.4"
#define FIRMWARE_BUILD_DATE __DATE__
#define FIRMWARE_BUILD_TIME __TIME__

void System_Init();

#endif