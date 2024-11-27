#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

void BlueTooth_Init();
void BlueTooth_SendByte(unsigned char Byte);
void BlueTooth_SendString(unsigned char *P_String);

#endif