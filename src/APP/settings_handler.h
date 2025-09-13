#ifndef __SETTINGS_HANDLER_H
#define __SETTINGS_HANDLER_H

#include "DS3231.h"
#include "Key.h"
#include "OLED.h"
#include "pwm_application.h"
#include "sys.h"
#include "system_init.h"

void WriteAlarm(AlarmTypeDef *Alarm, uint8_t *Mod);
void KeyNumber_Set();

#endif