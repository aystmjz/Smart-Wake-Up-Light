#include "ui_display.h"

void UI_Display_MainScreen()
{
    OLED_Clear(WHITE);
    OLED_Printf(Time_Hour < 10 ? 62 : 10, 4, OLED_52X104, BLACK, "%d", Time_Hour);
    OLED_Printf(104 + 10, 0, OLED_52X104, BLACK, ":");
    OLED_Printf(104 + 10 + 20, 4, OLED_52X104, BLACK, "%02d", Time_Min);
    OLED_Printf(8, 0, OLED_8X16, BLACK, "%d年%d月%d日 周%s %s %s", Time_Year, Time_Mon, Time_Day, Get_Week_Str(Time_Week), LowPower_Now ? "叶" : "  ", ASRPRO_Get_State() ? "麦 " : "  ");
    switch (Battery_GetState())
    {
    case 0:
        OLED_Printf(232, 5, OLED_6X8, BLACK, "%d%%", Battery_GetLevel() < 100 ? Battery_GetLevel() : 99);
        if (Battery_GetLevel() < 20)
            OLED_Printf(215, 0, OLED_8X16, BLACK, "%s", "零");
        else if (Battery_GetLevel() < 40)
            OLED_Printf(215, 0, OLED_8X16, BLACK, "%s", "壹");
        else if (Battery_GetLevel() < 60)
            OLED_Printf(215, 0, OLED_8X16, BLACK, "%s", "贰");
        else if (Battery_GetLevel() < 80)
            OLED_Printf(215, 0, OLED_8X16, BLACK, "%s", "叁");
        else
            OLED_Printf(215, 0, OLED_8X16, BLACK, "%s", "肆");
        break;
    case 1:
        OLED_Printf(215, 0, OLED_8X16, BLACK, "%s", "肆");
        OLED_Printf(215 + 16, 2, OLED_8X16, BLACK, "%s", "电");
        break;
    case 2:
        OLED_Printf(215, 0, OLED_8X16, BLACK, "%s", "肆插");
        break;
    default:
        break;
    }
    OLED_Printf(Alarm.Hour > 9 ? 8 : 16, 112, OLED_8X16, BLACK, "%.2f℃ %.0f%% %s%d:%02d 灯%smin %s", SHT.Temp, SHT.Hum, (Alarm.Enable && !LowPower_Now) ? "铃" : "否", Alarm.Hour, Alarm.Min, Get_PWM_Str(&Set.PwmMod), (Set.MuzicEnable) ? "乐" : " ");
    OLED_DrawLine(0, 20, LINE_END, 20, BLACK);
    OLED_DrawLine(0, 110, LINE_END, 110, BLACK);
    OLED_DrawLine(LINE_END, 0, LINE_END, OLED_H, BLACK);
    if (Set.WeekEnable[0])
        OLED_Printf(LINE_END + 6, OLED_8X16 * 0 + 1, OLED_8X16, BLACK, "周一");
    if (Set.WeekEnable[1])
        OLED_Printf(LINE_END + 6, OLED_8X16 * 1 + 3, OLED_8X16, BLACK, "周二");
    if (Set.WeekEnable[2])
        OLED_Printf(LINE_END + 6, OLED_8X16 * 2 + 5, OLED_8X16, BLACK, "周三");
    if (Set.WeekEnable[3])
        OLED_Printf(LINE_END + 6, OLED_8X16 * 3 + 7, OLED_8X16, BLACK, "周四");
    if (Set.WeekEnable[4])
        OLED_Printf(LINE_END + 6, OLED_8X16 * 4 + 9, OLED_8X16, BLACK, "周五");
    if (Set.WeekEnable[5])
        OLED_Printf(LINE_END + 6, OLED_8X16 * 5 + 11, OLED_8X16, BLACK, "周六");
    if (Set.WeekEnable[6])
        OLED_Printf(LINE_END + 6, OLED_8X16 * 6 + 13, OLED_8X16, BLACK, "周日");
}