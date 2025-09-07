#include "BT24.h"

static char AT_cmd[ATCMD_BUFF_LEN];

void BT24_GPIO_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
#ifndef BUILD_BOOT_LOADER
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif

#ifdef BUILD_BOOT_LOADER
    GPIO_ResetBits(GPIOB, GPIO_Pin_10);
#else

#if DEBUG_MODE == DEBUG_MODE_STM32 || DEBUG_MODE == DEBUG_MODE_ASRPRO
    GPIO_SetBits(GPIOB, GPIO_Pin_10);
#else
    GPIO_ResetBits(GPIOB, GPIO_Pin_10);
#endif
    GPIO_SetBits(GPIOA, GPIO_Pin_6);

#endif
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

// 持续复位蓝牙模块（保持复位状态）
void BT24_Reset_Assert(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_10);
}

// 解除持续复位（释放复位状态）
void BT24_Reset_Deassert(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_10);
}

// 发送低电平脉冲至KEY引脚(断开连接)
void BT24_Disconnect(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_6);
    Delay_ms(300);
    GPIO_SetBits(GPIOA, GPIO_Pin_6);
}

/**
 * @brief 获取蓝牙模块连接状态
 * @return 蓝牙连接状态，0表示未连接，1表示已连接
 */
BT24_Status BT24_GetStatus(void)
{
    return (BT24_Status)GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
}

// AT指令发送函数
static void ATcmd_Main_printf(char *SendBuf)
{
    BT24_printf(SendBuf);
}

// AT指令调试信息打印函数
static void ATcmd_Debug_printf(char *SendBuf)
{
    LOG_ERROR("[BT24] %s", SendBuf);
}

// 发送AT指令并延时
static void ATcmd_Send(char *ATcmd)
{
    ATcmd_Main_printf(ATcmd);
    Delay_ms(100);
}

/**
 * @brief 从接收缓存中扫描指定应答
 * @param Ack 指定应答字符串
 * @return 存在指定应答返回1，不存在返回0
 * @details 在全局接收缓存ATcmd_RxBuffer中查找指定的应答字符串
 */
static uint8_t ATcmd_Scan(char *Ack)
{
    char *strx;
    strx = strstr((const char *)ATcmd_RxBuffer, (const char *)Ack);
    if (strx)
        return 1;
    else
        return 0;
}

/**
 * @brief 从接收缓存中提取指定数据
 * @param Data 写入位置指针
 * @param Flag 数据标志字符串
 * @param Len  要提取的数据长度
 * @return 成功提取返回1，失败返回0
 * @details 在AT指令响应缓存中查找指定标志，从标志后提取指定长度的数据
 *          并存储到Data指向的内存位置，自动添加字符串结束符'\0'
 */
static uint8_t Data_Scan(char *Data, char *Flag, uint8_t Len)
{
    char *pstr, *pData = Data;
    uint8_t FlagLen = strlen(Flag);
    pstr            = strstr((const char *)ATcmd_RxBuffer, (const char *)Flag);
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

/**
 * @brief 从接收缓存中提取指定数据(整数)
 * @param Flag 数据标志字符串
 * @param Len  数据长度
 * @return 提取的整数值，失败返回0
 * @details 从AT指令响应缓存中查找指定标志后的数据，
 *          提取指定长度的字符串并转换为整数返回
 */
static uint16_t Data_ScanInt(char *Flag, uint8_t Len)
{
    char Data_Temp[10];
    if (!Data_Scan(Data_Temp, Flag, Len))
        return 0;
    return atoi(Data_Temp);
}

/**
 * @brief 发送AT_cmd中指令并等待目标答复
 * @param Ack 目标答复字符串
 * @param Error 错误提示信息
 * @param Wait_Time 等待答复的超时时间(毫秒)
 * @param Try_Time 重试次数，0表示无限重试
 * @return 成功返回1，失败返回0
 * @details 发送存储在全局变量AT_cmd中的AT指令，等待指定的应答，
 *          支持超时控制和重试机制
 */
static uint8_t ATcmd_Wait(char *Ack, char *Error, uint16_t Wait_Time, uint8_t Try_Time)
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

/**
 * @brief 生成AT指令到AT_cmd
 * @param num 指令参数数量
 * @param cmd 指令名称
 * @param ... 可变参数列表，包含指令参数
 * @details 根据提供的指令名称和参数生成完整的AT指令字符串，
 *          并存储在全局变量AT_cmd中，供后续发送使用
 */
static void ATcmd_Make(int num, const char *cmd, ...)
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

/**
 * @brief 生成AT指令并发送
 * @param num 指令参数数量
 * @param cmd 指令名称
 * @param ... 可变参数列表，包含指令参数
 * @details 根据提供的指令名称和参数生成完整的AT指令字符串，
 *          并立即通过串口发送出去，不等待响应
 */
static void ATcmd_MakeSend(int num, const char *cmd, ...)
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
 * @param DeviceName 要设置的蓝牙设备名称
 * @return 成功返回1，失败返回0
 */
uint8_t BT24_AT_Init(char *DeviceName)
{
    // 检查蓝牙模块是否已连接，如果已连接则无法进入AT模式进行配置
    if (BT24_GetStatus() == BT24_CONNECTED)
    {
        LOG_WARN("[BT24] Device Already Connected - Cannot Enter AT Mode\r\n");
        return 0;
    }

    // 初始化串口通信，首先尝试115200波特率
    ATcmd_UartInit(115200);
    LOG_INFO("[BT24] Initializing AT Command Interface\r\n");

    // 发送AT测试指令，检测模块是否响应
    ATcmd_Set("AT\r\n");
    if (!ATcmd_Wait("OK", "Device not responding in baud_115200\r\n", 500, 3))
    {
        // 如果115200波特率无响应，尝试9600波特率
        ATcmd_UartInit(9600);
        ATcmd_Set("AT\r\n");
        if (!ATcmd_Wait("OK", "Device not responding in baud_9600\r\n", 500, 3))
        {
            // 如果9600波特率也无响应，则初始化失败
            ATcmd_UartInit(DEBUG_BAUD);
            LOG_ERROR(
                "[BT24] Device Initialization Failed: No Response at Baud Rates 9600/115200.\r\n");
            return 0;
        }
        LOG_INFO("[BT24] Device recognized (9600)\r\n");
    }
    else
    {
        LOG_INFO("[BT24] Device recognized (115200)\r\n");
    }

    // 读取当前波特率设置
    ATcmd_Make(0, "BAUD");
    if (ATcmd_Wait("+BAUD", "Read BAUD failed\r\n", 500, 3))
    {
        LOG_INFO("[BT24] Current Baud Code = %d\r\n", Data_ScanInt("+BAUD=", 1));

        // 如果当前波特率与目标波特率不一致，则进行设置
        if (Data_ScanInt("+BAUD=", 1) != BT_DEVICE_BAUD)
        {
            LOG_INFO("[BT24] BAUD Changing...\r\n");
            char baud_cmd[8];
            sprintf(baud_cmd, "BAUD%d", BT_DEVICE_BAUD);
            ATcmd_Make(0, baud_cmd);
            if (ATcmd_Wait("OK", "Set BAUD failed\r\n", 500, 3))
            {
                LOG_INFO("[BT24] Change Baud Code = %d ,Reseting... \r\n", BT_DEVICE_BAUD);
                BT24_Reset();       // 复位蓝牙模块使新波特率生效
                NVIC_SystemReset(); // 系统重启以应用新配置
            }
            else
            {
                return 0;
            }
        }
    }

    // 读取并设置设备名称
    ATcmd_Make(0, "NAME");
    if (ATcmd_Wait("+NAME", "Error:Read NAME failed\r\n", 500, 2))
    {
        char name_value[32];
        if (Data_Scan(name_value, "+NAME=", strlen(DeviceName)))
        {
            LOG_INFO("[BT24] Current Name = %s\r\n", name_value);

            // 如果当前设备名称与目标名称不一致，则进行设置
            if (strncmp(name_value, DeviceName, strlen(DeviceName)) != 0)
            {
                LOG_INFO("[BT24] Name Changing...\r\n");
                char name_cmd[32];
                sprintf(name_cmd, "NAME%s", DeviceName);
                ATcmd_Make(0, name_cmd);
                if (ATcmd_Wait("OK", "Set NAME failed\r\n", 500, 3))
                {
                    LOG_INFO("[BT24] Change Name = %s ,Reseting... \r\n", DeviceName);
                    BT24_Reset(); // 重置蓝牙模块使新名称生效
                }
                else
                {
                    return 0;
                }
            }
        }
    }
    ATcmd_UartInit(DEBUG_BAUD);
    LOG_INFO("[BT24] Bluetooth Name: %s\r\n", DeviceName);

    return 1;
}

/**
 * @brief 通过蓝牙发送字符串数据
 * @param str 要发送的字符串
 */
void BT24_PubString(char *str)
{
    BT24_printf(str);
}

// void test()
// {
//     ATcmd_Set("AT+QHTTPPOST=?\r\n");
//     ATcmd_Make(5, "QMTPUBEX", "0", "0", "0", "0", "aystmjz/topic/hxd");
//     ATcmd_Send(AT_cmd);
//     ATcmd_Wait("ok", NULL, 200, 1);
//     ATcmd_Make_Send(5,"QMTPUBEX","0","0","0","0","aystmjz/topic/hxd");
// }