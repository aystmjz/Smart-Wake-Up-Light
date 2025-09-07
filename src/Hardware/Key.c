#include "Key.h"

static uint8_t KeyEvent;

/**
 * @brief 定时器初始化函数
 * @details 初始化TIM2定时器用于按键扫描，配置为1ms定时中断
 */
void Timer_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_InternalClockConfig(TIM2);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period        = 10 - 1; // 1ms
    TIM_TimeBaseInitStructure.TIM_Prescaler     = 7200 - 1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM2, ENABLE);
}

/**
 * @brief 按键初始化函数
 * @details 初始化按键硬件，包括定时器和GPIO引脚
 */
void Key_Init(void)
{
    Timer_Init();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
 * @brief 读取按键状态
 * @return 按键状态，1表示按下，0表示未按下
 * @details 直接读取GPIOB Pin5引脚电平状态并取反(按键按下为低电平)
 */
uint8_t Key_Read(void)
{
    return !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);
}

/**
 * @brief 清除按键事件缓存
 * @return 清除后的按键事件编号(KEY_NONE)
 * @details 将按键事件缓存清零
 */
uint8_t Key_Clear(void)
{
    KeyEvent = KEY_NONE;
    return KeyEvent;
}

/**
 * @brief 获取按键事件
 * @return 按键事件
 * @details 从按键事件缓存中读取按键事件，并清空缓存
 */
uint8_t Key_GetEvent(void)
{
    uint8_t temp = KeyEvent;

    if (temp)
    {
        KeyEvent = KEY_NONE;
        return temp;
    }
    else
        return KEY_NONE;
}

/**
 * @brief 按键处理入口函数
 * @details 按键状态机处理函数，识别单击、长按、双击等按键事件
 */
void Key_Entry(void)
{
    static uint8_t NowState, LastState;    // 当前状态和上次状态
    static uint16_t HoldTimer, ClickTimer; // 按住计时器和点击计时器
    static uint8_t ClickCount = 0;         // 点击计数器

    LastState = NowState;   // 保存上次状态
    NowState  = Key_Read(); // 获取当前状态

    // 按键状态机处理
    if (LastState == 0 && NowState == 1) // 检测到按键按下
    {
        ClickCount++;    // 点击计数加1
        HoldTimer   = 1; // 启动按住计时器
        Buzzer_Flag = 1; // 触发蜂鸣器
    }
    else if (LastState == 1 && NowState == 1) // 持续按住状态
    {
        if (HoldTimer)       // 如果按住计时器已启动
            HoldTimer++;     // 计时器递增
        if (HoldTimer >= 10) // 超过10个周期(10ms)判定为长按
        {
            KeyEvent  = KEY_LONG; // 设置长按事件
            HoldTimer = 0;        // 清零按住计时器
        }
    }
    else if (LastState == 1 && NowState == 0) // 松开按键
    {
        if (ClickCount >= 2)                  // 如果点击次数>=2
            KeyEvent = KEY_DOUBLE;            // 设置双击事件
        else if (HoldTimer && HoldTimer < 10) // 如果是短按
        {
            KeyEvent = KEY_CLICK; // 设置单击事件
        }
    }

    // 双击检测处理
    if (ClickCount && NowState == 0) // 有点击且当前未按下
    {
        if (ClickTimer < 5) // 双击间隔计时(5个周期)
        {
            ClickTimer++; // 计时器递增
        }
        else
        {
            ClickTimer = 0; // 清零点击计时器
            ClickCount = 0; // 超时清零点击计数器
        }
    }
}
