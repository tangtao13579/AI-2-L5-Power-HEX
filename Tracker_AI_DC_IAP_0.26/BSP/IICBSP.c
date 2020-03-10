#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "IIC.h"

/**********************************************************************************
 Private define
***********************************************************************************/
#define _IIC_SDA_HIGH() GPIO_SetBits(IIC_IO->SDA_group,IIC_IO->SDA_pin)
#define _IIC_SDA_LOW()  GPIO_ResetBits(IIC_IO->SDA_group,IIC_IO->SDA_pin)

#define _IIC_SCL_HIGH() GPIO_SetBits(IIC_IO->SCL_group,IIC_IO->SCL_pin)
#define _IIC_SCL_LOW()  GPIO_ResetBits(IIC_IO->SCL_group,IIC_IO->SCL_pin)


/**********************************************************************************
 Private functions
***********************************************************************************/
static void           IICStart(IIC_IO_Def *IIC_IO);
static void           IICStop(IIC_IO_Def *IIC_IO);
static unsigned char  IICGetACK(IIC_IO_Def *IIC_IO);
static void           IICSetACK(IIC_IO_Def *IIC_IO);
static void           IICSetNACK(IIC_IO_Def *IIC_IO);
static void           IICSendOneByte(IIC_IO_Def *IIC_IO, unsigned char data);
static void           IICGetOneByte(IIC_IO_Def *IIC_IO, unsigned char *out);

void Delayus(signed char us)
{
    signed char count;
    while(us-- >= 0)
    {
        count = 72;
        while(count-- >= 0);
    }
}
static void IICStart(IIC_IO_Def *IIC_IO)
{
    _IIC_SDA_HIGH();
    Delayus(2);
    _IIC_SCL_HIGH();
    Delayus(2);
    _IIC_SDA_LOW();
    Delayus(2);
}

static void IICStop(IIC_IO_Def *IIC_IO)
{
    _IIC_SCL_LOW();
    Delayus(1);
    _IIC_SDA_LOW();
    Delayus(1);
    _IIC_SCL_HIGH();
    Delayus(2);
    _IIC_SDA_HIGH();
    Delayus(1);
}

static unsigned char IICGetACK(IIC_IO_Def *IIC_IO)
{
    unsigned char return_value = 0;
    
    _IIC_SCL_LOW();
    _IIC_SDA_HIGH();       /* Release SDA */
    Delayus(1);
    _IIC_SCL_HIGH();
    Delayus(3);
    if(GPIO_ReadInputDataBit(IIC_IO->SDA_group,IIC_IO->SDA_pin) == Bit_SET)
    {
        return_value = !0;
    }
    _IIC_SCL_LOW();
    Delayus(1);
    
    return return_value;
}

static void IICSetACK(IIC_IO_Def *IIC_IO)
{
    _IIC_SCL_LOW();
    _IIC_SDA_LOW();
    Delayus(1);
    _IIC_SCL_HIGH();
    Delayus(5);
    _IIC_SCL_LOW();
    Delayus(1);
}

static void IICSetNACK(IIC_IO_Def *IIC_IO)
{
    _IIC_SCL_LOW();
    _IIC_SDA_HIGH();
    Delayus(1);
    _IIC_SCL_HIGH();
    Delayus(5);
    _IIC_SCL_LOW();
    Delayus(1);
}
static void IICSendOneByte(IIC_IO_Def *IIC_IO, unsigned char data)
{
    signed char         count;
    const unsigned char mask = 0x01;
    
    for (count = 7; count >= 0; count--)
    {
        _IIC_SCL_LOW();
        Delayus(1);
        if((data >> count & mask) != 0)
        {
            _IIC_SDA_HIGH();
        }
        else
        {
            _IIC_SDA_LOW();
        }
        Delayus(1);
        _IIC_SCL_HIGH();
        Delayus(5);
        _IIC_SCL_LOW();
        Delayus(1);
    }
}

static void IICGetOneByte(IIC_IO_Def *IIC_IO, unsigned char *out)
{
    signed char count;
    _IIC_SCL_LOW();
    Delayus(1);
    _IIC_SDA_HIGH();
    *out = 0;
    for (count = 7; count >= 0; count--)
    {
        _IIC_SCL_LOW();
        Delayus(1);
        _IIC_SCL_HIGH();
        Delayus(3);
        //*out &= 0xFE;
        if(GPIO_ReadInputDataBit(IIC_IO->SDA_group,IIC_IO->SDA_pin) == Bit_SET)
        {
            
            *out |= (0x01 << count);
        }
        //*out = *out << 1;
        _IIC_SCL_LOW();
        Delayus(2);
    }
}

/**********************************************************************************
 Interface functions
***********************************************************************************/
void IICInit(IIC_IO_Def *IIC_IO) 
{
    GPIO_InitTypeDef GPIOInitStruc;
    
    RCC_APB2PeriphClockCmd(IIC_IO->SDA_RCC,ENABLE);
    RCC_APB2PeriphClockCmd(IIC_IO->SCL_RCC,ENABLE);
    
    /*SDA Init*/
    GPIOInitStruc.GPIO_Pin   = IIC_IO->SDA_pin;
    GPIOInitStruc.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIOInitStruc.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IIC_IO->SDA_group,&GPIOInitStruc);
    GPIO_SetBits(IIC_IO->SDA_group,IIC_IO->SDA_pin);
    
    /* SDL Init*/
    GPIOInitStruc.GPIO_Pin   = IIC_IO->SCL_pin;
    GPIO_Init(IIC_IO->SCL_group,&GPIOInitStruc);
    GPIO_SetBits(IIC_IO->SCL_group,IIC_IO->SCL_pin);
}

int IICRead(IIC_IO_Def *IIC_IO, unsigned char slave_addr, unsigned char command_code, unsigned short *out)
{
    int           is_OK = 0;              /* return value */
    unsigned char out_temp;
    
    *out = 0;
    
    slave_addr &= 0xFE;
    
    IICStart(IIC_IO);                     /* S */
    IICSendOneByte(IIC_IO, slave_addr);   /* Slave address + Wr */
    if(IICGetACK(IIC_IO) != 0)            /* Ack by slave */
    {
        is_OK += -1;
    }
    IICSendOneByte(IIC_IO, command_code); /* Command code */
    if(IICGetACK(IIC_IO) != 0)            /* Ack by slave */
    {
        is_OK += -1;
    }
    
    slave_addr |= 0x01;
    
    IICStart(IIC_IO);                     /* Sr */
    IICSendOneByte(IIC_IO, slave_addr);   /* Slave address + Rd */
    if(IICGetACK(IIC_IO) != 0)            /* Ack by slave */
    {
        is_OK += -1;
    }
    
    IICGetOneByte(IIC_IO, &out_temp);     /* Out data byte low */
    *out |= out_temp;
    IICSetACK(IIC_IO);                    /* Ack by master */
    
    IICGetOneByte(IIC_IO, &out_temp);     /* Out data byte high */
    
    *out |= ((out_temp << 8) & 0xFF00);
    IICSetNACK(IIC_IO);                   /* NAck by master */
    
    IICStop(IIC_IO);
    
    return is_OK;
}

int IICWrite(IIC_IO_Def *IIC_IO, unsigned char slave_addr, unsigned char command_code, unsigned short data)
{
    int is_OK = 0;
    
    slave_addr &= 0xFE;
    
    IICStart(IIC_IO);                     /* S */
    IICSendOneByte(IIC_IO, slave_addr);   /* Slave address + Wr */
    if(IICGetACK(IIC_IO) != 0)            /* Ack by slave */
    {
        is_OK += -1;
    }
    IICSendOneByte(IIC_IO, command_code);   /* Command code */
    if(IICGetACK(IIC_IO) != 0)              /* Ack by slave */
    {
        is_OK += -1;
    }
    
    IICSendOneByte(IIC_IO, data&0xFF);      /* Send data low */
    if(IICGetACK(IIC_IO) != 0)              /* Ack by slave */
    {
        is_OK += -1;
    }
    
    IICSendOneByte(IIC_IO, (data>>8)&0xFF); /* Send data high */
    if(IICGetACK(IIC_IO) != 0)              /* Ack by slave */
    {
        is_OK += -1;
    }
    
    IICStop(IIC_IO);
    
    return is_OK;
}

int IICReadByte(IIC_IO_Def *IIC_IO, unsigned char slave_addr, unsigned char command_code, unsigned char *out)
{
    int           is_OK = 0;              /* return value */
    unsigned char out_temp;
    
    *out = 0;
    

    slave_addr &= 0xFE;
    
    IICStart(IIC_IO);                     /* S */
    IICSendOneByte(IIC_IO, slave_addr);   /* Slave address + Wr */
    if(IICGetACK(IIC_IO) != 0)            /* Ack by slave */
    {
        is_OK += -1;
    }
    IICSendOneByte(IIC_IO, command_code); /* Command code */
    if(IICGetACK(IIC_IO) != 0)            /* Ack by slave */
    {
        is_OK += -1;
    }
    
    slave_addr |= 0x01;
    
    IICStart(IIC_IO);                     /* Sr */
    IICSendOneByte(IIC_IO, slave_addr);   /* Slave address + Rd */
    if(IICGetACK(IIC_IO) != 0)            /* Ack by slave */
    {
        is_OK += -1;
    }
    
    IICGetOneByte(IIC_IO, &out_temp);     /* Out data byte low */
    *out = out_temp;
    
    IICSetNACK(IIC_IO);                   /* NAck by master */
    
    IICStop(IIC_IO);
    
    return is_OK;
}

int IICWriteByte(IIC_IO_Def *IIC_IO, unsigned char slave_addr, unsigned char command_code, unsigned char data)
{
    int is_OK = 0;
    
    slave_addr &= 0xFE;
    
    IICStart(IIC_IO);                     /* S */
    IICSendOneByte(IIC_IO, slave_addr);   /* Slave address + Wr */
    if(IICGetACK(IIC_IO) != 0)            /* Ack by slave */
    {
        is_OK += -1;
    }
    IICSendOneByte(IIC_IO, command_code);   /* Command code */
    if(IICGetACK(IIC_IO) != 0)              /* Ack by slave */
    {
        is_OK += -1;
    }
    
    IICSendOneByte(IIC_IO, data);      /* Send data */
    if(IICGetACK(IIC_IO) != 0)              /* Ack by slave */
    {
        is_OK += -1;
    }
    
    IICStop(IIC_IO);
    
    return is_OK;
}
