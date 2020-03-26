#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "PVPower.h"

void PVPowerInit()
{
    GPIO_InitTypeDef GPIOInitStruc;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    
    /*7801 Run pin*/
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_8;
    GPIOInitStruc.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIOInitStruc.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIOInitStruc);
    GPIO_ResetBits(GPIOA,GPIO_Pin_8);  //open PV power
}

void OpenPVPower()
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_8);
}
void ClosePVPower()
{
    GPIO_SetBits(GPIOA,GPIO_Pin_8);
}

