#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "LED.h"


/***********************************************************************************************************************
Public function
***********************************************************************************************************************/
void LEDInit()
{
    GPIO_InitTypeDef GPIOInitStruc;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
    
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_1;
    GPIOInitStruc.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIOInitStruc.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOE,&GPIOInitStruc);
    
    GPIO_SetBits(GPIOE,GPIO_Pin_1);
}

void LEDTurnOn()
{
    GPIO_ResetBits(GPIOE,GPIO_Pin_1);
}

void LEDTurnOff()
{
    GPIO_SetBits(GPIOE,GPIO_Pin_1);
}

void LEDFlash()
{
    static unsigned char led_state = 0;
    if(led_state == 0)
    {
        led_state = 1;
        GPIO_ResetBits(GPIOE,GPIO_Pin_1);
    }
    else
    {
        led_state = 0;
        GPIO_SetBits(GPIOE,GPIO_Pin_1);
    }

}
