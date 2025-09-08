#include "ADC.h"

/**
 * @brief  ADC初始化函数
 * @details 初始化ADC1模块，配置PB0引脚为模拟输入，设置ADC工作参数，并完成ADC校准
 */
void AD_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    // 配置PB0为模拟输入模式，用于ADC采样
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置ADC1的规则通道8（对应PB0引脚），转换顺序为1，采样时间为239.5个周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);

    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;
    ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ScanConvMode       = DISABLE;
    ADC_InitStructure.ADC_NbrOfChannel       = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Cmd(ADC1, ENABLE);

    // ADC1复位校准
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1) == SET)
        ;

    // ADC1开始校准
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) == SET)
        ;

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

/**
 * @brief  获取ADC转换值
 * @retval ADC转换结果(12位数据)
 * @details 启动一次ADC转换，等待转换完成后返回转换结果值
 */
uint16_t AD_GetValue(void)
{
    // 手动启动单次转换
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    // 等待转换完成标志位ADC_FLAG_EOC置位
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
        ;

    // 返回转换结果值
    return ADC_GetConversionValue(ADC1);
}
