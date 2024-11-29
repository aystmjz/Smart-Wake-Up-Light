#include "Key.h"

uint8_t Key_KeyNumber;

void Timer_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_InternalClockConfig(TIM2);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 9500 - 1; // 100ms
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
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

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

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
	int8_t Temp = 0;
	Temp = Key_KeyNumber;
	Key_KeyNumber = 0;
	if (Temp)
		return Temp;
}

uint8_t Key_Clear(void)
{
	Key_KeyNumber=0;
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
	static uint16_t HoldTimer;
	LastState = NowState;
	NowState = Key_GetState();
	if (LastState == 0 && NowState == 1)
	{
		Key_KeyNumber = 1;
		HoldTimer = 0;
	}
	else if (LastState == 1 && NowState == 1)
	{
		HoldTimer++;
		if (HoldTimer > 20)
			Key_KeyNumber = 2;
	}
}