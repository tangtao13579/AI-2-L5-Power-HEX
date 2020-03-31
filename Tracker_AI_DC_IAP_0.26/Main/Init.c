#include "GlobalDefine.h"
#include "Init.h"
#include "misc.h"
#include "HardwareVersion.h"

extern GlobalVariableDef GlobalVariable;

static void ConfigParaInit()
{
    GlobalVariable.ConfigPara.ComID                 = 1;
    
    GlobalVariable.ConfigPara.AngleCorrectionValue  = 0;
	
	  GlobalVariable.ConfigPara.MotorOrientation      = 0;
    
    GlobalVariable.ConfigPara.BackAngle             = 90;
    GlobalVariable.ConfigPara.CleaningDockAngle     = 45;
    GlobalVariable.ConfigPara.ESoftLimitAngle       = 45;
    GlobalVariable.ConfigPara.WSoftLimitAngle       = 135;
    GlobalVariable.ConfigPara.EWindProtectionAngle  = 90;
    GlobalVariable.ConfigPara.WWindProtectionAngle  = 90;
    GlobalVariable.ConfigPara.EWTracingAccuracy     = 2;
    
    GlobalVariable.ConfigPara.Latitude              = 30.0;
    GlobalVariable.ConfigPara.Longitude             = 121.0;
    GlobalVariable.ConfigPara.TimeZone              = 8.0;
    
    GlobalVariable.ConfigPara.PostSpacing           = 4.4;
    GlobalVariable.ConfigPara.PVModuleWidth         = 1.95;
    GlobalVariable.ConfigPara.TerrainSlope          = 0.0;
    
    GlobalVariable.ConfigPara.UpStartTime           = 60;
    GlobalVariable.ConfigPara.UpWindSpeed           = 18;
    GlobalVariable.ConfigPara.LowStopTime           = 300;
    GlobalVariable.ConfigPara.LowWindSpeed          = 14;
    
    GlobalVariable.ConfigPara.MotorOverCurrentValue = 3.0;
}
static void FixedParaInit()
{
    HDVInit();
    GlobalVariable.FixePara.DeviceEdition = 0x1D00;/*Fireware:0.29*/
    GlobalVariable.FixePara.DeviceEdition |= GetHardwareVersion();
    GlobalVariable.FixePara.DeviceType    = 0x0009;
}


void ParaInit()
{
    ConfigParaInit();
    FixedParaInit();
}
void NVICInit()
{
    NVIC_InitTypeDef NVICInitStruc;
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    /*Enable the TIM3 gloabal Interrupt*/
    NVICInitStruc.NVIC_IRQChannel = TIM3_IRQn; 
    NVICInitStruc.NVIC_IRQChannelPreemptionPriority = 0;
    NVICInitStruc.NVIC_IRQChannelSubPriority = 1;
    NVICInitStruc.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVICInitStruc);
    
    NVICInitStruc.NVIC_IRQChannel = DMA1_Channel1_IRQn; /*ADC*/
    NVICInitStruc.NVIC_IRQChannelPreemptionPriority = 0;
    NVICInitStruc.NVIC_IRQChannelSubPriority = 2;
    NVICInitStruc.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVICInitStruc); 
    
    NVICInitStruc.NVIC_IRQChannel = UART4_IRQn;
    NVICInitStruc.NVIC_IRQChannelPreemptionPriority = 1;
    NVICInitStruc.NVIC_IRQChannelSubPriority = 0;
    NVICInitStruc.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVICInitStruc);
}

void IWDGInit()
{

    RCC_LSICmd(ENABLE);

    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)==RESET);

    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    IWDG_SetPrescaler(IWDG_Prescaler_64);

    IWDG_SetReload(4000); 

    IWDG_ReloadCounter();

    IWDG_Enable();
}

