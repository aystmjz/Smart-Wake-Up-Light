#include "BT24.h"

static char AT_cmd[ATCMD_BUFF_LEN];

uint8_t BT24_AT_Init(char *DeviceName);
void BT24_Init(char *DeviceName)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

#if DEBUG_MODE == DEBUG_MODE_STM32 || DEBUG_MODE == DEBUG_MODE_ASRPRO
    GPIO_SetBits(GPIOB, GPIO_Pin_10);
#else
    GPIO_ResetBits(GPIOB, GPIO_Pin_10);
#endif
    GPIO_SetBits(GPIOA, GPIO_Pin_6);

    BT24_AT_Init(DeviceName);
}

// 发送高电平脉冲进行复位
void BT24_Reset(void)
{
#if DEBUG_MODE == DEBUG_MODE_STM32 || DEBUG_MODE == DEBUG_MODE_ASRPRO
    GPIO_SetBits(GPIOB, GPIO_Pin_10);
#else
    GPIO_SetBits(GPIOB, GPIO_Pin_10);
    Delay_ms(300);
    GPIO_ResetBits(GPIOB, GPIO_Pin_10);
#endif
}

// 发送低电平脉冲至KEY引脚(断开连接)
void BT24_Disconnect(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_6);
    Delay_ms(300);
    GPIO_SetBits(GPIOA, GPIO_Pin_6);
}

uint8_t BT24_GetStatus(void)
{
    return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
}

void ATcmd_Main_printf(char *SendBuf)
{
    BT24_printf(SendBuf);
}

void ATcmd_Debug_printf(char *SendBuf)
{
    LOG_ERROR("[BT24] %s", SendBuf);
}

void ATcmd_Send(char *ATcmd)
{
    ATcmd_Main_printf(ATcmd);
    Delay_ms(100);
}

/// @brief 从接收缓存中扫描指定应答
/// @param Ack 指定应答
/// @return 存在指定应答返回1
uint8_t ATcmd_Scan(char *Ack)
{
    char *strx;
    strx = strstr((const char *)ATcmd_RxBuffer, (const char *)Ack);
    if (strx)
        return 1;
    else
        return 0;
}

/// @brief 从接收缓存中提取指定数据
/// @param Data 写入位置
/// @param Flag 数据标志
/// @param Len  数据长度
/// @return 成功提取返回1
uint8_t Data_Scan(char *Data, char *Flag, uint8_t Len)
{
    char *pstr, *pData = Data;
    uint8_t FlagLen = strlen(Flag);
    pstr = strstr((const char *)ATcmd_RxBuffer, (const char *)Flag);
    if (!pstr)
        return 0;
    pstr += FlagLen;
    for (int i = 0; i < Len; i++)
    {
        *pData = *pstr;
        pstr++;
        pData++;
    }
    *pData = '\0';
    return 1;
}

/// @brief 从接收缓存中提取指定数据(整数)
/// @param Flag 数据标志
/// @param Len  数据长度
/// @return 提取的整数
uint16_t Data_ScanInt(char *Flag, uint8_t Len)
{
    char Data_Temp[10];
    if (!Data_Scan(Data_Temp, Flag, Len))
        return 0;
    return atoi(Data_Temp);
}

/// @brief 发送AT_cmd中指令并等待目标答复
/// @param Ack 目标答复
/// @param Error 错误提示
/// @param Wait_Time 等待答复时间
/// @param Try_Time 重试次数
/// @return 成功：1	失败：0
uint8_t ATcmd_Wait(char *Ack, char *Error, uint16_t Wait_Time, uint8_t Try_Time)
{
    ATcmd_Clear_Buffer();
    Delay_ms(100);
    if (!Try_Time)
        Try_Time = 0xff;
    for (uint8_t i = 1; i <= Try_Time; i++)
    {
        ATcmd_Send(AT_cmd);
        // ATcmd_Debug_printf("Wait\r\n");
        uint16_t timeout = 1;
        while (!ATcmd_Scan(Ack))
        {
            Delay_ms(1);
            timeout++;
            if (timeout > Wait_Time)
            {
                if (Error != NULL)
                    ATcmd_Debug_printf(Error);
                timeout = 0;
                break;
            }
        }
        if (timeout)
            return 1;
    }
    return 0;
}

/// @brief 生成AT指令到AT_cmd
/// @param num 指令参数数量
/// @param cmd 指令名称
/// @param
void ATcmd_Make(int num, const char *cmd, ...)
{
    va_list arg_list;
    char cmd_temp[ATCMD_BUFF_LEN] = {0};
    char *pstr_temp;
    va_start(arg_list, cmd);
    strcat(cmd_temp, "AT+");
    strcat(cmd_temp, cmd);
    if (num)
        strcat(cmd_temp, "=");
    for (int i = 0; i < num; i++)
    {
        pstr_temp = va_arg(arg_list, char *);
        if (*pstr_temp < '0' || *pstr_temp > '9')
        {
            strcat(cmd_temp, "\"");
            strcat(cmd_temp, pstr_temp);
            strcat(cmd_temp, "\"");
        }
        else
        {
            strcat(cmd_temp, pstr_temp);
        }
        if (i < num - 1)
            strcat(cmd_temp, ",");
    }
    strcat(cmd_temp, "\r\n");
    va_end(arg_list);
    sprintf(AT_cmd, cmd_temp);
}
/// @brief 生成AT指令并发送
/// @param num 指令参数数量
/// @param cmd 指令名称
/// @param
void ATcmd_MakeSend(int num, const char *cmd, ...)
{
    va_list arg_list;
    char cmd_temp[ATCMD_BUFF_LEN] = {0};
    char *pstr_temp;
    va_start(arg_list, cmd);
    strcat(cmd_temp, "AT+");
    strcat(cmd_temp, cmd);
    if (num)
        strcat(cmd_temp, "=");
    for (int i = 0; i < num; i++)
    {
        pstr_temp = va_arg(arg_list, char *);
        if (*pstr_temp < '0' || *pstr_temp > '9')
        {
            strcat(cmd_temp, "\"");
            strcat(cmd_temp, pstr_temp);
            strcat(cmd_temp, "\"");
        }
        else
        {
            strcat(cmd_temp, pstr_temp);
        }
        if (i < num - 1)
            strcat(cmd_temp, ",");
    }
    strcat(cmd_temp, "\r\n");
    va_end(arg_list);
    ATcmd_Send(cmd_temp);
}

/**
 * @brief 初始化蓝牙模块参数（波特率、设备名称等）
 * @return 成功返回1，失败返回0
 */
uint8_t BT24_AT_Init(char *DeviceName)
{
    if (BT24_GetStatus())
    {
        LOG_WARN("[BT24] Device Already Connected - Cannot Enter AT Mode\r\n");
        return 0;
    }

    ATcmd_UartInit(115200);
    LOG_INFO("[BT24] Initializing AT Command Interface\r\n");

    ATcmd_Set("AT\r\n");
    if (!ATcmd_Wait("OK", "Device not responding in baud_115200\r\n", 500, 3))
    {
        ATcmd_UartInit(9600);
        ATcmd_Set("AT\r\n");
        if (!ATcmd_Wait("OK", "Device not responding in baud_9600\r\n", 500, 3))
        {
            ATcmd_UartInit(DEBUG_BAUD);
            LOG_ERROR("[BT24] Device Initialization Failed: No Response at Baud Rates 9600/115200.\r\n");
            return 0;
        }
        LOG_INFO("[BT24] Device recognized (9600)\r\n");
    }
    else
    {
        LOG_INFO("[BT24] Device recognized (115200)\r\n");
    }

    ATcmd_Make(0, "BAUD");
    if (ATcmd_Wait("+BAUD", "Read BAUD failed\r\n", 500, 3))
    {
        LOG_INFO("[BT24] Current Baud Code = %d\r\n", Data_ScanInt("+BAUD=", 1));

        if (Data_ScanInt("+BAUD=", 1) != BT_DEVICE_BAUD)
        {
            LOG_INFO("[BT24] BAUD Changing...\r\n");
            char baud_cmd[8];
            sprintf(baud_cmd, "BAUD%d", BT_DEVICE_BAUD);
            ATcmd_Make(0, baud_cmd);
            if (ATcmd_Wait("OK", "Set BAUD failed\r\n", 500, 3))
            {
                LOG_INFO("[BT24] Change Baud Code = %d ,Reseting... \r\n", BT_DEVICE_BAUD);
                BT24_Reset();
                NVIC_SystemReset();
            }
            else
            {
                return 0;
            }
        }
    }

    ATcmd_Make(0, "NAME");
    if (ATcmd_Wait("+NAME", "Error:Read NAME failed\r\n", 500, 2))
    {
        char name_value[32];
        if (Data_Scan(name_value, "+NAME=", strlen(DeviceName)))
        {
            LOG_INFO("[BT24] Current Name = %s\r\n", name_value);

            if (strncmp(name_value, DeviceName, strlen(DeviceName)) != 0)
            {
                LOG_INFO("[BT24] Name Changing...\r\n");
                char name_cmd[32];
                sprintf(name_cmd, "NAME%s", DeviceName);
                ATcmd_Make(0, name_cmd);
                if (ATcmd_Wait("OK", "Set NAME failed\r\n", 500, 3))
                {
                    LOG_INFO("[BT24] Change Name = %s ,Reseting... \r\n", DeviceName);
                    BT24_Reset();
                }
                else
                {
                    return 0;
                }
            }
        }
    }
    LOG_INFO("[BT24] Bluetooth Name: %s\r\n", DeviceName);
    return 1;
}

// void test()
// {
//     ATcmd_Set("AT+QHTTPPOST=?\r\n");
//     ATcmd_Make(5, "QMTPUBEX", "0", "0", "0", "0", "aystmjz/topic/hxd");
//     ATcmd_Send(AT_cmd);
//     ATcmd_Wait("ok", NULL, 200, 1);
//     ATcmd_Make_Send(5,"QMTPUBEX","0","0","0","0","aystmjz/topic/hxd");
// }

void BT24_PubString(char *str)
{
    BT24_printf(str);
}
