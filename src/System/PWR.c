#include "PWR.h"

/**
 * @brief  电源管理初始化函数
 * @details 使能PWR电源管理时钟，为进入低功耗模式做准备
 */
void PWR_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
}

/**
 * @brief  进入STOP模式
 * @details 使系统进入STOP低功耗模式，唤醒后重新初始化系统时钟并设置向量表偏移
 */
void PWR_STOP(void)
{
    // 进入STOP模式，使用低功耗稳压器，通过WFI指令等待中断唤醒
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

    // 系统唤醒后重新初始化时钟系统
    SystemInit();

    // 重新设置向量表偏移地址，指向应用程序起始地址
    SCB->VTOR = FLASH_BASE | BOOTLOADER_SIZE_SYS;
}