#include "bt24_application.h"

/**
 * @brief 发布JSON数据到蓝牙模块
 * @param json 指向待发送的cJSON对象指针
 * @details 将cJSON对象转换为字符串并通过蓝牙模块发送，发送完成后释放内存
 */
void BT24_PubJson(cJSON *json)
{
    char *str = cJSON_Print(json);
    BT24_PubString(str);
    cJSON_free(str);
}

/**
 * @brief 在数据缓冲区中查找有效的JSON字符串
 * @param buffer 数据缓冲区
 * @param length 缓冲区长度
 * @param json_str 存储找到的JSON字符串的目标缓冲区
 * @return 找到有效JSON返回1，否则返回0
 * @details 从缓冲区中查找以{开头、以}结尾的完整JSON字符串，并复制到目标字符串中
 */
uint8_t BT24_FindValidJson(char *buffer, uint16_t length, char *json_str)
{
    uint16_t start = 0xffff, end = 0;

    // 如果缓冲区长度小于5，不可能包含有效JSON，直接返回0
    if (length < 5)
        return 0;

    // 查找JSON开始位置（{后跟"）
    for (uint16_t i = 0; i < length - 1; i++)
    {
        if (buffer[i] == '{' && buffer[i + 1] == '"')
        {
            start = i;
            break;
        }
    }

    // 如果未找到开始位置，返回0
    if (start == 0xffff)
        return 0;

    // 从开始位置之后查找JSON结束位置（}前有"）
    for (uint16_t i = start + 3; i < length; i++)
    {
        if (buffer[i] == '}' && buffer[i - 1] == '"')
        {
            end = i;
            break;
        }
    }

    // 如果未找到结束位置，返回0
    if (!end)
        return 0;

    // 计算JSON字符串长度并复制到目标缓冲区
    uint16_t json_length = end - start + 1;
    memcpy(json_str, &buffer[start], json_length);
    json_str[json_length] = '\0'; // 添加字符串结束符

    // 将原缓冲区中的JSON标记为已处理
    buffer[start] = '*';
    buffer[end]   = '*';

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

/**
 * @brief 发布设备数据到蓝牙模块
 * @param pub_data 指向待发布的数据结构体指针
 * @details 将设备的各种状态数据打包成JSON格式并通过蓝牙模块发送
 */
void BT24_PubData(PubDataTypeDef *pub_data)
{
    cJSON *data = NULL;
    cJSON *days = NULL;
    char data_temp[8];

    // 创建JSON对象
    data = cJSON_CreateObject();
    days = cJSON_CreateObject();

    // 添加温度数据（保留一位小数）
    cJSON_AddNumberToObject(data, "Temp",
                            (uint8_t)pub_data->SHT->Temp +
                                ((uint8_t)(pub_data->SHT->Temp * 10) % 10) / 10.0);
    // 添加湿度数据
    cJSON_AddNumberToObject(data, "Hum", (uint8_t)pub_data->SHT->Hum);

    // 添加闹钟使能状态
    cJSON_AddBoolToObject(data, "Alarm", pub_data->Alarm->Enable);

    // 添加闹钟时间（格式化为HH:MM）
    sprintf(data_temp, "%02d:%02d", pub_data->Alarm->Hour, pub_data->Alarm->Min);
    cJSON_AddStringToObject(data, "Time", data_temp);

    // 添加星期使能状态
    cJSON_AddBoolToObject(days, "mon", pub_data->Set->WeekEnable[0]);
    cJSON_AddBoolToObject(days, "tue", pub_data->Set->WeekEnable[1]);
    cJSON_AddBoolToObject(days, "wed", pub_data->Set->WeekEnable[2]);
    cJSON_AddBoolToObject(days, "thu", pub_data->Set->WeekEnable[3]);
    cJSON_AddBoolToObject(days, "fri", pub_data->Set->WeekEnable[4]);
    cJSON_AddBoolToObject(days, "sat", pub_data->Set->WeekEnable[5]);
    cJSON_AddBoolToObject(days, "sun", pub_data->Set->WeekEnable[6]);
    cJSON_AddItemToObject(data, "Days", days);

    // 添加PWM模式（转换为分钟字符串）
    sprintf(data_temp, "%smin", Get_PWM_Str(&pub_data->Set->PwmMod));
    cJSON_AddStringToObject(data, "Mode", data_temp);

    // 添加蜂鸣器和音乐使能状态
    cJSON_AddBoolToObject(data, "Buzzer", pub_data->Set->BuzzerEnable);
    cJSON_AddBoolToObject(data, "Muzic", pub_data->Set->MuzicEnable);

    // 发送JSON数据
    BT24_PubJson(data);

    // 释放JSON对象内存
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

    // 如果解析失败，返回-1
    if (!root)
        return -1;
    // 获取Cmd字段
    item = cJSON_GetObjectItem(root, "Cmd");

    // 如果Cmd字段是数字类型，返回其值
    if (cJSON_IsNumber(item))
    {
        cJSON_Delete(root);
        return item->valueint;
    }
    else
    {
        // 如果不是数字类型，释放内存并返回-1
        cJSON_Delete(root);
        return -1;
    }
}

/**
 * @brief 从JSON字符串解析数据到结构体
 * @param json_str JSON字符串
 * @param pub_data 目标数据结构体指针
 * @param extra_cmd 额外命令字符串缓冲区
 * @return 解析成功返回1，失败返回0
 */
uint8_t BT24_ParseData(const char *json_str, PubDataTypeDef *pub_data, char *extra_cmd)
{
    cJSON *root = cJSON_Parse(json_str);
    cJSON *item = NULL;

    // 如果解析失败，返回0
    if (!root)
        return 0;

    // 解析闹钟使能状态
    item = cJSON_GetObjectItem(root, "Alarm");
    if (cJSON_IsBool(item))
        pub_data->Alarm->Enable = cJSON_IsTrue(item);

    // 解析闹钟时间
    item = cJSON_GetObjectItem(root, "Time");
    if (cJSON_IsString(item))
    {
        uint8_t hour, min;
        // 从字符串中提取小时和分钟
        if (sscanf(item->valuestring, "%hhu:%hhu", &hour, &min) == 2)
        {
            pub_data->Alarm->Hour = hour;
            pub_data->Alarm->Min  = min;
        }
    }

    // 解析星期使能状态
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

    // 解析PWM模式
    item = cJSON_GetObjectItem(root, "Mode");
    if (cJSON_IsString(item))
    {
        uint8_t min_val;
        // 从模式字符串中提取分钟数
        if (sscanf(item->valuestring, "%hhumin", &min_val) == 1)
        {
            // 根据分钟数查找对应的模式编号
            for (uint8_t current_mod = 1; current_mod <= PWM_NUM; current_mod++)
            {
                const char *pwm_str = Get_PWM_Str(&current_mod);
                if (atoi(pwm_str) == min_val)
                {
                    pub_data->Set->PwmMod = current_mod;
                    break;
                }
            }

            // 如果模式编号超出范围，设置为最大模式
            if (pub_data->Set->PwmMod > PWM_NUM)
            {
                pub_data->Set->PwmMod = PWM_NUM;
            }
        }
    }

    // 解析蜂鸣器使能状态
    item = cJSON_GetObjectItem(root, "Buzzer");
    if (cJSON_IsBool(item))
        pub_data->Set->BuzzerEnable = cJSON_IsTrue(item);

    // 解析音乐使能状态
    item = cJSON_GetObjectItem(root, "Muzic");
    if (cJSON_IsBool(item))
        pub_data->Set->MuzicEnable = cJSON_IsTrue(item);

    // 解析额外命令
    item = cJSON_GetObjectItem(root, "Extra");
    if (cJSON_IsString(item))
    {
        sprintf(extra_cmd, "%s", item->valuestring);
    }

    // 释放JSON对象内存
    cJSON_Delete(root);
    return 1;
}

/**
 * @brief 处理蓝牙接收到的命令
 * @details 解析蓝牙接收到的JSON命令并执行相应操作
 */
void BT24_ProcessCommand(void)
{
    static char json_str[BT24_UART_REC_LEN], extra_cmd[EXTRA_CMD_BUFF];

    // 查找并解析有效的JSON命令
    if (BT24_FindValidJson((char *)BT24RxBuffer, BT24_UART_REC_LEN, json_str))
    {
        // 根据命令类型执行相应操作
        switch (BT24_ParseCmd(json_str))
        {
        case 0:
            // 配置命令处理
            BT24_ParseData(json_str, &PubData, extra_cmd);
            WriteAlarm(PubData.Alarm, &PubData.Set->PwmMod);
            W25Q128_WriteSetting(PubData.Set);
            DS3231_ReadAlarm(PubData.Alarm);
            W25Q128_ReadSetting(PubData.Set);
            PWM_AdjustAlarm(PubData.Alarm, &PubData.Set->PwmMod, 1);

            LOG_INFO("[BT24] Command Received: %s\r\n",
                     strlen(extra_cmd) ? extra_cmd : "No extra_cmd Data");

            // 处理额外命令
            if (strstr(extra_cmd, "TIME+"))
            {
                // 时间同步命令
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
                             parsed_time->tm_year + 1900, parsed_time->tm_mon + 1,
                             parsed_time->tm_mday, parsed_time->tm_hour, parsed_time->tm_min,
                             parsed_time->tm_sec);
                }
                else
                {
                    LOG_ERROR("[] Failed to convert timestamp to time structure\r\n");
                }
            }
            else if (strstr(extra_cmd, "NAME+"))
            {
                // 设备重命名命令
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
                // 语音唤醒命令
                WakeUp_Flag = 1;
            }
            else if (strstr(extra_cmd, "RESET"))
            {
                // 系统复位命令
                Delay_ms(100);
                NVIC_SystemReset();
            }
            else if (strstr(extra_cmd, "BL"))
            {
                // 进入引导加载程序命令
                Delay_ms(100);
                *(volatile uint32_t *)BOOT_FLAG_ADDR = BOOT_FLAG_VALUE;
                NVIC_SystemReset();
            }

            Refresh_Flag = 1;
            break;

        case 1:
            // LED控制命令
            ASRPRO_printf("%c%c", 0xaa, 0x01);
            LOG_INFO("[ASRPRO] onLedChange");
            // onLedChange
            break;

        case 2:
            // 风扇控制命令
            ASRPRO_printf("%c%c", 0xaa, 0x02);
            LOG_INFO("[ASRPRO] onFanChange");
            // onFanChange
            break;

        default:
            break;
        }
    }
}
