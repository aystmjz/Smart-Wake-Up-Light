#include "DS3231.h"

// RTC寄存器地址定义
static const uint8_t RTC_CLOCK_ADDR[7]  = {0x00, 0x01, 0x02, 0x04,
                                           0x05, 0x03, 0x06}; // 秒分时日月周年
static const uint8_t RTC_ALARM1_ADDR[4] = {0x07, 0x08, 0x09, 0x0A};
static const uint8_t RTC_ALARM2_ADDR[3] = {0x0B, 0x0C, 0x0D};

/**
 * @brief 向DS3231写入一个字节数据
 * @param WordAddress 寄存器地址
 * @param Data 要写入的数据
 * @details 通过I2C接口向DS3231指定寄存器地址写入数据
 */
static void DS3231_WriteByte(uint8_t WordAddress, uint8_t Data)
{
    DS3231_I2C_Start();
    DS3231_I2C_SendByte(DS3231_ADDRESS);
    DS3231_I2C_ReceiveAck();
    DS3231_I2C_SendByte(WordAddress);
    DS3231_I2C_ReceiveAck();
    DS3231_I2C_SendByte(Data);
    DS3231_I2C_ReceiveAck();
    DS3231_I2C_Stop();
}

/**
 * @brief 从DS3231读取一个字节数据
 * @param WordAddress 寄存器地址
 * @details 通过I2C接口从DS3231指定寄存器地址读取数据
 */
static uint8_t DS3231_ReadByte(uint8_t WordAddress)
{
    uint8_t Data;
    DS3231_I2C_Start();
    DS3231_I2C_SendByte(DS3231_ADDRESS);
    DS3231_I2C_ReceiveAck();
    DS3231_I2C_SendByte(WordAddress);
    DS3231_I2C_ReceiveAck();
    DS3231_I2C_Stop();
    DS3231_I2C_Start();
    DS3231_I2C_SendByte(DS3231_ADDRESS | 0x01);
    DS3231_I2C_ReceiveAck();
    Data = DS3231_I2C_ReceiveByte();
    DS3231_I2C_SendAck(1);
    DS3231_I2C_Stop();
    return Data;
}

/**
 * @brief 读取DS3231时钟数据
 * @param Time 时间结构体指针
 * @details 从DS3231读取当前时间数据，并转换为标准时间格式
 */
void DS3231_ReadTime(struct tm *Time)
{
    uint8_t temp, time_temp[7];

    // 依次读取秒、分、时、日、月、周、年寄存器数据
    for (uint8_t i = 0; i < 7; i++)
    {
        temp = DS3231_ReadByte(RTC_CLOCK_ADDR[i]);
        // 将BCD码转换为十进制
        time_temp[i] = (temp / 16) * 10 + (temp % 16);
    }

    // 将读取的数据转换为标准时间格式
    Time->tm_year = time_temp[6] + 2000 - 1900; // 年份转换
    Time->tm_mon  = time_temp[4] - 1;           // 月份转换
    Time->tm_mday = time_temp[3];               // 日期
    Time->tm_hour = time_temp[2];               // 小时
    Time->tm_min  = time_temp[1];               // 分钟
    Time->tm_sec  = time_temp[0];               // 秒

    mktime(Time); // 计算星期
}

/**
 * @brief 读取闹钟状态数据
 * @param Alarm 闹钟结构体指针
 * @return 闹钟状态，1表示已触发，0表示未触发
 * @details 读取DS3231状态寄存器中的闹钟标志位
 */
uint8_t DS3231_ReadStatus(AlarmTypeDef *Alarm)
{
    uint8_t temp;
    temp = DS3231_ReadByte(DS3231_STATUS); // 读取状态寄存器

    // 根据闹钟编号读取对应的标志位
    if (Alarm->Num == Alarm_1)
        Alarm->Status = temp & 0x01; // 闹钟1标志位
    else if (Alarm->Num == Alarm_2)
        Alarm->Status = (temp & 0x02) >> 1; // 闹钟2标志位

    return Alarm->Status;
}

/**
 * @brief 获取时间戳
 * @param Time 时间结构体指针
 * @return 时间戳(北京时间)
 * @details 将标准时间转换为时间戳，并调整为北京时间(UTC+8)
 */
time_t DS3231_GetTimeStamp(struct tm *Time)
{
    return (mktime(Time) - 8 * 60 * 60); // 减去8小时时差
}

/**
 * @brief 写入时钟数据到DS3231
 * @param Time 时间结构体指针
 * @details 将标准时间格式转换为BCD码并写入DS3231
 */
void DS3231_WriteTime(struct tm *Time)
{
    uint8_t time[7];
    mktime(Time); // 计算星期

    // 将时间数据转换为写入DS3231的格式
    time[6] = Time->tm_year + 1900 - 2000; // 年份转换
    time[4] = Time->tm_mon + 1;            // 月份转换
    time[5] = Time->tm_wday;               // 星期
    time[3] = Time->tm_mday;               // 日期
    time[2] = Time->tm_hour;               // 小时
    time[1] = Time->tm_min;                // 分钟
    time[0] = Time->tm_sec;                // 秒

    // 依次将时间数据写入对应的寄存器
    for (uint8_t i = 0; i < 7; i++)
    {
        // 将十进制转换为BCD码后写入
        DS3231_WriteByte(RTC_CLOCK_ADDR[i], (time[i] / 10) * 16 + time[i] % 10);
    }
}

/**
 * @brief 初始化闹钟结构体默认值
 * @param Alarm 闹钟结构体指针
 * @details 设置闹钟结构体的默认参数值
 */
void DS3231_InitAlarm(AlarmTypeDef *Alarm)
{
    Alarm->Day    = 0;
    Alarm->Week   = 0;
    Alarm->Hour   = 0;
    Alarm->Min    = 0;
    Alarm->Enable = 1;               // 默认启用闹钟
    Alarm->Num    = Alarm_1;         // 默认闹钟1
    Alarm->Mod    = Alarm_MatchHour; // 默认匹配模式
}

/**
 * @brief 写入闹钟设置到DS3231
 * @param Alarm 闹钟结构体指针
 * @details 根据闹钟结构体的设置将闹钟参数写入DS3231
 */
void DS3231_WriteAlarm(AlarmTypeDef *Alarm)
{
    uint8_t temp;

    // 根据闹钟编号设置对应的寄存器
    if (Alarm->Num == Alarm_1)
    {
        DS3231_WriteByte(RTC_ALARM1_ADDR[0], 0); // 秒寄存器设为0
        // 写入分钟寄存器
        DS3231_WriteByte(RTC_ALARM1_ADDR[1],
                         ((Alarm->Mod & 0x02) << 6) | ((Alarm->Min / 10) << 4) | (Alarm->Min % 10));
        // 写入小时寄存器
        DS3231_WriteByte(RTC_ALARM1_ADDR[2], ((Alarm->Mod & 0x04) << 5) |
                                                 ((Alarm->Hour / 10) << 4) | (Alarm->Hour % 10));

        // 根据匹配模式写入日期/星期寄存器
        if (Alarm->Mod == Alarm_MatchWeek)
            DS3231_WriteByte(RTC_ALARM1_ADDR[3], ((Alarm->Mod & 0x08) << 4) | 0x40 | Alarm->Week);
        else
            DS3231_WriteByte(RTC_ALARM1_ADDR[3], ((Alarm->Mod & 0x08) << 4) |
                                                     ((Alarm->Day / 10) << 4) | (Alarm->Day % 10));

        // 更新控制寄存器，设置闹钟1使能位
        temp = DS3231_ReadByte(DS3231_CONTROL);
        DS3231_WriteByte(DS3231_CONTROL, 0x04 | temp & 0x02 | Alarm->Enable);
    }
    else if (Alarm->Num == Alarm_2)
    {
        // 写入分钟寄存器
        DS3231_WriteByte(RTC_ALARM2_ADDR[0],
                         ((Alarm->Mod & 0x02) << 6) | ((Alarm->Min / 10) << 4) | (Alarm->Min % 10));
        // 写入小时寄存器
        DS3231_WriteByte(RTC_ALARM2_ADDR[1], ((Alarm->Mod & 0x04) << 5) |
                                                 ((Alarm->Hour / 10) << 4) | (Alarm->Hour % 10));

        // 根据匹配模式写入日期/星期寄存器
        if (Alarm->Mod == Alarm_MatchWeek)
            DS3231_WriteByte(RTC_ALARM2_ADDR[2], ((Alarm->Mod & 0x08) << 4) | 0x40 | Alarm->Week);
        else
            DS3231_WriteByte(RTC_ALARM2_ADDR[2], ((Alarm->Mod & 0x08) << 4) |
                                                     ((Alarm->Day / 10) << 4) | (Alarm->Day % 10));

        // 更新控制寄存器，设置闹钟2使能位
        temp = DS3231_ReadByte(DS3231_CONTROL);
        DS3231_WriteByte(DS3231_CONTROL, 0x04 | temp & 0x01 | Alarm->Enable << 1);
    }
}

/**
 * @brief 复位闹钟状态标志
 * @details 清除DS3231状态寄存器中的闹钟标志位
 */
void DS3231_ResetAlarm(void)
{
    DS3231_WriteByte(DS3231_STATUS, 0x00); // 清除状态寄存器
}

/**
 * @brief 从DS3231读取闹钟设置
 * @param Alarm 闹钟结构体指针
 * @details 从DS3231读取当前闹钟设置并保存到结构体中
 */
void DS3231_ReadAlarm(AlarmTypeDef *Alarm)
{
    uint8_t Temp[3];

    Temp[0] = DS3231_ReadByte(DS3231_CONTROL); // 读取控制寄存器

    // 根据闹钟编号读取对应的寄存器数据
    if (Alarm->Num == Alarm_1)
    {
        Alarm->Enable = Temp[0] & 0x01; // 读取闹钟1使能状态
        for (uint8_t i = 0; i < 3; i++)
        {
            Temp[i] = DS3231_ReadByte(RTC_ALARM1_ADDR[i + 1]); // 读取闹钟1设置
        }
    }
    else if (Alarm->Num == Alarm_2)
    {
        Alarm->Enable = (Temp[0] & 0x02) >> 1; // 读取闹钟2使能状态
        for (uint8_t i = 0; i < 3; i++)
        {
            Temp[i] = DS3231_ReadByte(RTC_ALARM2_ADDR[i]); // 读取闹钟2设置
        }
    }

    // 解析读取的数据
    Alarm->Mod = ((Temp[0] & 0x80) >> 6) | ((Temp[1] & 0x80) >> 5) | ((Temp[2] & 0x80) >> 4) |
                 ((Temp[2] & 0x40) >> 2);
    Alarm->Min  = ((Temp[0] & 0x70) >> 4) * 10 + (Temp[0] & 0x0F);
    Alarm->Hour = ((Temp[1] & 0x30) >> 4) * 10 + (Temp[1] & 0x0F);

    // 根据标志位判断是星期还是日期
    if (Temp[2] & 0x40)
        Alarm->Week = Temp[2] & 0x0F; // 星期模式
    else
        Alarm->Day = ((Temp[2] & 0x30) >> 4) * 10 + (Temp[2] & 0x0F); // 日期模式
}

/**
 * @brief 初始化DS3231时钟模块
 * @param Time 时间结构体指针
 * @param Alarm 闹钟结构体指针
 * @return 初始化状态，0表示成功，1表示失败
 * @details 初始化DS3231模块，包括I2C接口、闹钟状态和时间数据
 */
uint8_t DS3231_Init(struct tm *Time, AlarmTypeDef *Alarm)
{

    DS3231_I2C_Init();       // 初始化I2C接口
    DS3231_ResetAlarm();     // 复位闹钟状态
    DS3231_ReadTime(Time);   // 读取当前时间
    DS3231_ReadAlarm(Alarm); // 读取闹钟设置

    // 检查时间数据是否有效
    if (Time->tm_year < 120 || Time->tm_year > 150)
    {
        return 1; // 年份无效
    }

    // 检查各时间字段是否在有效范围内
    if (Time->tm_mon < 0 || Time->tm_mon > 11 || Time->tm_mday < 1 || Time->tm_mday > 31 ||
        Time->tm_hour < 0 || Time->tm_hour > 23 || Time->tm_min < 0 || Time->tm_min > 59 ||
        Time->tm_sec < 0 || Time->tm_sec > 59)
    {
        return 1; // 时间字段无效
    }

    return 0; // 初始化成功
}

/**
 * @brief 时间数据有效性检查和修正
 * @param Time 时间结构体指针
 * @details 检查时间数据的有效性，并对超出范围的值进行修正
 */
void TimeJudge(struct tm *Time)
{
    // 年份范围检查和修正
    if (Time->tm_year < 120)
        Time->tm_year = 150;
    else if (Time->tm_year > 150)
        Time->tm_year = 120;

    // 月份范围检查和修正
    if (Time->tm_mon < 0)
        Time->tm_mon = 11;
    else if (Time->tm_mon > 11)
        Time->tm_mon = 0;

    // 小时范围检查和修正
    if (Time->tm_hour < 0)
        Time->tm_hour = 23;
    else if (Time->tm_hour > 23)
        Time->tm_hour = 0;

    // 分钟范围检查和修正
    if (Time->tm_min < 0)
        Time->tm_min = 59;
    else if (Time->tm_min > 59)
        Time->tm_min = 0;

    // 秒范围检查和修正
    if (Time->tm_sec < 0)
        Time->tm_sec = 59;
    else if (Time->tm_sec > 59)
        Time->tm_sec = 0;

    // 日期范围检查和修正(考虑月份天数)
    uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // 检查闰年
    uint16_t year = Time->tm_year + 1900;
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        days_in_month[1] = 29; // 闰年2月有29天

    if (Time->tm_mday < 1)
        Time->tm_mday = days_in_month[Time->tm_mon];
    else if (Time->tm_mday > days_in_month[Time->tm_mon])
        Time->tm_mday = 1;

    mktime(Time); // 重新计算星期
}

/**
 * @brief 闹钟时间有效性检查和修正
 * @param Alarm 闹钟结构体指针
 * @details 检查闹钟时间的有效性，并对超出范围的值进行修正
 */
void Alarm_Judge(AlarmTypeDef *Alarm)
{
    // 小时范围检查和修正
    if (Alarm->Hour < 0)
        Alarm->Hour = 23;
    if (Alarm->Hour > 23)
        Alarm->Hour = 0;

    // 分钟范围检查和修正
    if (Alarm->Min < 0)
        Alarm->Min = 59;
    if (Alarm->Min > 59)
        Alarm->Min = 0;
}

/**
 * @brief 获取星期字符串
 * @param time_Week 星期数值(0-6)
 * @return 对应的星期字符串
 * @details 将数字星期转换为中文星期字符串
 */
char *Get_Week_Str(uint8_t time_Week)
{
    switch (time_Week)
    {
    case 0:
        return "日";
    case 1:
        return "一";
    case 2:
        return "二";
    case 3:
        return "三";
    case 4:
        return "四";
    case 5:
        return "五";
    case 6:
        return "六";
    }
    return 0;
}
