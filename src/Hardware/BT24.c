#include "BT24.h"

static char AT_cmd[ATCMD_BUFF_LEN];
void BT24_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOB, GPIO_Pin_10);
}

void BT24_Reset(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_10);
    Delay_ms(20);
    GPIO_ResetBits(GPIOB, GPIO_Pin_10);
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
    Debug_printf(SendBuf);
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

void BT24_PubJson(cJSON *json)
{
    char *str = cJSON_Print(json);
    BT24_PubString(str);
    cJSON_free(str);
}

uint8_t BT24_FindValidJson(char *buffer, uint16_t length, char *json_str)
{
    uint16_t start = 0xffff, end = 0;

    if (length < 5)
        return 0;

    for (uint16_t i = 0; i < length - 1; i++)
    {
        if (buffer[i] == '{' && buffer[i + 1] == '"')
        {
            start = i;
            break;
        }
    }
    if (start == 0xffff)
        return 0;

    for (uint16_t i = start + 3; i < length; i++)
    {
        if (buffer[i] == '}' && buffer[i - 1] == '"')
        {
            end = i;
            break;
        }
    }
    if (!end)
        return 0;

    uint16_t json_length = end - start + 1;
    memcpy(json_str, &buffer[start], json_length);
    json_str[json_length] = '\0';
    buffer[start] = '*';
    buffer[end] = '*';

    return 1;
}

// {
// "Cmd":0,
// "Temp":23.7,
// "Hum":24.2,
// "Alarm":false,
// "Time":"06:00",
// Days":{"mon":true,"tue":true,"wed":true,"thu":true,"fri":true,"sat":false,"sun":false},
// "Mode":"60min",
// "Voice":false,
// "Muzic":false,
// "Extra":""
// }
void BT24_PubData(PubDataTypeDef *PubData)
{
    cJSON *Data = NULL;
    cJSON *Days = NULL;
    char DataTemp[8];
    Data = cJSON_CreateObject();
    Days = cJSON_CreateObject();
    cJSON_AddNumberToObject(Data, "Temp", (uint8_t)PubData->SHT->Temp + ((uint8_t)(PubData->SHT->Temp * 10) % 10) / 10.0);
    cJSON_AddNumberToObject(Data, "Hum", (uint8_t)PubData->SHT->Hum);
    cJSON_AddBoolToObject(Data, "Alarm", PubData->Alarm->Enable);
    sprintf(DataTemp, "%02d:%02d", PubData->Alarm->Hour, PubData->Alarm->Min);
    cJSON_AddStringToObject(Data, "Time", DataTemp);
    cJSON_AddBoolToObject(Days, "mon", PubData->Set->WeekEnable[0]);
    cJSON_AddBoolToObject(Days, "tue", PubData->Set->WeekEnable[1]);
    cJSON_AddBoolToObject(Days, "wed", PubData->Set->WeekEnable[2]);
    cJSON_AddBoolToObject(Days, "thu", PubData->Set->WeekEnable[3]);
    cJSON_AddBoolToObject(Days, "fri", PubData->Set->WeekEnable[4]);
    cJSON_AddBoolToObject(Days, "sat", PubData->Set->WeekEnable[5]);
    cJSON_AddBoolToObject(Days, "sun", PubData->Set->WeekEnable[6]);
    cJSON_AddItemToObject(Data, "Days", Days);
    sprintf(DataTemp, "%smin", Get_PWM_Str(&PubData->Set->PwmMod));
    cJSON_AddStringToObject(Data, "Mode", DataTemp);
    cJSON_AddBoolToObject(Data, "Buzzer", PubData->Set->BuzzerEnable);
    cJSON_AddBoolToObject(Data, "Muzic", PubData->Set->MuzicEnable);
    BT24_PubJson(Data);
    cJSON_Delete(Data);
}

/**
 * @brief 从JSON字符串解析命令
 * @param json_str JSON字符串
 * @return 解析成功返回Cmd，失败返回-1
 */
int8_t BT24_ParseCmd(const char *json_str)
{
    cJSON *root = cJSON_Parse(json_str);
    cJSON *item = NULL;
    if (!root)
        return -1;

    item = cJSON_GetObjectItem(root, "Cmd");
    if (cJSON_IsNumber(item))
    {
        cJSON_Delete(root);
        return item->valueint;
    }
    else
    {
        cJSON_Delete(root);
        return -1;
    }
}

/**
 * @brief 从JSON字符串解析数据到结构体
 * @param json_str JSON字符串
 * @param PubData 目标数据结构体指针
 * @return 解析成功返回1，失败返回0
 */
uint8_t BT24_ParseData(const char *json_str, PubDataTypeDef *PubData, char *Extra)
{
    cJSON *root = cJSON_Parse(json_str);
    cJSON *item = NULL;
    if (!root)
        return 0;

    item = cJSON_GetObjectItem(root, "Alarm");
    if (cJSON_IsBool(item))
        PubData->Alarm->Enable = cJSON_IsTrue(item);

    item = cJSON_GetObjectItem(root, "Time");
    if (cJSON_IsString(item))
    {
        int hour, min;
        if (sscanf(item->valuestring, "%d:%d", &hour, &min) == 2)
        {
            PubData->Alarm->Hour = hour;
            PubData->Alarm->Min = min;
        }
    }

    cJSON *days = cJSON_GetObjectItem(root, "Days");
    if (days)
    {
        PubData->Set->WeekEnable[0] = cJSON_IsTrue(cJSON_GetObjectItem(days, "mon"));
        PubData->Set->WeekEnable[1] = cJSON_IsTrue(cJSON_GetObjectItem(days, "tue"));
        PubData->Set->WeekEnable[2] = cJSON_IsTrue(cJSON_GetObjectItem(days, "wed"));
        PubData->Set->WeekEnable[3] = cJSON_IsTrue(cJSON_GetObjectItem(days, "thu"));
        PubData->Set->WeekEnable[4] = cJSON_IsTrue(cJSON_GetObjectItem(days, "fri"));
        PubData->Set->WeekEnable[5] = cJSON_IsTrue(cJSON_GetObjectItem(days, "sat"));
        PubData->Set->WeekEnable[6] = cJSON_IsTrue(cJSON_GetObjectItem(days, "sun"));
    }

    item = cJSON_GetObjectItem(root, "Mode");
    if (cJSON_IsString(item))
    {
        int min_val;
        if (sscanf(item->valuestring, "%dmin", &min_val) == 1)
        {
            for (uint8_t current_mod = 1; current_mod <= PWM_NUM; current_mod++)
            {
                const char *pwm_str = Get_PWM_Str(&current_mod);
                if (atoi(pwm_str) == min_val)
                {
                    PubData->Set->PwmMod = current_mod;
                    break;
                }
            }

            if (PubData->Set->PwmMod > PWM_NUM)
            {
                PubData->Set->PwmMod = PWM_NUM;
            }
        }
    }

    item = cJSON_GetObjectItem(root, "Buzzer");
    if (cJSON_IsBool(item))
        PubData->Set->BuzzerEnable = cJSON_IsTrue(item);

    item = cJSON_GetObjectItem(root, "Muzic");
    if (cJSON_IsBool(item))
        PubData->Set->MuzicEnable = cJSON_IsTrue(item);

    item = cJSON_GetObjectItem(root, "Extra");
    if (cJSON_IsString(item))
    {
        sprintf(Extra, "%s", item->valuestring);
    }

    cJSON_Delete(root);
    return 1;
}