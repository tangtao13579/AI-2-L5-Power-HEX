#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "RTC.h"

#define   RTC_PORT_CLK         RCC_APB2Periph_GPIOC
#define   RTC_PORT             GPIOC
#define   RTC_DA_Pin           GPIO_Pin_6
#define   RTC_CLK_Pin          GPIO_Pin_8
#define   RTC_CE_Pin           GPIO_Pin_9
#define   RTC_WR_Pin           GPIO_Pin_7

#define   RTC_CLK_IsSet()      GPIO_SetBits(RTC_PORT,   RTC_CLK_Pin)
#define   RTC_CLK_IsReset()    GPIO_ResetBits(RTC_PORT, RTC_CLK_Pin)
#define   RTC_CE_IsSet()       GPIO_SetBits(RTC_PORT,   RTC_CE_Pin)
#define   RTC_CE_IsReset()     GPIO_ResetBits(RTC_PORT, RTC_CE_Pin)
#define   RTC_WR_IsSet()       GPIO_SetBits(RTC_PORT,   RTC_WR_Pin)
#define   RTC_WR_IsReset()     GPIO_ResetBits(RTC_PORT, RTC_WR_Pin)
#define   RTC_DATA_IsSet()     GPIO_SetBits(RTC_PORT,   RTC_DA_Pin)
#define   RTC_DATA_IsReset()   GPIO_ResetBits(RTC_PORT, RTC_DA_Pin) 


static void DelayUs(unsigned int nus)
{
    unsigned int temp = 12;
    while(nus--)
    {
        temp = 12;
        while(temp--);
    }
}



void RTCInit()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RTC_PORT_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = RTC_CLK_Pin | RTC_CE_Pin | RTC_WR_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(RTC_PORT, &GPIO_InitStructure);
    
    /*DATA in*/
    GPIO_InitStructure.GPIO_Pin = RTC_DA_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(RTC_PORT, &GPIO_InitStructure);
}

void GetTime(unsigned short *second,
             unsigned short *minute,
             unsigned short *hour,
             unsigned short *day,
             unsigned short *month,
             unsigned short *year)
{
    unsigned char i = 0;
    unsigned char temp[7] = {0};
    
    RTC_CLK_IsReset();
    RTC_CE_IsSet();
    RTC_WR_IsReset();
    DelayUs(10);

    for(i=0;i<52;i++)
    {
        RTC_CLK_IsSet();
        DelayUs(10);

        if(i<24)
        {
            if(GPIO_ReadInputDataBit(RTC_PORT, RTC_DA_Pin)==1)
            {
                temp[i>>3] |= 1<<(i%8);
            }
        }
        else if(i<28)
        {
            if(GPIO_ReadInputDataBit(RTC_PORT, RTC_DA_Pin)==1)
            {
                temp[3] |= 1<<(i-24);
            }
        }
        else if(i<52)
        {
            if(GPIO_ReadInputDataBit(RTC_PORT, RTC_DA_Pin)==1)
            {
                temp[(i+4)>>3] |= 1<<((i+4)%8);
            }
        }

        RTC_CLK_IsReset();
        DelayUs(10);
    }

    RTC_CE_IsReset();
    
    /* s */
    *second = (temp[0]&0x0F) + ((temp[0]>>4)&0x07)*10;
    
    /* m */
    *minute = (temp[1]&0x0F) + ((temp[1]>>4)&0x07)*10;
    
    /* h */
    *hour = (temp[2]&0x0F) + ((temp[2]>>4)&0x03)*10;
   
    /* day */
    *day = (temp[4]&0x0F) + ((temp[4]>>4)&0x03)*10;
   
    /* month */
    *month = (temp[5]&0x0F) + ((temp[5]>>4)&0x01)*10;
    
    /* year */
    *year = (temp[6]&0x0F) + ((temp[6]>>4)&0x0F)*10 + 2010;
}

void SetTime(unsigned short second,
             unsigned short minute,
             unsigned short hour,
             unsigned short day,
             unsigned short month,
             unsigned short year)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    unsigned char i = 0;
    unsigned char j = 0;

    unsigned char time[7];

    time[0] = second & 0xFF;
    time[1] = minute & 0xFF;
    time[2] = hour & 0xFF;
    time[3] = 1;
    time[4] = day & 0xFF;
    time[5] = month & 0xFF;
    time[6] = (year - 2010) & 0xFF;

    // DATA配置为输出
    GPIO_InitStructure.GPIO_Pin = RTC_DA_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(RTC_PORT, &GPIO_InitStructure);

    RTC_CLK_IsReset();
    RTC_CE_IsSet();
    RTC_WR_IsSet();
    DelayUs(10);
    
    for(i=0;i<7;i++)
    {
        j= time[i];
        time[i] = (j/10)*16 + (j%10);
    }

    // s,m,h
    for(j=0;j<3;j++)
    {
        for(i=0;i<8;i++)
        {
            if(time[j]&(1<<i))
            {
                RTC_DATA_IsSet();
            }
            else
            {
                RTC_DATA_IsReset();
            }
            
            DelayUs(10);
            RTC_CLK_IsSet();
            
            DelayUs(10);
            RTC_CLK_IsReset();
            
        }
    }
    
    // week
    for(i=0;i<4;i++)
    {
        if(time[3]&(1<<i))
        {
            RTC_DATA_IsSet();
        }
        else
        {
            RTC_DATA_IsReset();
        }
        
        DelayUs(10);    
        RTC_CLK_IsSet();
        
        DelayUs(10);
        RTC_CLK_IsReset();
        
    }
    
    // day,month,year
    for(j=4;j<7;j++)
    {
        for(i=0;i<8;i++)
        {
            if(time[j]&(1<<i))
            {
                RTC_DATA_IsSet();
            }
            else
            {
                RTC_DATA_IsReset();
            }
            
            DelayUs(10);
            RTC_CLK_IsSet();
            
            DelayUs(10);
            RTC_CLK_IsReset();
            
        }
    }

    RTC_CE_IsReset();

    // DATA配置为输入
    GPIO_InitStructure.GPIO_Pin = RTC_DA_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(RTC_PORT, &GPIO_InitStructure);
}

