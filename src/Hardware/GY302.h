#ifndef __GY302_H__
#define __GY302_H__

extern unsigned int lx;
void GY302_SendCMD(unsigned char CMD_Num);
void GY302_Read_lx(void);
void GY302_Init();


#endif