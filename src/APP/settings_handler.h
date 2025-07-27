#ifndef __SETTINGS_HANDLER_H
#define __SETTINGS_HANDLER_H

#include "sys.h"
#include "system_init.h"
#include "pwm_application.h"
#include "Key.h"
#include "OLED.h"
#include "DS3231.h"

void WriteAlarm(AlarmTypeDef *Alarm, uint8_t *Mod);
void KeyNumber_Set();

#endif