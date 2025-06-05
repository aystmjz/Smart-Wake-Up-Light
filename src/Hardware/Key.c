#include "Key.h"

uint8_t Key_KeyNumber;

void Timer_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_InternalClockConfig(TIM2);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 10 - 1; // 1ms
	TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM2, ENABLE);
}

void Key_Init(void)
{
	Timer_Init();

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint8_t Key_Read(void)
{
	return !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);
}

uint8_t Key_GetNumber(void)
{
	uint8_t temp = Key_KeyNumber;
	if (temp)
	{
		Key_KeyNumber = 0;
		return temp;
	}
	else
		return 0;
}

uint8_t Key_Clear(void)
{
	Key_KeyNumber = 0;
	return Key_KeyNumber;
}

uint8_t Key_GetState(void)
{
	uint8_t KeyNumber = 0;
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 0)
	{
		KeyNumber = 1;
	}

	return KeyNumber;
}

void Key_Entry(void)
{
	static uint8_t NowState, LastState;
	static uint16_t HoldTimer, ClickTimer;
	static uint8_t ClickCount = 0;

	LastState = NowState;
	NowState = Key_GetState();

	if (LastState == 0 && NowState == 1) // 检测到按键按下
	{
		ClickCount++;
		if (ClickCount >= 2)
			Key_KeyNumber = 3;
		else
			Key_KeyNumber = 1;
		HoldTimer = 0;
		Buzzer_Flag = 1;
	}
	else if (LastState == 1 && NowState == 1) // 持续按住
	{
		HoldTimer++;
		if (HoldTimer > 20)
		{
			Key_KeyNumber = 2; // 长按事件
		}
	}

	// 更新双击计时器
	if (ClickCount && NowState == 0)
	{
		if (ClickTimer < 5) // 设置双击最大间隔时间
		{
			ClickTimer++;
		}
		else
		{
			ClickTimer = 0;
			ClickCount = 0; // 超时清零
		}
	}
}