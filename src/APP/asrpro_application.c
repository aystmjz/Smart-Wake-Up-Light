#include "asrpro_application.h"

/**
 * @brief 处理ASRPRO语音识别模块的命令
 * @param CmdNum 命令编号，决定执行哪种操作
 *          1 - 返回温度数据
 *          2 - 返回湿度数据
 *          3 - 返回当前时间
 *          其他 - 无操作
 * @details 根据接收到的命令编号执行相应的操作，通过串口返回数据给ASRPRO模块
 */
void ASRPRO_ProcessCommand(uint8_t CmdNum)
{
    switch (CmdNum)
    {
    case 1:
        // 发送温度数据给ASRPRO模块
        // 数据格式：0xaa(帧头) + 0x00(保留) + 0x01(命令号) + 温度值
        ASRPRO_printf("%c%c%c%c", 0xaa, 0x00, 0x01, (uint8_t)SHT.Temp);
        break;
    case 2:
        // 发送湿度数据给ASRPRO模块
        // 数据格式：0xaa(帧头) + 0x00(保留) + 0x02(命令号) + 湿度值
        ASRPRO_printf("%c%c%c%c", 0xaa, 0x00, 0x02, (uint8_t)SHT.Hum);
        break;
    case 3:
        // 发送时间数据给ASRPRO模块
        // 数据格式：0xaa(帧头) + 0x00(保留) + 0x03(命令号) + 小时 + 分钟 + 秒
        ASRPRO_printf("%c%c%c%c%c%c", 0xaa, 0x00, 0x03, (uint8_t)Time_Hour, (uint8_t)Time_Min,
                      (uint8_t)Time_Sec);
        break;
    default:
        break;
    }
}