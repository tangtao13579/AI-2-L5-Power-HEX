#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "MotorControl.h"
#include "GlobalDefine.h"

#define TIMER_PERIOD        36000/1.1  /*Timer period count 36000 = 1ms 频率改为1.1K*/
#define PWM_START_VOLTAGE   8
#define PWM_STOP_VOLTAGE    27
#define MOTOR_RATED_VOLTAGE 30
#define PWM_RAMP_TIME       2000
#define START_COUNT         (TIMER_PERIOD - (TIMER_PERIOD * PWM_START_VOLTAGE / MOTOR_RATED_VOLTAGE))
#define COUNT_PER_MS        (TIMER_PERIOD * (PWM_STOP_VOLTAGE - PWM_START_VOLTAGE) / (MOTOR_RATED_VOLTAGE * PWM_RAMP_TIME))
#define COUNT_PER_MS1       (TIMER_PERIOD * (12 - 8) / (MOTOR_RATED_VOLTAGE * PWM_RAMP_TIME))
#define COUNT_PER_MS2       (TIMER_PERIOD * (19 - 12) / (MOTOR_RATED_VOLTAGE * PWM_RAMP_TIME))
#define COUNT_PER_MS3       (TIMER_PERIOD * (27 - 19) / (MOTOR_RATED_VOLTAGE * PWM_RAMP_TIME))
extern GlobalVariableDef GlobalVariable;

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
static signed short _pwm_test = 1;
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
		_pwm_test = 0;
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
        if(GlobalVariable.ConfigPara.MotorOrientation==0)  //朝东
			  {
			      GPIO_SetBits(_motor_list[motor_ID].MotorIO.EW_group,_motor_list[motor_ID].MotorIO.EW_pin);
				}
				else                                               //朝西
				{
					  GPIO_ResetBits(_motor_list[motor_ID].MotorIO.EW_group,_motor_list[motor_ID].MotorIO.EW_pin);
				}
    }
    else if(EW == MotorDirWest)
    {
         if(GlobalVariable.ConfigPara.MotorOrientation==0)  //朝东
			   {  
			       GPIO_ResetBits(_motor_list[motor_ID].MotorIO.EW_group,_motor_list[motor_ID].MotorIO.EW_pin);
				 }
				 else
				 {
				     GPIO_SetBits(_motor_list[motor_ID].MotorIO.EW_group,_motor_list[motor_ID].MotorIO.EW_pin);
				 }
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
				if(_pwm_count>(5600*1.1f))  //频率改为1.1K，所以时间计数也要换算，电机缓启动时间定为5.6S，这样会比较平缓
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
				
        if(_pwm_count<(800*1.1f))  //这里初步定为三个上升斜率      
				{
					  _pwm_test = _pwm_count *COUNT_PER_MS1 * 2000.0f / (800.0f*1.1f);
				}
        else if(_pwm_count<(2400*1.1f))
				{
					  _pwm_test = _pwm_count * COUNT_PER_MS2 * 2000.0f / (1600.0f*1.1f);
				}
				else if(_pwm_count<(4000*1.1f))
				{
					  _pwm_test = _pwm_count * COUNT_PER_MS3 * 2000.0f / (1600.0f*1.1f);
				}
				else if(_pwm_count<(5600*1.1f))  //继续维持一段时间，试下来能有效减小最后一下的冲击电流
				{
					  _pwm_test = (4000*1.1f) * COUNT_PER_MS3 * 2000.0f / (1600.0f*1.1f);
				    if(GlobalVariable.Motor[0].Motorcurrent > 1.2f || GlobalVariable.Motor[0].Motorcurrent < -1.2f)
				    {
					      _pwm_test = (4000-2)*1.1f * COUNT_PER_MS3 * 2000.0f / (1600.0f*1.1f);
				    }
				}						
				TIM_SetCompare1(TIM3,START_COUNT - _pwm_test);//(_pwm_count * COUNT_PER_MS));          
    }
    if(TIM_GetITStatus(TIM3,TIM_IT_CC1) != RESET)
    {
        TIM_ClearITPendingBit(TIM3,TIM_IT_CC1);
        MotorEN(_current_run_motor_ID, MotorEnable);
    }
}


/***********************PID bak************************
static signed short _pwm_count1 = 0;
static signed short _pwm_error = 0;
static signed short _pwm_errord = 0;
static signed short _pwm_errordd = 0;
static signed short _pwm_errorpre = 0;
static signed short _pwm_errordpre = 0;

//				if(_pwm_count%60 == 0)_pwm_count1 ++;
//				if(_pwm_count%60 == 0)
//				{
//					if((GlobalVariable.Motor[0].Motorcurrent*1000.0f)>0.0f)
//						{
//							  _pwm_error = (GlobalVariable.Motor[0].Motorcurrent*1000.0f) - 1000.0f;
//						}
//						else
//						{
//                _pwm_error = (GlobalVariable.Motor[0].Motorcurrent*(-1000.0f)) - 1000.0f;
//						}							
//					  _pwm_errord = _pwm_error - _pwm_errorpre;
//					  _pwm_errordd = _pwm_errord - _pwm_errordpre;
//					  _pwm_errorpre = _pwm_error;
//					  _pwm_errordpre = _pwm_errord;
//					  _pwm_test += (0.700f*_pwm_errord) + (5.400f*_pwm_error) + (0.000f*_pwm_errordd);

//第二种					_pwm_error = 1000.0f - (GlobalVariable.Motor[0].Motorcurrent*1000.0f);
//					  _pwm_errord += _pwm_error;
//					  _pwm_errordd = _pwm_errorpre - _pwm_errordpre;
//					  _pwm_errordpre = _pwm_errorpre;
//					  _pwm_errorpre = _pwm_error;
//					  _pwm_test = (10.000f*_pwm_error) + (6.000f*_pwm_errord) + (0.000f*_pwm_errordd);
//          if( GlobalVariable.Motor[0].Motorcurrent>1.0f || GlobalVariable.Motor[0].Motorcurrent < -1.0f)
//					{
//						_pwm_error = 1000.0f - (GlobalVariable.Motor[0].Motorcurrent*1000.0f);
//			      _pwm_test = (1.000f*(_pwm_error-_pwm_errorpre)) + (0.000f*_pwm_error) + (0.000f*(_pwm_error-(2*_pwm_errorpre)+_pwm_errordpre));
//						_pwm_errordpre = _pwm_errorpre;
//					  _pwm_errorpre = _pwm_error;
//						_pwm_errord -= _pwm_test;
//					}
//					if(_pwm_count1<2000)testtest[_pwm_count1] = GlobalVariable.Motor[0].Motorcurrent;
//					if(_pwm_count1<2000)testtest1[_pwm_count1] = _pwm_errord;
//				}
		
//				_pwm_test1 = _pwm_test1 + _pwm_test;




******************************************************/
