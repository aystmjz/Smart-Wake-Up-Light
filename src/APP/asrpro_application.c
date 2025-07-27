#include "asrpro_application.h"

void ASRPRO_ProcessCommand(uint8_t CmdNum)
{
    switch (CmdNum)
    {
    case 1:
        ASRPRO_printf("%c%c%c%c", 0xaa, 0x00, 0x01, (uint8_t)SHT.Temp);
        break;
    case 2:
        ASRPRO_printf("%c%c%c%c", 0xaa, 0x00, 0x02, (uint8_t)SHT.Hum);
        break;
    case 3:
        ASRPRO_printf("%c%c%c%c%c%c", 0xaa, 0x00, 0x03, (uint8_t)Time_Hour, (uint8_t)Time_Min, (uint8_t)Time_Sec);
        break;
    default:
        break;
    }
}