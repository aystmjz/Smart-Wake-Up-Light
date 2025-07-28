#include "PWR.h"

void PWR_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
}

void PWR_STOP(void)
{
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    SystemInit();
    SCB->VTOR = FLASH_BASE | BOOTLOADER_SIZE;
}