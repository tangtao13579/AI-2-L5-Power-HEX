#include "GlobalDefine.h"
#include "GlobalOs.h"

#define TASK_MOTOR_PROTECTION_PERIOD  50

extern GlobalVariableDef GlobalVariable;

void DetectOverCurrent()
{
    static unsigned char  motor_current_normal_count             = 0;
    static unsigned char  motor_over_current_1x_count            = 0;
    static unsigned char  motor_over_current_restart_count       = 0;
    static unsigned short motor_over_current_restart_delay_count = 0;
    
    if(GlobalVariable.Motor[0].MotorRunningState > 0) /*Motor is not stop*/
    {
        if((GlobalVariable.Motor[0].Motorcurrent >= GlobalVariable.ConfigPara.MotorOverCurrentValue) 
        || (-1 * GlobalVariable.Motor[0].Motorcurrent) >=  GlobalVariable.ConfigPara.MotorOverCurrentValue )
        {
            motor_current_normal_count = 0;
            motor_over_current_1x_count ++ ;
            if(motor_over_current_1x_count > 0.70 * 1000 / TASK_MOTOR_PROTECTION_PERIOD) /* 0.7s */
            {
                motor_over_current_1x_count = 0;
                GlobalVariable.WarningAndFault.T0OverCurrent = 1;
            }
        }
        
        if((GlobalVariable.Motor[0].Motorcurrent <= (GlobalVariable.ConfigPara.MotorOverCurrentValue * 0.9f)) 
        && (GlobalVariable.Motor[0].Motorcurrent >= (GlobalVariable.ConfigPara.MotorOverCurrentValue * -0.9f)))
        {
            motor_current_normal_count ++ ;
            if(motor_current_normal_count > 5 * 1000 / TASK_MOTOR_PROTECTION_PERIOD)/* 5s */
            {
                motor_current_normal_count = 0;
                motor_over_current_1x_count = 0;
                GlobalVariable.WarningAndFault.T0OverCurrent = 0;
                motor_over_current_restart_count = 0;
            }
        }
    }
    else
    {
        motor_current_normal_count = 0;
        motor_over_current_1x_count = 0;
    }
    
    if(GlobalVariable.WarningAndFault.T0OverCurrent == 1)
    {
        motor_over_current_restart_delay_count ++ ;
        if(motor_over_current_restart_delay_count > 60 * 1000 / TASK_MOTOR_PROTECTION_PERIOD) /*1min*/
        {
            motor_over_current_restart_delay_count = 0 ;
            motor_over_current_restart_count ++;
            if(motor_over_current_restart_count < 8)
            {
                GlobalVariable.WarningAndFault.T0OverCurrent = 0;
            }
            else
            {
                motor_over_current_restart_count = 9;
            }
        }
    }
    else
    {
        motor_over_current_restart_delay_count = 0 ;
    }
}

void DetectSoftLimit()
{
    static unsigned char  soft_limit_count = 0;
    if(GlobalVariable.Motor[0].ActualAngle > GlobalVariable.ConfigPara.WSoftLimitAngle + 2)
    {
        soft_limit_count ++;
        if(soft_limit_count > 50)
        {
            soft_limit_count = 52;
            GlobalVariable.WarningAndFault.T0WSoftLimit = 1;
        }
    }
    else if(GlobalVariable.Motor[0].ActualAngle < GlobalVariable.ConfigPara.ESoftLimitAngle - 2)
    {
        soft_limit_count ++;
        if(soft_limit_count > 50)
        {
            soft_limit_count = 52;
            GlobalVariable.WarningAndFault.T0ESoftLimit = 1;
        }
    }
    else if((GlobalVariable.Motor[0].ActualAngle < (GlobalVariable.ConfigPara.WSoftLimitAngle-1))
          &&(GlobalVariable.Motor[0].ActualAngle > (GlobalVariable.ConfigPara.ESoftLimitAngle+1)))
    {
        soft_limit_count = 0;
        GlobalVariable.WarningAndFault.T0ESoftLimit = 0;
        GlobalVariable.WarningAndFault.T0WSoftLimit = 0;
    }
}

void DetectMotorReverseAngleNoChange()
{
	  OS_ERR err;
	  static float pre_angle = 0.0f;
    static unsigned short east_delay_count = 0;
    static unsigned short west_delay_count = 0;
    static unsigned short T0AngleNoChange_first_flag = 0;
	
	  if(GlobalVariable.Motor[0].MotorActualDir == 1)/*east*/
    {
        west_delay_count = 0;
        east_delay_count ++;
        if(east_delay_count < 5)
        {
            pre_angle = GlobalVariable.Motor[0].ActualAngle;
        }
        if(east_delay_count > 600)
        {
            east_delay_count = 0;
            if((pre_angle - GlobalVariable.Motor[0].ActualAngle) < -0.5f)
            {
                GlobalVariable.WarningAndFault.T0Reversed = 1;
            }
            else
            {
                GlobalVariable.WarningAndFault.T0Reversed = 0;
            }
            if(((pre_angle - GlobalVariable.Motor[0].ActualAngle) > 1.0f) || ((pre_angle - GlobalVariable.Motor[0].ActualAngle) < -1.0f))
            {
                GlobalVariable.WarningAndFault.T0AngleNoChange = 0;      //clear the fault
								T0AngleNoChange_first_flag = 0;                          //clear the first happen flag
            }
            else
            {
                GlobalVariable.WarningAndFault.T0AngleNoChange = 1;      //角度不变故障-即电机不转
								if(T0AngleNoChange_first_flag == 1)
								{
									  OSFlagPend((OS_FLAG_GRP*)&T0AngleNoChange_first_Flags,
				                       (OS_FLAGS	)T0AngleNoChange_first_step2_FLAG,
		     	                         (OS_TICK     )0,
				                           (OS_OPT	    )OS_OPT_PEND_FLAG_SET_ALL+OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_NON_BLOCKING,
				                           (CPU_TS*     )0,
				                           (OS_ERR*	    )&err);
		                if(err == OS_ERR_NONE)
		                {
			                  OSFlagPost((OS_FLAG_GRP*)&T0AngleNoChange_first_Flags,      //post a event flag step3
								                   (OS_FLAGS	  )T0AngleNoChange_first_step3_FLAG,
								                   (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					                         (OS_ERR*	  )&err);
		                } 
								}
							  else if(T0AngleNoChange_first_flag == 0 && GlobalVariable.WarningAndFault.BatError == 0)                      //if first time happen
								{
								    T0AngleNoChange_first_flag = 1;                             //happened
                    GlobalVariable.WarningAndFault.T0AngleNoChange = 0;         //clear the fault
									  OSFlagPost((OS_FLAG_GRP*)&T0AngleNoChange_first_Flags,      //post a event flag step1
								               (OS_FLAGS	  )T0AngleNoChange_first_step1_FLAG,
								               (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					                     (OS_ERR*	  )&err);
								}
            }
        }
    }
    else if(GlobalVariable.Motor[0].MotorActualDir == 2)/*west*/
    {
        east_delay_count = 0;
        west_delay_count ++;
        if(west_delay_count < 5)
        {
            pre_angle = GlobalVariable.Motor[0].ActualAngle;
        }
        if(west_delay_count > 600)
        {
            west_delay_count = 0;
            if((pre_angle - GlobalVariable.Motor[0].ActualAngle) > 0.5f)
            {
                GlobalVariable.WarningAndFault.T0Reversed = 1;
            }
            else
            {
                GlobalVariable.WarningAndFault.T0Reversed = 0;
            }
            if(((pre_angle - GlobalVariable.Motor[0].ActualAngle) > 1.0f) || ((pre_angle - GlobalVariable.Motor[0].ActualAngle) < -1.0f))
            {
                GlobalVariable.WarningAndFault.T0AngleNoChange = 0;      //clear the fault
							  T0AngleNoChange_first_flag = 0;                          //clear the first happen flag
            }
            else
            {
                GlobalVariable.WarningAndFault.T0AngleNoChange = 1;      //角度不变故障-即电机不转
							  if(T0AngleNoChange_first_flag == 1)
								{
									  OSFlagPend((OS_FLAG_GRP*)&T0AngleNoChange_first_Flags,
				                       (OS_FLAGS	)T0AngleNoChange_first_step2_FLAG,
		     	                         (OS_TICK     )0,
				                           (OS_OPT	    )OS_OPT_PEND_FLAG_SET_ALL+OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_NON_BLOCKING,
				                           (CPU_TS*     )0,
				                           (OS_ERR*	    )&err);
		                if(err == OS_ERR_NONE)
		                {
			                  OSFlagPost((OS_FLAG_GRP*)&T0AngleNoChange_first_Flags,      //post a event flag step3
								                   (OS_FLAGS	  )T0AngleNoChange_first_step3_FLAG,
								                   (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					                         (OS_ERR*	  )&err);
		                } 
								}
							  else if(T0AngleNoChange_first_flag == 0 && GlobalVariable.WarningAndFault.BatError == 0)                      //if first time happen
								{
								    T0AngleNoChange_first_flag = 1;                             //happened
                    GlobalVariable.WarningAndFault.T0AngleNoChange = 0;         //clear the fault
									  OSFlagPost((OS_FLAG_GRP*)&T0AngleNoChange_first_Flags,      //post a event flag step1
								               (OS_FLAGS	  )T0AngleNoChange_first_step1_FLAG,
								               (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					                     (OS_ERR*	  )&err);
								}
            }
        }
    }
    else/*stop*/
    {
        east_delay_count = 0;
        west_delay_count = 0;
    }
}

void DetectEmergencyStop()
{
    static unsigned char delay_count = 0;
    if(GlobalVariable.PowerPara.EmergencyVoltage <= 15.0f)
    {
        delay_count ++;
        if(delay_count >= 1 * 1000 / TASK_MOTOR_PROTECTION_PERIOD)
        {
            delay_count = 0;
            GlobalVariable.WarningAndFault.EMSOpen = 1;
        }
    }
    else
    {
        delay_count = 0;
        GlobalVariable.WarningAndFault.EMSOpen = 0;
    }
}

