#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"
#include "ADCSample.h"

#define NUM_OF_CHANNEL 5
#define NUM_OF_SAMPLES 10
static unsigned char  i,j;

static unsigned short ADC_converted_value[NUM_OF_CHANNEL * NUM_OF_SAMPLES];

static float motor_current_ADC  = 0;
static float PV_bucker_voltage  = 0;
static float emergency_volatge  = 0;
static float PV_string_cur_up   = 0;
static float PV_string_cur_down = 0;

static float adc_out[NUM_OF_CHANNEL];

void ADCInit()
{
    GPIO_InitTypeDef GPIOInitStruc;
    ADC_InitTypeDef  ADCInitStruc;
    DMA_InitTypeDef  DMAInitStruc;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 ,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);  /*ADC clock 12MHz*/
    
    /*Motor current*/
    GPIOInitStruc.GPIO_Pin  = GPIO_Pin_0;
    GPIOInitStruc.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIOInitStruc);
    
    /*PVBucker voltage*/
    GPIOInitStruc.GPIO_Pin  = GPIO_Pin_1;
    GPIOInitStruc.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIOInitStruc);
    
    /*Emergency stop voltage*/
    GPIOInitStruc.GPIO_Pin  = GPIO_Pin_0;
    GPIOInitStruc.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIOInitStruc);
    
    /*PV string cur up*/
    GPIOInitStruc.GPIO_Pin  = GPIO_Pin_3;
    GPIOInitStruc.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIOInitStruc);
    
    /*PV string cur up*/
    GPIOInitStruc.GPIO_Pin  = GPIO_Pin_2;
    GPIOInitStruc.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIOInitStruc);
    
    /*ADC - DMA1C1*/
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    DMAInitStruc.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMAInitStruc.DMA_MemoryBaseAddr     = (uint32_t)ADC_converted_value;
    DMAInitStruc.DMA_DIR                = DMA_DIR_PeripheralSRC;
    DMAInitStruc.DMA_BufferSize         = NUM_OF_CHANNEL * NUM_OF_SAMPLES;
    DMAInitStruc.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMAInitStruc.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMAInitStruc.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMAInitStruc.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    DMAInitStruc.DMA_Mode               = DMA_Mode_Normal;
    DMAInitStruc.DMA_Priority           = DMA_Priority_High;
    DMAInitStruc.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1,&DMAInitStruc);
    DMA_Cmd(DMA1_Channel1, ENABLE);
    DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE);
    
    /*ADC1*/
    ADCInitStruc.ADC_Mode               = ADC_Mode_Independent;
    ADCInitStruc.ADC_ScanConvMode       = ENABLE;
    ADCInitStruc.ADC_ContinuousConvMode = ENABLE;
    ADCInitStruc.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
    ADCInitStruc.ADC_DataAlign          = ADC_DataAlign_Right;
    ADCInitStruc.ADC_NbrOfChannel       = NUM_OF_CHANNEL;
    ADC_Init(ADC1, &ADCInitStruc);
    
    ADC_RegularChannelConfig(ADC1,ADC_Channel_8, 1,ADC_SampleTime_239Cycles5); /*Motor current*/
    ADC_RegularChannelConfig(ADC1,ADC_Channel_9, 2,ADC_SampleTime_239Cycles5); /*PVBucker Voltage*/
    ADC_RegularChannelConfig(ADC1,ADC_Channel_10, 3,ADC_SampleTime_239Cycles5); /*Emergency stop voltage*/
    ADC_RegularChannelConfig(ADC1,ADC_Channel_12, 4,ADC_SampleTime_239Cycles5); /*PV string cur Down*/
    ADC_RegularChannelConfig(ADC1,ADC_Channel_13, 5,ADC_SampleTime_239Cycles5); /*PV string cur Top*/
    ADC_DMACmd(ADC1, ENABLE);
    
    ADC_Cmd(ADC1, ENABLE);
    
    ADC_ResetCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


float GetMotorCurrent()
{
    return (motor_current_ADC * 33.0f /4096.0f - 165.0f)/11.0f;
}

float GetPVBuckerVoltage()
{
    return PV_bucker_voltage*3.3f*12.914f/40960.0f;
}

float GetEmergencyVoltage()
{
    return emergency_volatge * 3.3f * 12.914f/40960.0f;
}

float GetPVStringDownCur()
{
    return (PV_string_cur_down * 33.0f /4096.0f - 165.0f)/11.0f;
}

float GetPVStringUpCur()
{
    return (PV_string_cur_up * 33.0f /4096.0f - 165.0f)/11.0f;
}

void ADCConvert()
{
    ADC_ClearFlag(ADC1,ADC_FLAG_EOC);
    DMA_Cmd(DMA1_Channel1, DISABLE);
    DMA1_Channel1->CNDTR = NUM_OF_CHANNEL * NUM_OF_SAMPLES;
    DMA_Cmd(DMA1_Channel1, ENABLE);
    ADC_DMACmd(ADC1, ENABLE);
    ADC1->CR2 |= 0x00000002;
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void DMA1_Channel1_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC1) != RESET)
    {
        ADC1->CR2 &= 0xFFFFFFFD;/*Disable continue mode*/
        ADC_SoftwareStartConvCmd(ADC1, DISABLE);
        ADC_DMACmd(ADC1, DISABLE);
        DMA_ClearITPendingBit(DMA1_IT_TC1);
        
        for(i = 0; i < NUM_OF_CHANNEL; i ++)
        {
            adc_out[i] = 0;
            for(j = 0; j < NUM_OF_SAMPLES; j++)
            {
                adc_out[i] += (float)ADC_converted_value[i+j*NUM_OF_CHANNEL];
            }
        }
        motor_current_ADC  = adc_out[0];/*Prevent reading during statistics, causing errors*/
        PV_bucker_voltage  = adc_out[1];
        emergency_volatge  = adc_out[2];
        PV_string_cur_down = adc_out[3];
        PV_string_cur_up   = adc_out[4];
        
        DMA_Cmd(DMA1_Channel1, DISABLE);
    }
}
