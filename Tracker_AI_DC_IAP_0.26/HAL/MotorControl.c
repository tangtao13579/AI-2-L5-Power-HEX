#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "MotorControl.h"

#define TIMER_PERIOD        36000  /*Timer period count 36000 = 1ms*/
#define PWM_START_VOLTAGE   8
#define PWM_STOP_VOLTAGE    27
#define MOTOR_RATED_VOLTAGE 30
#define PWM_RAMP_TIME       150
#define START_COUNT         (TIMER_PERIOD - (TIMER_PERIOD * PWM_START_VOLTAGE / MOTOR_RATED_VOLTAGE))
#define COUNT_PER_MS        (TIMER_PERIOD * (PWM_STOP_VOLTAGE - PWM_START_VOLTAGE) / (MOTOR_RATED_VOLTAGE * PWM_RAMP_TIME))


/***************************************************************************************************
                                Private variable declaration
***************************************************************************************************/
typedef struct
{
    uint32_t      EN_RCC;
    GPIO_TypeDef *EN_group;
    uint16_t      EN_pin;
    
    uint32_t      EW_RCC;
    GPIO_TypeDef *EW_group;
    uint16_t      EW_pin;
    
} MotorIODef;

typedef enum
{
    MotorDirStop = 0,
    MotorDirEast,
    MotorDirWest,
} MotorDirDef;

typedef enum
{
    MotorEnable = 1,
    MotorDisable
} MotorENDef;

typedef struct
{
    MotorIODef     MotorIO;
    MotorDirDef    MotorDir;
    MotorENDef     MororEN;
    unsigned char  MotorRunningState;/* 0-stop,1-pwm,2-run*/
}MotorListDef;

/***************************************************************************************************
                                Private variable definition
***************************************************************************************************/
static MotorListDef  _motor_list[MAX_MOTOR_NUM];
static unsigned char _current_run_motor_ID;
static unsigned short _pwm_count;

/***************************************************************************************************
                                Private functions
***************************************************************************************************/

static void TIM3Init()
{
    TIM_TimeBaseInitTypeDef TIMInitStruc;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    TIMInitStruc.TIM_Period        = TIMER_PERIOD;
    TIMInitStruc.TIM_Prescaler     = 1;
    TIMInitStruc.TIM_CounterMode   = TIM_CounterMode_Up;
    TIMInitStruc.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3,&TIMInitStruc);
    
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_SetAutoreload(TIM3, TIMER_PERIOD);
    
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_ClearITPendingBit(TIM3,TIM_IT_CC1);
    TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);
    
    TIM_Cmd(TIM3, DISABLE);
}

static void TimerStart()
{
    _pwm_count = 0;/*clear count*/
    /* Set the initial value of the comparator */
    TIM_SetCompare1(TIM3,START_COUNT);
    TIM_Cmd(TIM3, ENABLE);
}

static void TimerStop()
{
    _pwm_count = 0;
    TIM_Cmd(TIM3, DISABLE);
}

static void MotorIOInit()
{
    GPIO_InitTypeDef GPIOInitStruc;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
    
    /* Moror0 enable IO & EW IO */
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_15|GPIO_Pin_14;
    GPIOInitStruc.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIOInitStruc.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIOInitStruc);
    GPIO_ResetBits(GPIOD,GPIO_Pin_15);
    
    
    _motor_list[0].MotorIO.EN_group = GPIOD;
    _motor_list[0].MotorIO.EN_pin   = GPIO_Pin_15;
    _motor_list[0].MotorIO.EN_RCC   = RCC_APB2Periph_GPIOD;
    
    
    _motor_list[0].MotorIO.EW_group = GPIOD;
    _motor_list[0].MotorIO.EW_pin   = GPIO_Pin_14;
    _motor_list[0].MotorIO.EW_RCC   = RCC_APB2Periph_GPIOD;
    
    _motor_list[0].MotorDir = MotorDirStop;
    _motor_list[0].MotorRunningState = 0;
    /*
    Other Motor IO init here
    */
    
}
static void MotorEN(unsigned char motor_ID, MotorENDef EN)
{
    if(EN == MotorEnable)
    {
        GPIO_SetBits(_motor_list[motor_ID].MotorIO.EN_group,_motor_list[motor_ID].MotorIO.EN_pin);
    }
    else
    {
        GPIO_ResetBits(_motor_list[motor_ID].MotorIO.EN_group,_motor_list[motor_ID].MotorIO.EN_pin);
    }
}
static void MotorEW(unsigned char motor_ID, MotorDirDef EW)
{
    if(EW == MotorDirEast)
    {
        GPIO_SetBits(_motor_list[motor_ID].MotorIO.EW_group,_motor_list[motor_ID].MotorIO.EW_pin);
    }
    else if(EW == MotorDirWest)
    {
        GPIO_ResetBits(_motor_list[motor_ID].MotorIO.EW_group,_motor_list[motor_ID].MotorIO.EW_pin);
    }
}

/***************************************************************************************************
                                Public functions
***************************************************************************************************/
void MotorInit()
{    
    MotorIOInit();
    TIM3Init();
}
    
void MotorTurnEast(unsigned char motor_ID)
{
    if(motor_ID <= MAX_MOTOR_NUM)
    {
         /* Indicates that the running motor is not the motor to be set.*/
        if((_current_run_motor_ID != motor_ID)||(_motor_list[motor_ID].MotorDir != MotorDirEast)) 
        {
            /* Stop all motor and timer */
            MotorAllStop();
            /* Set the current motor to run */
            _motor_list[motor_ID].MotorDir = MotorDirEast;
            _current_run_motor_ID = motor_ID;
            /* Set current motor turn to east */
            MotorEW(_current_run_motor_ID,MotorDirEast);
            /* Start timer*/
            TimerStart();
        }
    }
}

void MotorTurnWest(unsigned char motor_ID)
{
    if(motor_ID <= MAX_MOTOR_NUM)
    {
         /* Indicates that the running motor is not the motor to be set.*/
        if((_current_run_motor_ID != motor_ID)||(_motor_list[motor_ID].MotorDir != MotorDirWest)) 
        {
            /* Stop all motor and timer */
            MotorAllStop();
            
            /* Set the current motor to run */
            _motor_list[motor_ID].MotorDir = MotorDirWest;
            _current_run_motor_ID = motor_ID;
            
            /* Set current motor turn to west */
            MotorEW(_current_run_motor_ID,MotorDirWest);
            /* Start timer*/
            TimerStart();
        }
    }
}
void MotorStop(unsigned char motor_ID)
{
    if(motor_ID <= MAX_MOTOR_NUM)
    {
        /* Stop timer */
        TimerStop();
        /* Stop motor */
        MotorEN(motor_ID,MotorDisable);
        /* Indicates that no motor is running */
        _motor_list[motor_ID].MotorDir = MotorDirStop;
        _motor_list[motor_ID].MotorRunningState = 0;
    }
}

/*
    Since only one motor is running, 
    stopping all motors has the same effect as stopping the current motor.
*/
void MotorAllStop()
{
    unsigned char i;
    /* Stop timer */
    TimerStop();
    /* Stop all motor */
    for(i = 0; i < MAX_MOTOR_NUM; i++)
    {
        MotorEN(i,MotorDisable);
        _motor_list[i].MotorDir = MotorDirStop;
        _motor_list[i].MotorRunningState = 0;
    }
}

unsigned char GetMotorRunningState(unsigned char motor_ID)
{
    return _motor_list[motor_ID].MotorRunningState;
}
unsigned char GetMotorDirState(unsigned char motor_ID)
{
    return _motor_list[motor_ID].MotorDir;
}
void TIM3_IRQHandler()
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update); 
        _pwm_count ++;
        if (_pwm_count >= PWM_RAMP_TIME)
        {
            TimerStop();
            MotorEN(_current_run_motor_ID, MotorEnable);
            _motor_list[_current_run_motor_ID].MotorRunningState = 2;
        }
        else
        {
            MotorEN(_current_run_motor_ID, MotorDisable);
            _motor_list[_current_run_motor_ID].MotorRunningState = 1;
        }

        TIM_SetCompare1(TIM3,START_COUNT - _pwm_count * COUNT_PER_MS);
    }
    if(TIM_GetITStatus(TIM3,TIM_IT_CC1) != RESET)
    {
        TIM_ClearITPendingBit(TIM3,TIM_IT_CC1);
        MotorEN(_current_run_motor_ID, MotorEnable);
    }
}

