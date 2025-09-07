#include "Battery.h"

static BatteryState State; // 电池状态（0=正常，1=充电中，2=充满）
static uint8_t Level;      // 当前电池电量百分比

/**
 * 根据ADC值查表计算电池电量百分比
 * @param battery_ADC 电池电压的ADC读数
 * @return 电池电量百分比 (0-100)
 */
static uint8_t Battery_Calculate(uint16_t battery_ADC)
{
    uint8_t batteryLevel;

    // 调整ADC值以匹配电池电压曲线
    battery_ADC -= BATTERY_FULL - 4160;
    battery_ADC -= battery_ADC % 4;

    // 边界检查：高于4160认为是100%，低于3580认为是0%
    if (battery_ADC > 4160)
        return 100;
    if (battery_ADC < 3580)
        return 0;

    // clang-format off
	// 使用查表法将ADC值转换为电量百分比
	switch (battery_ADC)
	{
	case 4160:batteryLevel=100;break;
	case 4156:batteryLevel=99;break;
	case 4152:batteryLevel=99;break;
	case 4148:batteryLevel=99;break;
	case 4144:batteryLevel=98;break;
	case 4140:batteryLevel=97;break;
	case 4136:batteryLevel=97;break;
	case 4132:batteryLevel=96;break;
	case 4128:batteryLevel=96;break;
	case 4124:batteryLevel=96;break;
	case 4120:batteryLevel=95;break;
	case 4116:batteryLevel=95;break;
	case 4112:batteryLevel=94;break;
	case 4108:batteryLevel=93;break;
	case 4104:batteryLevel=93;break;
	case 4100:batteryLevel=92;break;
	case 4096:batteryLevel=92;break;
	case 4092:batteryLevel=91;break;
	case 4088:batteryLevel=90;break;
	case 4084:batteryLevel=90;break;
	case 4080:batteryLevel=90;break;//90
	case 4076:batteryLevel=90;break;
	case 4072:batteryLevel=89;break;
	case 4068:batteryLevel=89;break;
	case 4064:batteryLevel=89;break;
	case 4060:batteryLevel=88;break;
	case 4056:batteryLevel=88;break;
	case 4052:batteryLevel=87;break;//87
	case 4048:batteryLevel=87;break;
	case 4044:batteryLevel=87;break;
	case 4040:batteryLevel=86;break;
	case 4036:batteryLevel=86;break;
	case 4032:batteryLevel=85;break;//85
	case 4028:batteryLevel=85;break;
	case 4024:batteryLevel=85;break;
	case 4020:batteryLevel=84;break;
	case 4016:batteryLevel=84;break;
	case 4012:batteryLevel=84;break;
	case 4008:batteryLevel=83;break;
	case 4004:batteryLevel=83;break;
	case 4000:batteryLevel=83;break;
	case 3996:batteryLevel=82;break;
	case 3992:batteryLevel=82;break;
	case 3988:batteryLevel=82;break;
	case 3984:batteryLevel=81;break;
	case 3980:batteryLevel=80;break;
	case 3976:batteryLevel=80;break;
	case 3972:batteryLevel=80;break;//80
	case 3968:batteryLevel=80;break;
	case 3964:batteryLevel=79;break;
	case 3960:batteryLevel=79;break;
	case 3956:batteryLevel=78;break;
	case 3952:batteryLevel=78;break;
	case 3948:batteryLevel=77;break;
	case 3944:batteryLevel=77;break;
	case 3940:batteryLevel=76;break;
	case 3936:batteryLevel=75;break;
	case 3932:batteryLevel=75;break;//75
	case 3928:batteryLevel=74;break;
	case 3924:batteryLevel=73;break;
	case 3920:batteryLevel=73;break;
	case 3916:batteryLevel=72;break;
	case 3912:batteryLevel=71;break;
	case 3908:batteryLevel=71;break;
	case 3904:batteryLevel=70;break;
	case 3900:batteryLevel=70;break;//70
	case 3896:batteryLevel=70;break;
	case 3892:batteryLevel=69;break;
	case 3888:batteryLevel=68;break;
	case 3884:batteryLevel=67;break;
	case 3880:batteryLevel=66;break;
	case 3876:batteryLevel=65;break;
	case 3872:batteryLevel=65;break;//65
	case 3868:batteryLevel=65;break;
	case 3864:batteryLevel=64;break;
	case 3860:batteryLevel=63;break;
	case 3856:batteryLevel=62;break;
	case 3852:batteryLevel=62;break;
	case 3848:batteryLevel=61;break;
	case 3844:batteryLevel=61;break;
	case 3840:batteryLevel=60;break;//60
	case 3836:batteryLevel=60;break;
	case 3832:batteryLevel=59;break;
	case 3828:batteryLevel=58;break;
	case 3824:batteryLevel=57;break;
	case 3820:batteryLevel=56;break;
	case 3816:batteryLevel=55;break;
	case 3812:batteryLevel=55;break;//55
	case 3808:batteryLevel=54;break;
	case 3804:batteryLevel=53;break;
	case 3800:batteryLevel=52;break;
	case 3796:batteryLevel=51;break;
	case 3792:batteryLevel=50;break;//50
	case 3788:batteryLevel=49;break;
	case 3784:batteryLevel=48;break;
	case 3780:batteryLevel=47;break;
	case 3776:batteryLevel=46;break;
	case 3772:batteryLevel=45;break;//45
	case 3768:batteryLevel=44;break;
	case 3764:batteryLevel=43;break;
	case 3760:batteryLevel=42;break;//42
	case 3756:batteryLevel=41;break;//41
	case 3752:batteryLevel=40;break;//40
	case 3748:batteryLevel=39;break;
	case 3744:batteryLevel=38;break;
	case 3740:batteryLevel=37;break;
	case 3736:batteryLevel=36;break;
	case 3732:batteryLevel=35;break;//35
	case 3733:batteryLevel=34;break;
	case 3731:batteryLevel=33;break;
	case 3730:batteryLevel=32;break;
	case 3729:batteryLevel=31;break;
	case 3728:batteryLevel=30;break;//30
	case 3724:batteryLevel=29;break;
	case 3720:batteryLevel=28;break;
	case 3716:batteryLevel=27;break;
	case 3712:batteryLevel=26;break;
	case 3708:batteryLevel=25;break;//25
	case 3704:batteryLevel=24;break;
	case 3700:batteryLevel=23;break;
	case 3696:batteryLevel=22;break;
	case 3691:batteryLevel=21;break;
	case 3692:batteryLevel=20;break;///20
	case 3688:batteryLevel=19;break;
	case 3684:batteryLevel=18;break;
	case 3680:batteryLevel=17;break;
	case 3676:batteryLevel=16;break;
	case 3672:batteryLevel=15;break;//15
	case 3668:batteryLevel=14;break;
	case 3664:batteryLevel=13;break;
	case 3660:batteryLevel=12;break;//12
	case 3656:batteryLevel=11;break;
	case 3652:batteryLevel=10;break;//10
	case 3648:batteryLevel=9;break;
	case 3644:batteryLevel=8;break;//8
	case 3640:batteryLevel=8;break;
	case 3636:batteryLevel=7;break;
	case 3632:batteryLevel=6;break;
	case 3628:batteryLevel=5;break;//5
	case 3624:batteryLevel=4;break;
	case 3620:batteryLevel=4;break;
	case 3616:batteryLevel=3;break;
	case 3612:batteryLevel=3;break;//3
	case 3608:batteryLevel=3;break;
	case 3604:batteryLevel=3;break;
	case 3600:batteryLevel=2;break;
	case 3596:batteryLevel=2;break;
	case 3592:batteryLevel=1;break;//1
	case 3588:batteryLevel=1;break;
	case 3584:batteryLevel=1;break;
	case 3580:batteryLevel=0;break;//0
	}
    // clang-format on
    return batteryLevel;
}

/**
 * 更新电池电量和状态
 * @param battery_ADC 电池电压的ADC读数
 */
void Battery_UpdateLevel(uint16_t battery_ADC)
{
    static uint8_t batteryLevel_Last[5], batteryLevel_Init_Flag = 1, batteryLevel_Index = 0,
                                         lastBattery;
    uint16_t batteryLevel = 0;

    // 根据硬件配置调整ADC值
    battery_ADC *= BATTERY_MULT;

    // 重置电池状态
    State = BATTERY_STATE_NORMAL;

    // 检查是否正在充电（电压高于充电阈值）
    if (battery_ADC > BATTERY_CHARGE)
    {
        batteryLevel_Init_Flag = 1;
        State                  = BATTERY_STATE_CHARGING;
        return;
    }

    // 检查是否充满（电压高于充满阈值但低于充电阈值）
    if (battery_ADC > BATTERY_DC)
    {
        batteryLevel_Init_Flag = 1;
        State                  = BATTERY_STATE_FULL;
        return;
    }

    // 初始化历史电量数组
    if (batteryLevel_Init_Flag)
    {
        for (uint8_t i = 0; i < 5; i++)
            batteryLevel_Last[i] = Battery_Calculate(battery_ADC) + 5;
        lastBattery            = Battery_Calculate(battery_ADC) + 5;
        batteryLevel_Init_Flag = 0;
    }

    // 更新历史电量数组（循环缓冲区）
    batteryLevel_Last[batteryLevel_Index] = Battery_Calculate(battery_ADC);
    batteryLevel_Index++;
    batteryLevel_Index %= 5;

    // 计算平均电量
    for (uint8_t i = 0; i < 5; i++)
    {
        batteryLevel += batteryLevel_Last[i];
    }

    // 只有当电量下降时才更新，防止电量跳变
    if ((batteryLevel / 5) < lastBattery)
        Level = batteryLevel / 5;
}

/**
 * 获取当前电池电量
 * @return 电池电量百分比
 */
uint8_t Battery_GetLevel(void)
{
    return Level;
}

/**
 * 获取当前电池状态
 * @return 电池状态（0=正常，1=充电中，2=充满）
 */
BatteryState Battery_GetState(void)
{
    return State;
}

// int16_t AD_Value = 0;
// float Voltage;
// while (1)
// {
// 	Delay_ms(100);
// 	AD_Value = AD_GetValue();
// 	Battery_UpdateLevel(AD_Value);
// 	Voltage = (float)AD_Value / 4095.0  * 3.3;
// 	sprintf(Debug_str, "AD_Value:%d Voltage:%f Battery:%d State:%d\r\n", AD_Value*2,
// Voltage*2,Battery_GetLevel(),Battery_GetState()); 	Debug_printf(Debug_str);
// }