#ifndef __BATTERY_H
#define __BATTERY_H
#include "sys.h"

#define Battery_FULL 5174
#define Battery_CHARGE 5200
#define Battery_DC 5450
#define BATTERY_MULT 2

extern uint8_t Battery_Flag;

uint8_t Battery_calculate(uint16_t Battery);
void Battery_UpdateLevel(uint16_t battery_ADC);
uint8_t Battery_GetLevel();
uint8_t Battery_GetState();

#endif