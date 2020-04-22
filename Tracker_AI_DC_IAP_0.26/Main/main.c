#include "stm32f10x.h"
#include "core_cm3.h"
#include "os.h"
#include "cpu.h"
#include "GlobalDefine.h"
#include "GlobalOs.h"
#include "AngleSensor.h"
#include "MotorControl.h"
#include "LED.h"
#include "FlashStorage.h"
#include "Init.h"
#include "ModbusFrame.h"
#include "ProtocolAnalysis.h"
#include "WorkModeManager.h"
#include "PowerManagement.h"
#include "ADCSample.h"
#include "MotorProtection.h"
#include "RTC.h"
#include "AstronomicalAlgorithms.h"
#include "TMPSensor.h"
#include "BackupRegister.h"

GlobalVariableDef GlobalVariable = {0};
/***************************************************************************************************
                                    Task priorities
***************************************************************************************************/
#define ANGLE_MANAGEMENT_PRIO      5
#define SAMPLING_PRIO              6
#define POWER_MANAGEMENT_PRIO      8
#define MODBUS_OVER_LORA_PRIO      9
#define RTC_TIME_PRIO             11
#define FAULT_DETECT_PRIO         12
#define WORKMODE_CONTROL_PRIO     13
#define MOTOR_ANGLE_CONTROL_PRIO  14
#define PARAMETER_SAVE_PRIO       15
#define ASTRONOMY_PRIO            16
#define LED_IWDG_PRIO             17
/***************************************************************************************************
                                    Task stack
***************************************************************************************************/
#define ANGLE_MANAGEMENT_STK_SIZE    256
#define SAMPLING_STK_SIZE            256
#define POWER_MANAGEMENT_STK_SIZE    256
#define MODBUS_OVER_LORA_STK_SIZE    512
#define RTC_TIME_STK_SIZE            256
#define ASTRONOMY_STK_SIZE           512
#define MOTOR_PROTECTION_STK_SIZE    256
#define WORKMODE_CONTROL_STK_SIZE    256
#define MOTOR_ANGLE_CONTROL_STK_SIZE 256
#define PARAMETER_SAVE_STK_SIZE      256
#define LED_IWDG_STK_SIZE            256

static CPU_STK angle_management_stk[ANGLE_MANAGEMENT_STK_SIZE];
static CPU_STK sampling_stk[SAMPLING_STK_SIZE];
static CPU_STK power_management_stk[POWER_MANAGEMENT_STK_SIZE];
static CPU_STK modbus_over_lora_stk[MODBUS_OVER_LORA_STK_SIZE];
static CPU_STK RTC_time_stk[RTC_TIME_STK_SIZE];
static CPU_STK astronomy_stk[ASTRONOMY_STK_SIZE];
static CPU_STK motor_protection_stk[MOTOR_PROTECTION_STK_SIZE];
static CPU_STK workmode_control_stk[WORKMODE_CONTROL_STK_SIZE];
static CPU_STK motor_angle_control_stk[MOTOR_ANGLE_CONTROL_STK_SIZE];
static CPU_STK parameter_save_stk[PARAMETER_SAVE_STK_SIZE];
static CPU_STK led_IWDG_stk[LED_IWDG_STK_SIZE];
/***************************************************************************************************
                                    Task TCB
***************************************************************************************************/
OS_TCB AngleMangementTCB;
OS_TCB SamplingTCB;
OS_TCB PowerManagementTCB;
OS_TCB ModbusOverLoRaTCB;
OS_TCB RTCTimeTCB;
OS_TCB AstronomyTCB;
OS_TCB MotorProtectionTCB;
OS_TCB WorkmodeControlTCB;
OS_TCB MotorAngleControlTCB;
OS_TCB ParameterSaveTCB;
OS_TCB LedIWDGTCB;
/***************************************************************************************************
                                    OS EVENT FLAG
***************************************************************************************************/
OS_FLAG_GRP	Power_switch_Flags;  //create a event flag
/***************************************************************************************************
                                    Task functions declaration
***************************************************************************************************/
static void AngleMangement(void *p_arg);
static void Sampling(void *p_arg);
static void PowerManagement(void *p_arg);
static void ModbusOverLoRa(void *p_arg);
static void RTCTime(void *p_arg);
static void Astronomy(void *p_arg);
static void MotorProtection(void *p_arg);
static void WorkmodeControl(void *p_arg);
static void MotorAngleControl  (void *p_arg);
static void ParameterSave(void *p_arg);
static void LedIWDG(void *p_arg);
/***************************************************************************************************
                                    Main program
***************************************************************************************************/
int main(void)
{
    OS_ERR err;

    OSInit(&err);
	  OSFlagCreate((OS_FLAG_GRP*)&Power_switch_Flags,		             //指向事件标志组
                 (CPU_CHAR*	  )"Power switch Flags",	             //名字
                 (OS_FLAGS	  )Power_switch_FLAGS_VALUE,           //事件标志组初始值
                 (OS_ERR*  	  )&err);			                         //错误码
    NVICInit();  
    ParaInit();
 //   IWDGInit();
    OSTaskCreate((OS_TCB *    )&ParameterSaveTCB,
                 (CPU_CHAR *  )"ParameterSave",
                 (OS_TASK_PTR )ParameterSave,
                 (void *      )0,
                 (OS_PRIO     )PARAMETER_SAVE_PRIO,
                 (CPU_STK *   )&parameter_save_stk,
                 (CPU_STK_SIZE)PARAMETER_SAVE_STK_SIZE/10,
                 (CPU_STK_SIZE)PARAMETER_SAVE_STK_SIZE,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void *      )0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
                 (OS_ERR *    )&err);
               
    OSTaskCreate((OS_TCB *    )&AngleMangementTCB,
                 (CPU_CHAR *  )"AngleMangement",
                 (OS_TASK_PTR )AngleMangement,
                 (void *      )0,
                 (OS_PRIO     )ANGLE_MANAGEMENT_PRIO,
                 (CPU_STK *   )&angle_management_stk,
                 (CPU_STK_SIZE)ANGLE_MANAGEMENT_STK_SIZE/10,
                 (CPU_STK_SIZE)ANGLE_MANAGEMENT_STK_SIZE,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void *      )0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
                 (OS_ERR *    )&err); 
                 
    OSTaskCreate((OS_TCB *    )&SamplingTCB,
                 (CPU_CHAR *  )"Sampling",
                 (OS_TASK_PTR )Sampling,
                 (void *      )0,
                 (OS_PRIO     )SAMPLING_PRIO,
                 (CPU_STK *   )&sampling_stk,
                 (CPU_STK_SIZE)SAMPLING_STK_SIZE/10,
                 (CPU_STK_SIZE)SAMPLING_STK_SIZE,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void *      )0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
                 (OS_ERR *    )&err);              
           
    OSTaskCreate((OS_TCB *    )&PowerManagementTCB,
                 (CPU_CHAR *  )"PowerManagement",
                 (OS_TASK_PTR )PowerManagement,
                 (void *      )0,
                 (OS_PRIO     )POWER_MANAGEMENT_PRIO,
                 (CPU_STK *   )&power_management_stk,
                 (CPU_STK_SIZE)POWER_MANAGEMENT_STK_SIZE/10,
                 (CPU_STK_SIZE)POWER_MANAGEMENT_STK_SIZE,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void *      )0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
                 (OS_ERR *    )&err); 
     
    OSTaskCreate((OS_TCB *    )&ModbusOverLoRaTCB,
                 (CPU_CHAR *  )"ModbusOverLoRa",
                 (OS_TASK_PTR )ModbusOverLoRa,
                 (void *      )0,
                 (OS_PRIO     )MODBUS_OVER_LORA_PRIO,
                 (CPU_STK *   )&modbus_over_lora_stk,
                 (CPU_STK_SIZE)MODBUS_OVER_LORA_STK_SIZE/10,
                 (CPU_STK_SIZE)MODBUS_OVER_LORA_STK_SIZE,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void *      )0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
                 (OS_ERR *    )&err);
                 
           
    OSTaskCreate((OS_TCB *    )&RTCTimeTCB,
                 (CPU_CHAR *  )"RTCTime",
                 (OS_TASK_PTR )RTCTime,
                 (void *      )0,
                 (OS_PRIO     )RTC_TIME_PRIO,
                 (CPU_STK *   )&RTC_time_stk,
                 (CPU_STK_SIZE)RTC_TIME_STK_SIZE/10,
                 (CPU_STK_SIZE)RTC_TIME_STK_SIZE,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void *      )0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
                 (OS_ERR *    )&err);
 
    OSTaskCreate((OS_TCB *    )&MotorProtectionTCB,
                 (CPU_CHAR *  )"MotorProtection",
                 (OS_TASK_PTR )MotorProtection,
                 (void *      )0,
                 (OS_PRIO     )FAULT_DETECT_PRIO,
                 (CPU_STK *   )&motor_protection_stk,
                 (CPU_STK_SIZE)MOTOR_PROTECTION_STK_SIZE/10,
                 (CPU_STK_SIZE)MOTOR_PROTECTION_STK_SIZE,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void *      )0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
                 (OS_ERR *    )&err);

            
    OSTaskCreate((OS_TCB *    )&WorkmodeControlTCB,
                 (CPU_CHAR *  )"WorkmodeControl",
                 (OS_TASK_PTR )WorkmodeControl,
                 (void *      )0,
                 (OS_PRIO     )WORKMODE_CONTROL_PRIO,
                 (CPU_STK *   )&workmode_control_stk,
                 (CPU_STK_SIZE)WORKMODE_CONTROL_STK_SIZE/10,
                 (CPU_STK_SIZE)WORKMODE_CONTROL_STK_SIZE,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void *      )0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
                 (OS_ERR *    )&err);

    OSTaskCreate((OS_TCB *    )&MotorAngleControlTCB,
                 (CPU_CHAR *  )"MotorAngleControl",
                 (OS_TASK_PTR )MotorAngleControl,
                 (void *      )0,
                 (OS_PRIO     )MOTOR_ANGLE_CONTROL_PRIO,
                 (CPU_STK *   )&motor_angle_control_stk,
                 (CPU_STK_SIZE)MOTOR_ANGLE_CONTROL_STK_SIZE/10,
                 (CPU_STK_SIZE)MOTOR_ANGLE_CONTROL_STK_SIZE,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void *      )0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
                 (OS_ERR *    )&err);
                 
    OSTaskCreate((OS_TCB *    )&AstronomyTCB,
                 (CPU_CHAR *  )"Astronomy",
                 (OS_TASK_PTR )Astronomy,
                 (void *      )0,
                 (OS_PRIO     )ASTRONOMY_PRIO,
                 (CPU_STK *   )&astronomy_stk,
                 (CPU_STK_SIZE)ASTRONOMY_STK_SIZE/10,
                 (CPU_STK_SIZE)ASTRONOMY_STK_SIZE,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void *      )0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
                 (OS_ERR *    )&err);
                 
                 
    OSTaskCreate((OS_TCB *    )&LedIWDGTCB,
                 (CPU_CHAR *  )"LEDIWDG",
                 (OS_TASK_PTR )LedIWDG,
                 (void *      )0,
                 (OS_PRIO     )LED_IWDG_PRIO,
                 (CPU_STK *   )&led_IWDG_stk,
                 (CPU_STK_SIZE)LED_IWDG_STK_SIZE/10,
                 (CPU_STK_SIZE)LED_IWDG_STK_SIZE,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void *      )0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
                 (OS_ERR *    )&err);
                 
    OS_CPU_SysTickInit(720000);
    
    OSStart(&err);
}

/***************************************************************************************************
                                    Task functions
***************************************************************************************************/
static void AngleMangement(void *p_arg)
{
    OS_ERR         err;
    unsigned short i = 0;
    float          current_angle;
    p_arg = p_arg;
    AngleSensorInit();
    
    while(1)
    {
        
        if(GetAngle(0,&current_angle) == 0)
        {
            GlobalVariable.Motor[0].ActualAngle = current_angle + GlobalVariable.ConfigPara.AngleCorrectionValue;
            i = 0;
        }
        else
        {
            if(i < 20)
            {
                i++;
            }
            else
            {
                GlobalVariable.WarningAndFault.T0AngleSensorNoCom = 1;
            }
        }

        OSTimeDly(2,OS_OPT_TIME_DLY,&err);
    }        

}

static void Sampling(void *p_arg)
{
    OS_ERR         err;
    static unsigned char delay_count = 0;
    p_arg = p_arg;
    
    ADCInit();
    TMPSensorInit();
    while(1)
    {
        ADCConvert();
        GlobalVariable.Motor[0].Motorcurrent      = GetMotorCurrent();
        GlobalVariable.PowerPara.PVBuckerVoltage  = GetPVBuckerVoltage();
        GlobalVariable.PowerPara.PVStringUpCur    = GetPVStringUpCur();
        GlobalVariable.PowerPara.PVStringDownCur  = GetPVStringDownCur();
        GlobalVariable.PowerPara.EmergencyVoltage = GetEmergencyVoltage();
        delay_count ++;
        if(delay_count >= 30)
        {
            delay_count = 0;
            GetTMP(&GlobalVariable.Tmp.BoardTmp);
        }
        OSTimeDly(3,OS_OPT_TIME_DLY,&err);
    }
}

static void PowerManagement(void *p_arg)
{
    OS_ERR err;
    p_arg = p_arg;
    PowerInit();
    while(1)
    {
        PowerMangement();
        OSTimeDly(10,OS_OPT_TIME_DLY,&err); 
    }
}
static void ModbusOverLoRa(void *p_arg)
{
    OS_ERR                err;
    static unsigned char  send_buffer[300];
    static unsigned char  read_buffer[300];
    static unsigned short reply_num_of_bytes;
    static unsigned short num_of_bytes;
    static unsigned short delay_count = 0;
    
    ModbusPortInit(0);
    ProtocolInit();
    while(1)
    {
        if(ModbusRead(0,&num_of_bytes,read_buffer) == 0)
        {
            if(read_buffer[0] == GlobalVariable.ConfigPara.ComID || read_buffer[0] == 0)
            {
                delay_count = 0;
							  reply_num_of_bytes = 0;
                reply_num_of_bytes = ProtocolAnalysis(read_buffer,send_buffer,num_of_bytes);
                if((reply_num_of_bytes > 0)&&(read_buffer[0] != 0))
                {
                    ModbusSend(0,reply_num_of_bytes,send_buffer);
                }
            }
						else
						{
							if((delay_count++) >= (5 * 60 * 1000 /10))  //持续5分钟收的没有自己的ID或者广发
							{
								LoRaModulePowerOff();                     //lora模块断电
								OSTimeDly(100,OS_OPT_TIME_DLY,&err);      //延时1S
								LoRaModulePowerOn();                      //lora模块上电
								delay_count = 0;													//重新计时
							}						
						}
        }
				else
				{
				  if((delay_count++) >= (5 * 60 * 1000 /10))  //持续5分钟收的没有数据或校验错误
					{
					  LoRaModulePowerOff();                     //lora模块断电
					  OSTimeDly(100,OS_OPT_TIME_DLY,&err);      //延时1S
						LoRaModulePowerOn();                      //lora模块上电
						delay_count = 0;													//重新计时
					}						
				}
        OSTimeDly(1,OS_OPT_TIME_DLY,&err);
    }
}

static void RTCTime(void *p_arg)
{
    OS_ERR        err;
    unsigned char count = 0;
    p_arg = p_arg;
    BKPInit();
    RTCInit();
    while(1)
    {
        if(GlobalVariable.WriteFlag.TimeWrite == 1)
        {
            GlobalVariable.WriteFlag.TimeWrite = 0;
            
            SetTime((GlobalVariable.TimeBuffer.Second ),
                    (GlobalVariable.TimeBuffer.Minute ),
                    (GlobalVariable.TimeBuffer.Hour ),
                    (GlobalVariable.TimeBuffer.Day ),
                    (GlobalVariable.TimeBuffer.Month ),
                    (GlobalVariable.TimeBuffer.Year ));
        }
        if(count ++ > 3)
        {
            /*Read Time*/
            count = 0;
            GetTime(&GlobalVariable.Time.Second,
                    &GlobalVariable.Time.Minute,
                    &GlobalVariable.Time.Hour,
                    &GlobalVariable.Time.Day,
                    &GlobalVariable.Time.Month,
                    &GlobalVariable.Time.Year);
            GlobalVariable.Time.TodayTime = (float)GlobalVariable.Time.Hour + (float)GlobalVariable.Time.Minute / 60.0f;
            
            /* Check RTC power down flag */
           if(BKPReadRTCUpdateFlag() == 0)
           {
               GlobalVariable.WarningAndFault.TimeLost = 0;
           }
           else
           {
               GlobalVariable.WarningAndFault.TimeLost = 1;
           }
        }
        
        OSTimeDly(5,OS_OPT_TIME_DLY,&err);
    }
}


static void MotorProtection(void *p_arg)
{
    OS_ERR err;
    p_arg = p_arg;
    
    while(1)
    {
        /*motor over current*/
        DetectOverCurrent();
        /*angle soft limit */
        DetectSoftLimit();
        /*MotorReverse */
        DetectMotorReverseAngleNoChange(); 
        /*Emergency stop*/
        DetectEmergencyStop();
        
        OSTimeDly(5,OS_OPT_TIME_DLY,&err);
    }
}
static void WorkmodeControl(void *p_arg)
{
    OS_ERR err;
    p_arg = p_arg;
    
    WorkModeInit();
    while(1)
    {
        if(GlobalVariable.WorkMode.SystemStatus == 0)
        {
            WorkModeMg();
        }
        else
        {
            GlobalVariable.Motor[0].MotorEnable = 0;
        }
        
        OSTimeDly(1,OS_OPT_TIME_DLY,&err);
    }
}
static void MotorAngleControl(void *p_arg)
{
    OS_ERR               err;
    static unsigned char motor_control_state = 0;
    static unsigned char dir_sw_delay = 0;
	  static unsigned short motor_turn_delay = 0;    //电机连续运行时间
    float motor_lead_angle = 0.0f;
    p_arg = p_arg;
    MotorInit();
    
    while(1)
    {
        if(GlobalVariable.Motor[0].MotorEnable != 0
        &&GlobalVariable.WarningAndFault.T0AngleNoChange == 0
        &&GlobalVariable.WarningAndFault.T0AngleSensorNoCom  == 0
        &&GlobalVariable.WarningAndFault.T0OverCurrent == 0
        &&GlobalVariable.WarningAndFault.T0Reversed == 0
        &&GlobalVariable.WarningAndFault.BatError == 0
        &&GlobalVariable.WarningAndFault.EMSOpen == 0)
        {
            /*Determine if need to lesd a certain angle*/
            if(GlobalVariable.Motor[0].NeedLeadAngle == 1)
            {
                motor_lead_angle = GlobalVariable.ConfigPara.EWTracingAccuracy / 2.0f;
            }
            else
            {
                motor_lead_angle = 0.0f;
            }
            /*Motor to target angle*/
            switch(motor_control_state)
            {
                case 0: /* state judgment */
                    if(GlobalVariable.WorkMode.Target - GlobalVariable.Motor[0].ActualAngle > GlobalVariable.ConfigPara.EWTracingAccuracy)
                    {
                        GlobalVariable.Motor[0].MotorControlDir = 2;
											  if(dir_sw_delay ++ > 200)    //电机判断到需要转动到实际转动有2S延时
                        {
                            motor_control_state = 2; /* West */
                            dir_sw_delay = 0;
                        }
                    }
                    else if (GlobalVariable.WorkMode.Target - GlobalVariable.Motor[0].ActualAngle <(-1 * GlobalVariable.ConfigPara.EWTracingAccuracy))
                    {
                        GlobalVariable.Motor[0].MotorControlDir = 1;
                        if(dir_sw_delay ++ >200)     //电机判断到需要转动到实际转动有2S延时
                        {
                            motor_control_state = 1; /* East */
                            dir_sw_delay = 0;
                        } 
                    }
                    else
                    {
                        MotorAllStop();            /* Stop */
                        motor_control_state = 0;
                        dir_sw_delay = 0;
                        GlobalVariable.Motor[0].MotorControlDir = 0;
                    }
                    break;
                case 1:/* East */
                    if(((GlobalVariable.WorkMode.Target - GlobalVariable.Motor[0].ActualAngle) >= motor_lead_angle)
                    ||(GlobalVariable.WarningAndFault.T0ESoftLimit == 1))
                    {
                        MotorAllStop(); 
                        motor_control_state = 0;
											  motor_turn_delay = 0;    //电机连续运行时间清零
                    }
                    else
                    {
												motor_turn_delay ++;     //电机连续运行时间累加
											  MotorTurnEast(0);
                    }
                    break;
                case 2:/* West */
                    if(((GlobalVariable.Motor[0].ActualAngle - GlobalVariable.WorkMode.Target) >= motor_lead_angle)
                    ||(GlobalVariable.WarningAndFault.T0WSoftLimit == 1))
                    {
                        MotorAllStop();
                        motor_control_state = 0;
											  motor_turn_delay = 0;    //电机连续运行时间清零
                    }
                    else
                    {         
												motor_turn_delay ++;     //电机连续运行时间累加
											  MotorTurnWest(0);
                    }
                    break;
                default:
                    MotorAllStop();
                    motor_control_state = 0;
										motor_turn_delay = 0;        //电机连续运行时间清零
            }
        }
        else
        {
            MotorAllStop();
            motor_control_state = 0;
            GlobalVariable.Motor[0].MotorControlDir = 0;
					  motor_turn_delay = 0;                                  //电机连续运行时间清零	
        }
				
				if(motor_turn_delay > 2000)                                //连续转动时间超出20S，换电池驱动
				{
					  motor_turn_delay = 0;                                  //电机连续运行时间清零
					  OSFlagPost((OS_FLAG_GRP*)&Power_switch_Flags,          //post a event switch bat
					             (OS_FLAGS	  )Power_switch_BAT_FLAG,
								       (OS_OPT	    )OS_OPT_POST_FLAG_SET,
					             (OS_ERR*	    )&err);	
				}
				if( (motor_control_state == 0) && (GlobalVariable.Motor[0].MotorControlDir != 0) ) //表明电机在准备转动阶段，2S时间
				{
				    if( ((GlobalVariable.WorkMode.WorkMode & 0xFF) != AUTO_TRACKER_MODE) &&        //不是自动跟踪模式
								((GlobalVariable.WorkMode.WorkMode & 0xFF) != AUTO_AI_MODE) 			)        //不是AI模式
						{
						    OSFlagPost((OS_FLAG_GRP*)&Power_switch_Flags,                              //post a event switch bat
					                 (OS_FLAGS	  )Power_switch_BAT_FLAG,
								           (OS_OPT	    )OS_OPT_POST_FLAG_SET,
					                 (OS_ERR*	    )&err);	               
						}
						else                                                                           //自动跟踪模式和AI模式
						{
						    if(GlobalVariable.Motor[0].MotorControlDir == 2)                           //to west
								{
							      if( GlobalVariable.Motor[0].ActualAngle < 90 )
							      {
										    OSFlagPost((OS_FLAG_GRP*)&Power_switch_Flags,                      //post a event switch bat
														       (OS_FLAGS	  )Power_switch_BAT_FLAG,
																	 (OS_OPT	    )OS_OPT_POST_FLAG_SET,
																	 (OS_ERR*	    )&err);
							      }
							      else
									  {
										    OSFlagPost((OS_FLAG_GRP*)&Power_switch_Flags,                      //post a event switch PV
														       (OS_FLAGS	  )Power_switch_PV_FLAG,
																	 (OS_OPT	    )OS_OPT_POST_FLAG_SET,
																	 (OS_ERR*	    )&err);
									  }
								}
								else if(GlobalVariable.Motor[0].MotorControlDir == 1)                      //to east
								{
									  if( GlobalVariable.Motor[0].ActualAngle > 90 )
							      {
							          OSFlagPost((OS_FLAG_GRP*)&Power_switch_Flags,                      //post a event switch bat
														       (OS_FLAGS	  )Power_switch_BAT_FLAG,
																	 (OS_OPT	    )OS_OPT_POST_FLAG_SET,
																	 (OS_ERR*	    )&err);
							      }
							      else
									  {
										    OSFlagPost((OS_FLAG_GRP*)&Power_switch_Flags,                      //post a event switch PV
														       (OS_FLAGS	  )Power_switch_PV_FLAG,
																	 (OS_OPT	    )OS_OPT_POST_FLAG_SET,
																	 (OS_ERR*	    )&err);
									  }
								}
						}
				}
				else if( (motor_control_state == 0) && (GlobalVariable.Motor[0].MotorControlDir == 0) ) //电机停止转动的时候
				{
				    OSFlagPost((OS_FLAG_GRP*)&Power_switch_Flags,                                  //post a event switch PV
														       (OS_FLAGS	  )Power_switch_PV_FLAG,
																	 (OS_OPT	    )OS_OPT_POST_FLAG_SET,
																	 (OS_ERR*	    )&err);											
				}
        GlobalVariable.Motor[0].MotorRunningState = GetMotorRunningState(0);
        GlobalVariable.Motor[0].MotorActualDir = GetMotorDirState(0);
        OSTimeDly(1,OS_OPT_TIME_DLY,&err);
    }
}


static void ParameterSave(void *p_arg)
{
    OS_ERR err;
    p_arg = p_arg;
    CPU_SR_ALLOC();
    if(ReadConfigParaFromFlash((unsigned short *)&GlobalVariable.ConfigPara.AngleCorrectionValue,sizeof(GlobalVariable.ConfigPara)/2) != 0)
    {
        ParaInit();
    }
    if(ReadWorkModeFromFlash(&GlobalVariable.WorkMode.WorkMode) != 0)
    {
        GlobalVariable.WorkMode.WorkMode = 0;
    }
    GlobalVariable.WorkMode.PreWorkMode = GlobalVariable.WorkMode.WorkMode;
    
    while(1)
    {
        /*Para update*/
        if(GlobalVariable.WriteFlag.ConfigParaWrite == 1)
        {
            GlobalVariable.WriteFlag.ConfigParaWrite = 0;
            WriteConfigParaToFlash((unsigned short *)&GlobalVariable.ConfigPara.AngleCorrectionValue,sizeof(GlobalVariable.ConfigPara)/2);
        }
        if((GlobalVariable.WorkMode.WorkMode != GlobalVariable.WorkMode.PreWorkMode) && (GlobalVariable.WorkMode.WorkMode != 0xFF))
        {
            GlobalVariable.WorkMode.PreWorkMode = GlobalVariable.WorkMode.WorkMode;
            WriteWorkModeToFlash(GlobalVariable.WorkMode.WorkMode);
        }
        
        /*System Update*/
        if(GlobalVariable.WorkMode.SystemStatus == 1)    /*Flash wipe*/
        {
            EraseNewSystemFlash();
            GlobalVariable.WorkMode.SystemStatus = 2;
        }
        else if(GlobalVariable.WorkMode.SystemStatus == 2)
        {
            GlobalVariable.IAPUpdateSys.UpdateTimeOut ++;
            if(GlobalVariable.IAPUpdateSys.UpdateTimeOut >= 30*1000/10) /*TimeOut 30s*/
            {
                GlobalVariable.WorkMode.SystemStatus = 0;
            }
            else
            {
                if(GlobalVariable.WriteFlag.UpdatePackWrite == 1)
                {
                    OS_CRITICAL_ENTER();
                    GlobalVariable.WriteFlag.UpdatePackWrite = 0;
                    WriteNewSystemToFlash((unsigned short *)GlobalVariable.IAPUpdateSys.UpdateBuffer,GlobalVariable.IAPUpdateSys.PackNumber);
                    GlobalVariable.IAPUpdateSys.PackNumber++;
                    OS_CRITICAL_EXIT();
                }
            }
        }
        else if(GlobalVariable.WorkMode.SystemStatus == 3)   /*Update finish*/
        {
            GlobalVariable.WorkMode.SystemStatus = 0;
            WriteIAPFlagToFlash(GlobalVariable.IAPUpdateSys.PackNumber * 256 / 2 + 10);
            NVIC_SystemReset();
        }
        else
        {
            GlobalVariable.WorkMode.SystemStatus = 0;
        }
        
        OSTimeDly(1,OS_OPT_TIME_DLY,&err);
    }
}

static void Astronomy(void *p_arg)
{
    OS_ERR err;
    p_arg = p_arg;
    
    while(1)
    {
        CalcSolarZenithAngleAndAzimuthAngle();
        GlobalVariable.AstronomyPara.AstronomicalTargetAngle = GetAstronomicalTargetAngle();
        OSTimeDly(30,OS_OPT_TIME_DLY,&err);
    }
}
static void LedIWDG(void *p_arg)
{
    OS_ERR           err;
    unsigned char   i = 0;	
    p_arg = p_arg;
  
    LEDInit();
    while(1)
    {
			  i++;
        if(GlobalVariable.WorkMode.SystemStatus == 0)
        {
            if(i > 6)
            {
                i = 0;
                LEDFlash();
            }
        }
        else if(GlobalVariable.WorkMode.SystemStatus == 1)
        {
            LEDTurnOn();
        }
        else if(GlobalVariable.WorkMode.SystemStatus == 2)
        {
            if(i >= 2)
            {
                i = 0;
                LEDFlash();
            }
        }
        else
        {
            LEDTurnOff();
        }
        IWDG_ReloadCounter(); 
        OSTimeDly(10,OS_OPT_TIME_DLY,&err);
    }
}
