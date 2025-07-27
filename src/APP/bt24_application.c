#include "bt24_application.h"

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
void BT24_PubData(PubDataTypeDef *pub_data)
{
    cJSON *data = NULL;
    cJSON *days = NULL;
    char data_temp[8];
    data = cJSON_CreateObject();
    days = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "Temp", (uint8_t)pub_data->SHT->Temp + ((uint8_t)(pub_data->SHT->Temp * 10) % 10) / 10.0);
    cJSON_AddNumberToObject(data, "Hum", (uint8_t)pub_data->SHT->Hum);
    cJSON_AddBoolToObject(data, "Alarm", pub_data->Alarm->Enable);
    sprintf(data_temp, "%02d:%02d", pub_data->Alarm->Hour, pub_data->Alarm->Min);
    cJSON_AddStringToObject(data, "Time", data_temp);
    cJSON_AddBoolToObject(days, "mon", pub_data->Set->WeekEnable[0]);
    cJSON_AddBoolToObject(days, "tue", pub_data->Set->WeekEnable[1]);
    cJSON_AddBoolToObject(days, "wed", pub_data->Set->WeekEnable[2]);
    cJSON_AddBoolToObject(days, "thu", pub_data->Set->WeekEnable[3]);
    cJSON_AddBoolToObject(days, "fri", pub_data->Set->WeekEnable[4]);
    cJSON_AddBoolToObject(days, "sat", pub_data->Set->WeekEnable[5]);
    cJSON_AddBoolToObject(days, "sun", pub_data->Set->WeekEnable[6]);
    cJSON_AddItemToObject(data, "Days", days);
    sprintf(data_temp, "%smin", Get_PWM_Str(&pub_data->Set->PwmMod));
    cJSON_AddStringToObject(data, "Mode", data_temp);
    cJSON_AddBoolToObject(data, "Buzzer", pub_data->Set->BuzzerEnable);
    cJSON_AddBoolToObject(data, "Muzic", pub_data->Set->MuzicEnable);
    BT24_PubJson(data);
    cJSON_Delete(data);
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
 * @param pub_data 目标数据结构体指针
 * @return 解析成功返回1，失败返回0
 */
uint8_t BT24_ParseData(const char *json_str, PubDataTypeDef *pub_data, char *extra_cmd)
{
    cJSON *root = cJSON_Parse(json_str);
    cJSON *item = NULL;
    if (!root)
        return 0;

    item = cJSON_GetObjectItem(root, "Alarm");
    if (cJSON_IsBool(item))
        pub_data->Alarm->Enable = cJSON_IsTrue(item);

    item = cJSON_GetObjectItem(root, "Time");
    if (cJSON_IsString(item))
    {
        int hour, min;
        if (sscanf(item->valuestring, "%d:%d", &hour, &min) == 2)
        {
            pub_data->Alarm->Hour = hour;
            pub_data->Alarm->Min = min;
        }
    }

    cJSON *days = cJSON_GetObjectItem(root, "Days");
    if (days)
    {
        pub_data->Set->WeekEnable[0] = cJSON_IsTrue(cJSON_GetObjectItem(days, "mon"));
        pub_data->Set->WeekEnable[1] = cJSON_IsTrue(cJSON_GetObjectItem(days, "tue"));
        pub_data->Set->WeekEnable[2] = cJSON_IsTrue(cJSON_GetObjectItem(days, "wed"));
        pub_data->Set->WeekEnable[3] = cJSON_IsTrue(cJSON_GetObjectItem(days, "thu"));
        pub_data->Set->WeekEnable[4] = cJSON_IsTrue(cJSON_GetObjectItem(days, "fri"));
        pub_data->Set->WeekEnable[5] = cJSON_IsTrue(cJSON_GetObjectItem(days, "sat"));
        pub_data->Set->WeekEnable[6] = cJSON_IsTrue(cJSON_GetObjectItem(days, "sun"));
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
                    pub_data->Set->PwmMod = current_mod;
                    break;
                }
            }

            if (pub_data->Set->PwmMod > PWM_NUM)
            {
                pub_data->Set->PwmMod = PWM_NUM;
            }
        }
    }

    item = cJSON_GetObjectItem(root, "Buzzer");
    if (cJSON_IsBool(item))
        pub_data->Set->BuzzerEnable = cJSON_IsTrue(item);

    item = cJSON_GetObjectItem(root, "Muzic");
    if (cJSON_IsBool(item))
        pub_data->Set->MuzicEnable = cJSON_IsTrue(item);

    item = cJSON_GetObjectItem(root, "Extra");
    if (cJSON_IsString(item))
    {
        sprintf(extra_cmd, "%s", item->valuestring);
    }

    cJSON_Delete(root);
    return 1;
}

void BT24_ProcessCommand(void)
{
    static char json_str[BT24_UART_REC_LEN], extra_cmd[EXTRA_CMD_BUFF];;
    if (BT24_FindValidJson((char *)BT24RxBuffer, BT24_UART_REC_LEN, json_str))
    {
        switch (BT24_ParseCmd(json_str))
        {
        case 0:
            BT24_ParseData(json_str, &PubData, extra_cmd);
            WriteAlarm(PubData.Alarm, &PubData.Set->PwmMod);
            W25Q128_WriteSetting(PubData.Set);
            DS3231_ReadAlarm(PubData.Alarm);
            W25Q128_ReadSetting(PubData.Set);
            PWM_AdjustAlarm(PubData.Alarm, &PubData.Set->PwmMod, 1);
            LOG_INFO("[BT24] Command Received: %s\r\n", strlen(extra_cmd) ? extra_cmd : "No extra_cmd Data");
            if (strstr(extra_cmd, "TIME+"))
            {
                time_t timestamp;
                sscanf(extra_cmd, "TIME+%ld", &timestamp);
                LOG_INFO("[BT24] timestamp: %ld\r\n", timestamp);

                timestamp += 8 * 3600; // 将UTC时间转换为北京时间
                struct tm *parsed_time = localtime(&timestamp);
                if (parsed_time != NULL)
                {
                    DS3231_WriteTime(parsed_time);
                    Time = *parsed_time;
                    LOG_INFO("[BT24] Time Updated Successfully: %04d-%02d-%02d %02d:%02d:%02d\r\n",
                             parsed_time->tm_year + 1900,
                             parsed_time->tm_mon + 1,
                             parsed_time->tm_mday,
                             parsed_time->tm_hour,
                             parsed_time->tm_min,
                             parsed_time->tm_sec);
                }
                else
                {
                    LOG_ERROR("[] Failed to convert timestamp to time structure\r\n");
                }
            }
            else if (strstr(extra_cmd, "NAME+"))
            {
                char name[32] = {0};
                char *pstr, *pData = name;
                pstr = strstr(extra_cmd, "NAME+") + 5;
                while (*pstr != '\0')
                {
                    *pData = *pstr;
                    pstr++;
                    pData++;
                }
                if (strlen(name) > 0)
                {
                    sprintf((char *)PubData.Set->DeviceName, "%s", name);
                    W25Q128_WriteSetting(PubData.Set);
                    LOG_INFO("[BT24] Setting device name to: %s\r\n", name);
                    LOG_WARN("Device Reseting...");
                    BT24_Reset();
                    NVIC_SystemReset();
                }
                else
                {
                    LOG_ERROR("Error: Invalid device name");
                }
            }
            else if (strstr(extra_cmd, "VOICE"))
            {
                WakeUp_Flag = 1;
            }
            else if (strstr(extra_cmd, "RESET"))
            {
                Delay_ms(100);
                NVIC_SystemReset();
            }
            Refresh_Flag = 1;
            break;
        case 1:
            ASRPRO_printf("%c%c", 0xaa, 0x01);
            LOG_INFO("[ASRPRO] onLedChange");
            // onLedChange
            break;
        case 2:
            ASRPRO_printf("%c%c", 0xaa, 0x02);
            LOG_INFO("[ASRPRO] onFanChange");
            // onFanChange
            break;

        default:
            break;
        }
    }
}