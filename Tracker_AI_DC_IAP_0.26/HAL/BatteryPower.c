#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "IIC.h"
#include "LTC4015RegDefs.h"
#include "BatteryPower.h"

typedef struct
{
    float BatteryCapacity;
    float VChargeLimit;
    float IChargeLimit;
    float (*BatteryCharacter)(float BatteryOCV);
    
}BatteryParaDef;

static BatteryParaDef BatteryPara = {0};
static IIC_IO_Def _LTC4015IO;

/******************************************************************************************************************
                                                Private functions
*******************************************************************************************************************/
static void          HeatingInit(void);
static void          BatteryTypeIOInit(void);
static unsigned char GetBatteryType(void);
static float         BatteryCharacterB7030(float BatteryOCV);
static int           ChargerInit(void);

static void HeatingInit()
{
    GPIO_InitTypeDef GPIOInitStruc;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
    
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_10;
    GPIOInitStruc.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIOInitStruc.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOE,&GPIOInitStruc);
    
    /* Disable heating */
    GPIO_ResetBits(GPIOE, GPIO_Pin_10);
}

static void BatteryTypeIOInit()
{
    GPIO_InitTypeDef GPIOInitStruc;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
    
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_6;
    GPIOInitStruc.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIOInitStruc.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOD,&GPIOInitStruc);
    
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_7;
    GPIO_Init(GPIOD,&GPIOInitStruc);
}

static int ChargerInit()
{
    int is_OK = 0;
    
    if(BatteryPara.BatteryCharacter == 0)
    {
        return -1; /*Battery type is not initialized properly*/
    }

/*Charger init*/
    /*suspend charger*/
    is_OK = IICWrite(&_LTC4015IO,LTC4015_ADDR,LTC4015_CONFIG_BITS_ADDR,LTC4015_CONFIG_BITS_SUSPEND_CHARGER);
    /*Disable jeita*/
    is_OK += IICWrite(&_LTC4015IO,LTC4015_ADDR,LTC4015_CHARGE_CONFIG_BITS_ADDR,LTC4015_CHARGE_CONFIG_BITS_VALUE);
    /*IIN limit = 1A*/
    is_OK += IICWrite(&_LTC4015IO,LTC4015_ADDR,LTC4015_IIN_LIMIT_SETTING_ADDR,LTC4015_IIN_LIMIT_SETTING_VALUE);
    /*(ICHARGE_TARGET + 1) * 1mV /R_SNSB*/
    is_OK += IICWrite(&_LTC4015IO,LTC4015_ADDR,LTC4015_ICHARGE_TARGET_ADDR,(unsigned short)(BatteryPara.IChargeLimit * LTC4015_R_SNSB - 1));
    /*(VCHARGE_SETTING/80.0 + 3.8125)V/Cell */
    is_OK += IICWrite(&_LTC4015IO,LTC4015_ADDR,LTC4015_VCHARGE_SETTING_ADDR,(unsigned short)((BatteryPara.VChargeLimit - 3.8125f) * 80.0f));
    /*Max_CV_TIME = 1h*/
    is_OK += IICWrite(&_LTC4015IO,LTC4015_ADDR,LTC4015_MAX_CV_TIME_ADDR,LTC4015_MAX_CV_TIME_VALUE);
    /*MAX_CHARGE_TIME = 6h*/
    is_OK += IICWrite(&_LTC4015IO,LTC4015_ADDR,LTC4015_MAX_CHARGE_TIME_ADDR,LTC4015_MAX_CHARGE_TIME_VALUE);

    return is_OK;
}

static unsigned char GetBatteryType()
{
    unsigned char battery_type = 0;
    
    battery_type |= GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_6);
    battery_type = battery_type << 1;
    battery_type |= GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_7);
    
    return battery_type;
}
static float BatteryCharacterB7030(float BatteryOCV)
{
    float soc = 0.0f;
    soc = (0.1581f * BatteryOCV - 3.6f);
    if(soc >= 1.0f)
    {
        soc = 1.0f;
    }
    if(soc <= 0.01f)
    {
        soc = 0.01f;
    }
    return soc;
}
/**********************************************************************************************************************
    Interface functions
***********************************************************************************************************************/
int BatteryPowerInit()
{
    int is_OK = 0;
    unsigned char battery_type = 0;
    
    /* Heating Init*/
    HeatingInit();
    
    /*IO Init*/
    _LTC4015IO.SCL_group = GPIOB;
    _LTC4015IO.SCL_pin   = GPIO_Pin_10;
    _LTC4015IO.SCL_RCC   = RCC_APB2Periph_GPIOB;
    
    _LTC4015IO.SDA_group = GPIOB;
    _LTC4015IO.SDA_pin   = GPIO_Pin_11;
    _LTC4015IO.SDA_RCC   = RCC_APB2Periph_GPIOB;
    
    IICInit(&_LTC4015IO); 
    BatteryTypeIOInit();

    /*Battery Type Init*/
    battery_type = GetBatteryType();
    switch(battery_type)
    {
        case 0: /*3.3Ah*/
            BatteryPara.BatteryCapacity  = 3.3; /*unit Ah*/
            BatteryPara.VChargeLimit     = 4.1; /*unit V/Cell*/
            BatteryPara.IChargeLimit     = 0.376; /*unit A */
            BatteryPara.BatteryCharacter = BatteryCharacterB7030;
            break;

        default: /*default 3.3Ah*/
            BatteryPara.BatteryCapacity  = 3.3; /*unit Ah*/
            BatteryPara.VChargeLimit     = 4.1; /*unit V/Cell*/
            BatteryPara.IChargeLimit     = 0.376; /*unit A */
            BatteryPara.BatteryCharacter = BatteryCharacterB7030;
            break;
    }
    
    /*Charger Init*/
    is_OK = ChargerInit();
    return is_OK;
}

void ChargerSuspend()
{
    IICWrite(&_LTC4015IO,LTC4015_ADDR,LTC4015_CONFIG_BITS_ADDR,LTC4015_CONFIG_BITS_SUSPEND_CHARGER);
}

void ChargerActive()
{
    IICWrite(&_LTC4015IO,LTC4015_ADDR,LTC4015_CONFIG_BITS_ADDR,LTC4015_CONFIG_BITS_ENABLE_CHARGER);
}
void ChargerReset()
{
    IICWrite(&_LTC4015IO,LTC4015_ADDR,LTC4015_CONFIG_BITS_ADDR,LTC4015_CONFIG_BITS_SUSPEND_CHARGER);
    Delayus(20);
    IICWrite(&_LTC4015IO,LTC4015_ADDR,LTC4015_CONFIG_BITS_ADDR,LTC4015_CONFIG_BITS_ENABLE_CHARGER);
}

void HeatingCmd(int isOpen)
{
    if(isOpen == 1)/*Heating*/
    {
        GPIO_SetBits(GPIOE, GPIO_Pin_10);
    }   
    else          /*Not heating*/  
    {
        GPIO_ResetBits(GPIOE, GPIO_Pin_10);
    }
}

int GetBatteryChargerState(BatteryChargeStateDef *charge_state)
{
    int             is_OK = 0;
    unsigned short  state;
    is_OK = IICRead(&_LTC4015IO,LTC4015_ADDR,LTC4015_CHARGER_STATE_ADDR,&state);
    if(is_OK != 0)
    {
        return -1;
    }
    
    if((state & 0x0100) != 0)/*suspend*/
    {
        charge_state->ChargerSuspend = 1;
        charge_state->PreCharge = 0;
        charge_state->CCCV = 0;
        charge_state->Term = 0;
    }
    else
    {
        charge_state->ChargerSuspend = 0;
        /*Precharge status*/
        if((state & 0x0080) != 0)
        {
            charge_state->PreCharge = 1;
        }
        else
        {
            charge_state->PreCharge = 0;
        }
        /*CC-CV status*/
        if((state & 0x0040) != 0)
        {
            charge_state->CCCV = 1;
        }
        else
        {
            charge_state->CCCV = 0;
        }
        /*Term Status*/
        if((state & 0x0018) != 0)
        {
            charge_state->Term = 1;
        }
        else
        {
            charge_state->Term = 0;
        }
    }
    
    /*MaxChargeTimeFault*/
    if((state & 0x0004) != 0)
    {
        charge_state->MaxChargeTimeFault = 1;
    }
    else
    {
        charge_state->MaxChargeTimeFault = 0;
    }
    /*BatShortFault*/
    if((state & 0x0002) != 0)
    {
        charge_state->BatMissingFault = 1;
    }
    else
    {
        charge_state->BatMissingFault = 0;
    }
    /*BatShortFault*/
    if((state & 0x0001) != 0)
    {
        charge_state->BatShortFault = 1;
    }
    else
    {
        charge_state->BatShortFault = 0;
    }
    return 0;
}
int GetBatteryVoltage(float * Vbat)
{
    signed short temp;
    int          is_OK = 0;
    is_OK = IICRead(&_LTC4015IO,LTC4015_ADDR,LTC4015_VBAT_ADDR,(unsigned short *)&temp);
    if(is_OK == 0)
    {
        *Vbat = (float)temp * 7.0f * 0.192264f / 1000.0f;
    }
    return is_OK;
}

int GetBatteryCurrent(float *Ibat)
{
    signed short temp;
    int          is_OK = 0;
    is_OK = IICRead(&_LTC4015IO,LTC4015_ADDR,LTC4015_IBAT_ADDR,(unsigned short *)&temp);
    if(is_OK == 0)
    {
        *Ibat = (float)temp * 1.46487f / LTC4015_R_SNSB / 1000;
    }
    
    return is_OK;
}

int GetBatteryTemp(float *Temp)
{
    float  R_NTC;
    signed short temp;
    int          is_OK = 0;
    is_OK = IICRead(&_LTC4015IO,LTC4015_ADDR,LTC4015_NTC_RATIO_ADDR,(unsigned short *)&temp);
    if(is_OK == 0)
    {
        R_NTC = LTC4015_R_NTCBIAS * temp / (21845.0f - temp);
        
        /*Calculate temperature*/
        if(R_NTC <= 4.8f)
        {
            *Temp = -9.7509f * R_NTC + 90.173f;
        }
        else if(R_NTC > 4.8f && R_NTC <= 10.0f)
        {
            *Temp = -3.7804f * R_NTC + 61.663f;
        }
        else if(R_NTC > 10.0f && R_NTC <= 14.3f)
        {
            *Temp = -2.0663 * R_NTC + 45.304f;
        }
        else if(R_NTC > 14.3f && R_NTC <= 22.4f)
        {
            *Temp = -1.2697f * R_NTC + 33.619f;
        }
        else if(R_NTC > 22.4f && R_NTC <= 28.5f)
        {
            *Temp = -0.884f * R_NTC + 25.169f;
        }
        else
        {
            *Temp = -0.3557f * R_NTC + 7.3795f;
        }
    }
    
    return is_OK;
}


float GetBatterySOC(float Vbat)
{
    return (*BatteryPara.BatteryCharacter)(Vbat);
}
