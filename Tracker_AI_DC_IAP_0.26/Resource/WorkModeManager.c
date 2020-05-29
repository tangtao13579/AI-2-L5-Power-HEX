#include "GlobalDefine.h"

#define TASK_PERIOD 10  /*ms*/

extern GlobalVariableDef GlobalVariable;

static void (*WorkModeFunction)(void);
static void (*AutoModeFunction)(void);


/**********************************************************************************************************************
                                        Private Function
***********************************************************************************************************************/

static void WindCheck(void);
static void BatSOCLowCheck(void);

static void IdelMode(void);

static void ManualMode(void);

static void MaintenanceMode(void);
static void AngleCalibrationMode(void);
static void CleanMode(void);

static void AutoWindMode(void);
static void AutoBatSOCLowMode(void);
static void AutoAIMode(void);
static void AutoRainMode(void);
static void AutoSnowMode(void);
static void AutoTrackMode(void);
static void AutoModeSwitch(void);
static void AutoMode(void);


static void WindCheck()
{
    static unsigned int big_wind_count = 0;
    static unsigned int breeze_wind_count = 0;
    
    if((GlobalVariable.WorkMode.WorkMode & 0xFF) == AUTO_WIND_MODE)/*In Wind Mode*/
    {
        if(GlobalVariable.Weather.RemoteWindSpeed <= GlobalVariable.ConfigPara.LowWindSpeed)
        {
            big_wind_count = 0;
            breeze_wind_count++;
            if(breeze_wind_count >= 60000)
            {
                breeze_wind_count = 60000;
            }
        }
        else
        {
            breeze_wind_count = 0;
        }
        if(breeze_wind_count /1000 * TASK_PERIOD >= GlobalVariable.ConfigPara.LowStopTime)
        {
            breeze_wind_count = 0;
            GlobalVariable.WorkMode.WorkMode = 0x00000000 | AUTO_MODE;
        }
    }
    else/*It's not wind mode*/
    {
        if(GlobalVariable.Weather.RemoteWindSpeed >= GlobalVariable.ConfigPara.UpWindSpeed)
        {
            breeze_wind_count = 0;
            big_wind_count ++;
            if(big_wind_count >= 60000)
            {
                big_wind_count = 60000;
            }
        }
        else
        {
            big_wind_count = 0;
        }
        if(big_wind_count / 1000 * TASK_PERIOD  >= GlobalVariable.ConfigPara.UpStartTime)
        {
            big_wind_count = 0;
            GlobalVariable.WorkMode.WorkMode = 0x00000000 | AUTO_WIND_MODE;
        }
    }
}
static void BatSOCLowCheck()
{
    if((GlobalVariable.WorkMode.WorkMode & 0xFF) != AUTO_WIND_MODE)
    {
        if((GlobalVariable.WorkMode.WorkMode & 0xFF) == AUTO_BATSOCLOW_MODE) /*BATSOCLOW */
        {
            if(GlobalVariable.WarningAndFault.BatSOCLow == 0)
            {
                GlobalVariable.WorkMode.WorkMode = 0x00000000 | AUTO_MODE;
            }
        }
        else
        {
            if(GlobalVariable.WarningAndFault.BatSOCLow != 0)
            {
                GlobalVariable.WorkMode.WorkMode = 0x00000000 | AUTO_BATSOCLOW_MODE;
            }
        }
    }
    
}


static void IdelMode()
{
    GlobalVariable.Motor[0].MotorEnable = 0;
    if(GlobalVariable.WorkMode.WorkMode == 0xFF)
    {
        GlobalVariable.WorkMode.WorkMode = 0x00;
    }
    else if((GlobalVariable.WorkMode.WorkMode & 0xF0) == AUTO_MODE)
    {
        WorkModeFunction = AutoMode;
    }
    else if((GlobalVariable.WorkMode.WorkMode & 0xF0) == MANUAL_MODE)
    {
        WorkModeFunction = ManualMode;
    }
    else if((GlobalVariable.WorkMode.WorkMode & 0xF0) == MAINTENANCE_MODE)
    {
        WorkModeFunction = MaintenanceMode;
    }
    else if((GlobalVariable.WorkMode.WorkMode & 0xF0) == ANGLE_CALIBRATION_MODE)
    {
        WorkModeFunction = AngleCalibrationMode;
    }
		else if((GlobalVariable.WorkMode.WorkMode & 0xF0) == CLEAN_MODE)							//清洗模式
		{
			  WorkModeFunction = CleanMode;
		}
    else
    {
        GlobalVariable.WorkMode.WorkMode = 0x00;
        GlobalVariable.WorkMode.Target = GlobalVariable.AstronomyPara.AstronomicalTargetAngle;
    }
}
static void ManualMode()
{
    if((GlobalVariable.WorkMode.WorkMode & 0xF0) != MANUAL_MODE)
    {
        WorkModeFunction = IdelMode;
        return;
    }
    GlobalVariable.Motor[0].NeedLeadAngle = 0;
    if((GlobalVariable.WorkMode.WorkMode & 0xFF) == MANUAL_EAST_MODE)
    {
        GlobalVariable.Motor[0].MotorEnable = 1;
        GlobalVariable.WorkMode.Target = GlobalVariable.ConfigPara.ESoftLimitAngle+2;
    }
    else if((GlobalVariable.WorkMode.WorkMode & 0xFF) == MANUAL_WEST_MODE)
    {
        GlobalVariable.Motor[0].MotorEnable = 1;
        GlobalVariable.WorkMode.Target = GlobalVariable.ConfigPara.WSoftLimitAngle-2;
    }
    else
    {
        GlobalVariable.Motor[0].MotorEnable = 0;
        GlobalVariable.WorkMode.Target = GlobalVariable.AstronomyPara.AstronomicalTargetAngle;
    }
}

static void MaintenanceMode()	//	维护模式-放平
{
    if(((GlobalVariable.WorkMode.WorkMode & 0xF0) != MAINTENANCE_MODE)||
			 ((GlobalVariable.WorkMode.WorkMode & 0x0F) != 0))
    {
        WorkModeFunction = IdelMode;
        return;
    }
    GlobalVariable.Motor[0].NeedLeadAngle = 0;
    GlobalVariable.WorkMode.Target = 90.0;
    GlobalVariable.Motor[0].MotorEnable = 1;
}

static void CleanMode()	//清洗模式
{
    if(((GlobalVariable.WorkMode.WorkMode & 0xF0) != CLEAN_MODE)||
			 ((GlobalVariable.WorkMode.WorkMode & 0x0F) != 0))
    {
        WorkModeFunction = IdelMode;
        return;
    }
    GlobalVariable.Motor[0].NeedLeadAngle = 0;
    if(GlobalVariable.ConfigPara.CleaningDockAngle < (GlobalVariable.ConfigPara.ESoftLimitAngle +1))
    {
        GlobalVariable.WorkMode.Target = (GlobalVariable.ConfigPara.ESoftLimitAngle +1);
    }
    else if(GlobalVariable.ConfigPara.CleaningDockAngle > (GlobalVariable.ConfigPara.WSoftLimitAngle -1))
    {
        GlobalVariable.WorkMode.Target = (GlobalVariable.ConfigPara.WSoftLimitAngle -1);
    }
    else
    {
        GlobalVariable.WorkMode.Target = GlobalVariable.ConfigPara.CleaningDockAngle;
    }
    GlobalVariable.Motor[0].MotorEnable = 1;
}

static void AngleCalibrationMode()
{
    static unsigned short delay_count = 0;
    float temp_correction_value = 0.0f;
    
    if(((GlobalVariable.WorkMode.WorkMode & 0xF0) != ANGLE_CALIBRATION_MODE)
     ||((GlobalVariable.WorkMode.WorkMode & 0x0F) != 0))
    {
        delay_count = 0;
        WorkModeFunction = IdelMode;
        return;
    }
    GlobalVariable.Motor[0].NeedLeadAngle = 0;
    GlobalVariable.Motor[0].MotorEnable = 0;
    delay_count ++;
    if(delay_count < (3 * 1000 / TASK_PERIOD))/*3s*/
    {
      return;   /*delay until stable*/
    }
    delay_count = 0;
    if(GlobalVariable.WarningAndFault.T0AngleNoChange == 0
     &&GlobalVariable.WarningAndFault.T0AngleSensorNoCom == 0)
    {
        temp_correction_value = 90 - GlobalVariable.Motor[0].ActualAngle+ GlobalVariable.ConfigPara.AngleCorrectionValue;
        if(temp_correction_value < 10 && temp_correction_value > -10)
        {
            GlobalVariable.ConfigPara.AngleCorrectionValue = temp_correction_value;
        }
        else
        {
            GlobalVariable.ConfigPara.AngleCorrectionValue = 0;
        }
        GlobalVariable.WriteFlag.ConfigParaWrite = 1;
    }
    GlobalVariable.WorkMode.WorkMode = 0;
}
static void AutoWindMode()
{
    if((GlobalVariable.WorkMode.WorkMode & 0xFF) != AUTO_WIND_MODE)
    {
        AutoModeFunction = AutoModeSwitch;
        return;
    }
    GlobalVariable.Motor[0].NeedLeadAngle = 0;
    GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.BackAngle;
    GlobalVariable.Motor[0].MotorEnable = 1;
}
static void AutoBatSOCLowMode()
{
    if((GlobalVariable.WorkMode.WorkMode & 0xFF) != AUTO_BATSOCLOW_MODE)
    {
        AutoModeFunction = AutoModeSwitch;
        return;
    }
    GlobalVariable.Motor[0].NeedLeadAngle = 0;
    GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.BackAngle;
    GlobalVariable.Motor[0].MotorEnable = 1;
}
static void AutoAIMode()
{
    if((GlobalVariable.WorkMode.WorkMode & 0xFF) != AUTO_AI_MODE)
    {
        AutoModeFunction = AutoModeSwitch;
        return;
    }
    GlobalVariable.Motor[0].NeedLeadAngle = 0;
    if(GlobalVariable.AIPara.AIRemoteAngle < (GlobalVariable.ConfigPara.ESoftLimitAngle +1))
    {
        GlobalVariable.WorkMode.Target = (GlobalVariable.ConfigPara.ESoftLimitAngle +1);
    }
    else if(GlobalVariable.AIPara.AIRemoteAngle > (GlobalVariable.ConfigPara.WSoftLimitAngle -1))
    {
        GlobalVariable.WorkMode.Target = (GlobalVariable.ConfigPara.WSoftLimitAngle -1);
    }
    else
    {
        GlobalVariable.WorkMode.Target = GlobalVariable.AIPara.AIRemoteAngle;
    }
//    if(GlobalVariable.AIPara.AIModeDelayCount ++ > (150 * 60* 1000 / TASK_PERIOD))/*150min*/
//    {
//        GlobalVariable.AIPara.AIModeDelayCount = 20 * 60* 1000 / TASK_PERIOD;
//        GlobalVariable.WorkMode.WorkMode = 0x00000000 | AUTO_MODE;
//    }
    GlobalVariable.Motor[0].MotorEnable = 1; 
}
static void AutoRainMode()
{
    static unsigned char  rain_state = 0;
    static unsigned short rain_delay_count = 0;
    static unsigned char  rain_cycle_count = 0;
    
    if((GlobalVariable.WorkMode.WorkMode & 0xFF) != AUTO_RAIN_MODE)
    {
        rain_state = 0;
        rain_delay_count = 0;
        AutoModeFunction = AutoModeSwitch;
        return;
    }
    GlobalVariable.Motor[0].NeedLeadAngle = 0;
    switch(rain_state)
    {
        case 0:/*run to start position*/
            GlobalVariable.WorkMode.Target = 60.0f;
            GlobalVariable.Motor[0].MotorEnable = 1;
            if((GlobalVariable.Motor[0].ActualAngle <= (GlobalVariable.ConfigPara.EWTracingAccuracy + 60.0f))
             &&(GlobalVariable.Motor[0].ActualAngle >= (60.0f - GlobalVariable.ConfigPara.EWTracingAccuracy)))
            {
                rain_cycle_count = 1;
                rain_state = 1;
                GlobalVariable.Motor[0].MotorEnable = 0;
            }
            break;
        case 1:
            GlobalVariable.Motor[0].MotorEnable = 1;
            GlobalVariable.WorkMode.Target = 60 + 10 * rain_cycle_count;
            if(GlobalVariable.WorkMode.Target >= 122.0f)
            {
                rain_cycle_count = 1;
                rain_state = 3;
                GlobalVariable.Motor[0].MotorEnable = 0;
            }
            else
            {
                if(GlobalVariable.Motor[0].ActualAngle > (GlobalVariable.WorkMode.Target - 1.0f))
                {
                    rain_state = 2;
                    rain_delay_count = 0;
                    GlobalVariable.Motor[0].MotorEnable = 0;
                }
            }
            break;
        case 2:
            GlobalVariable.Motor[0].MotorEnable = 0;
            if(rain_delay_count++ >= 2 * 60 * 1000 / TASK_PERIOD)
            {
                rain_delay_count = 0;
                rain_state = 1;
                rain_cycle_count ++;
            }
            break;
        case 3:
            GlobalVariable.Motor[0].MotorEnable = 1;
            GlobalVariable.WorkMode.Target = 120 - 10 * rain_cycle_count;
            if(GlobalVariable.WorkMode.Target <= 58.0f)
            {
                rain_cycle_count = 1;
                rain_state = 1;
                GlobalVariable.Motor[0].MotorEnable = 0;
            }
            else
            {
                
                if(GlobalVariable.Motor[0].ActualAngle <= (GlobalVariable.WorkMode.Target + 1.0f))
                {
                    rain_state = 4;
                    rain_delay_count = 0;
                    GlobalVariable.Motor[0].MotorEnable = 0;
                }
            }
            break;
        case 4:
            GlobalVariable.Motor[0].MotorEnable = 0;
            if(rain_delay_count++ >= 2 * 60 * 1000 / TASK_PERIOD)
            {
                rain_delay_count = 0;
                rain_state = 3;
                rain_cycle_count ++;
            }
            break;
        default :
            rain_state = 0;
            rain_delay_count = 0;
            rain_cycle_count = 0;
            GlobalVariable.Motor[0].MotorEnable = 0;
        break;
            
    }
}
static void AutoSnowMode()
{
    if((GlobalVariable.WorkMode.WorkMode & 0xFF) != AUTO_SNOW_MODE)
    {
        AutoModeFunction = AutoModeSwitch;
        return;
    }
    GlobalVariable.Motor[0].NeedLeadAngle = 0;
    if(GlobalVariable.Motor[0].ActualAngle - 90.0f > 0.1f)
    {
        /* w */
        GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.WSoftLimitAngle - 2;
    }
    else
    {
        GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.ESoftLimitAngle + 2;
    }
    GlobalVariable.Motor[0].MotorEnable = 1;
}
static void AutoTrackMode()
{
    if((GlobalVariable.WorkMode.WorkMode & 0xFF) != AUTO_TRACKER_MODE)
    {
        AutoModeFunction = AutoModeSwitch;
        return;
    }
    GlobalVariable.Motor[0].NeedLeadAngle = 1;
    if(GlobalVariable.AstronomyPara.AstronomicalTargetAngle >= (GlobalVariable.ConfigPara.WSoftLimitAngle - 1))
    {
        GlobalVariable.WorkMode.Target = GlobalVariable.ConfigPara.WSoftLimitAngle - 1;
    }
    else if(GlobalVariable.AstronomyPara.AstronomicalTargetAngle <= (GlobalVariable.ConfigPara.ESoftLimitAngle + 1))
    {
        GlobalVariable.WorkMode.Target = GlobalVariable.ConfigPara.ESoftLimitAngle + 1;
    }
    else
    {
        GlobalVariable.WorkMode.Target = GlobalVariable.AstronomyPara.AstronomicalTargetAngle;
    }
    GlobalVariable.Motor[0].MotorEnable = 1;
}
static void AutoModeSwitch()
{
    GlobalVariable.Motor[0].MotorEnable = 0;
    
    if((GlobalVariable.WorkMode.WorkMode & 0xFF) == AUTO_WIND_MODE)
    {
        AutoModeFunction = AutoWindMode;
    }
    else if((GlobalVariable.WorkMode.WorkMode & 0xFF) == AUTO_BATSOCLOW_MODE)
    {
        AutoModeFunction = AutoBatSOCLowMode;
    }
    else if((GlobalVariable.WorkMode.WorkMode & 0xFF) == AUTO_AI_MODE)
    {
        AutoModeFunction = AutoAIMode;
        GlobalVariable.AIPara.AIModeDelayCount = 0;
    }
    else if((GlobalVariable.WorkMode.WorkMode & 0xFF) == AUTO_RAIN_MODE)
    {
        AutoModeFunction = AutoRainMode;
    }
    else if((GlobalVariable.WorkMode.WorkMode & 0xFF) == AUTO_SNOW_MODE)
    {
        AutoModeFunction = AutoSnowMode;
    }
    else if((GlobalVariable.WorkMode.WorkMode & 0xFF) == AUTO_MODE)
    {
        AutoModeFunction = AutoTrackMode;
    }
    else
    {
        WorkModeFunction = IdelMode;
    }
}

static void AutoMode()
{
    if((GlobalVariable.WorkMode.WorkMode & 0xF0) == AUTO_MODE)
    {
        if(GlobalVariable.WarningAndFault.TimeLost == 0
         &&GlobalVariable.WarningAndFault.RTCError == 0)
        {
            /*Mode Decide*/
            /*WindCheck*/
            WindCheck();
            /*BatSOClowCheck*/
            BatSOCLowCheck();
            /*AutoModeRun*/
            (*AutoModeFunction)();
        }
        else
        {
            GlobalVariable.Motor[0].MotorEnable = 0;
        }
    }
    else
    {
        WorkModeFunction = IdelMode;
        GlobalVariable.Motor[0].MotorEnable = 0;
    }
}

/***********************************************************************************************************************
                                            Public function
***********************************************************************************************************************/
void WorkModeInit()
{
    AutoModeFunction = AutoModeSwitch;
    WorkModeFunction = IdelMode;
}
void WorkModeMg()
{
    (*WorkModeFunction)();
}

