#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "HardwareVersion.h"
void HDVInit()
{
    GPIO_InitTypeDef GPIOInitStruc;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
    
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_7;
    GPIOInitStruc.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIOInitStruc.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOE,&GPIOInitStruc);
    
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_8;
    GPIO_Init(GPIOE,&GPIOInitStruc);
    
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_9;
    GPIO_Init(GPIOE,&GPIOInitStruc);

}

unsigned char GetHardwareVersion()
{
    unsigned char version = 0;
    version |= GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_9);
    version = version << 1;
    version |= GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_8);
    version = version << 1;
    version |= GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_7);
    
    return version ;
}
