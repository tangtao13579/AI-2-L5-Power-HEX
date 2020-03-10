#ifndef _IIC_H_
#define _IIC_H_
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

typedef struct
{
    uint32_t      SDA_RCC;
    GPIO_TypeDef *SDA_group;
    uint16_t      SDA_pin;
    
    uint32_t      SCL_RCC;
    GPIO_TypeDef *SCL_group;
    uint16_t      SCL_pin;
    
} IIC_IO_Def;

void Delayus(signed char us);
void IICInit(IIC_IO_Def *IIC_IO);
int  IICRead(IIC_IO_Def *IIC_IO, unsigned char slave_addr, unsigned char command_code, unsigned short *out);
int  IICWrite(IIC_IO_Def *IIC_IO, unsigned char slave_addr, unsigned char command_code, unsigned short data);
int  IICReadByte(IIC_IO_Def *IIC_IO, unsigned char slave_addr, unsigned char command_code, unsigned char *out);
int  IICWriteByte(IIC_IO_Def *IIC_IO, unsigned char slave_addr, unsigned char command_code, unsigned char data);
#endif
