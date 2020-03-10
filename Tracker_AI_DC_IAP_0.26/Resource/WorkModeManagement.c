#include "GlobalDefine.h"
#include "WorkModeManagement.h"

extern GlobalVariableDef GlobalVariable;
static unsigned char rain_mode_state = 0;
/***********************************************************************************************************************
                                            Private function
***********************************************************************************************************************/
static unsigned char WindCheck(void);
static void IdelAction(void);
static void AutoTrackerAction(void);
static void AutoWindAction(void);
static void AutoBatSOCLowAction(void);
static void AutoRainAction(void);
static void AutoSnowAction(void);
static void AutoAIAction(void);
static void ManualEastAction(void);
static void ManualWestAction(void);

static void IdelMode(void);
static void AutoMode(void);
static void ManualMode(void);
static void AngleCailbrationMode(void);

static unsigned char WindCheck()
{
    static float big_wind_count = 0.0f;
    static float breeze_wind_count = 0.0f; 
    static unsigned char is_big_wind = 0;
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
    if(big_wind_count * 50.0f / 1000.0f >= GlobalVariable.ConfigPara.UpStartTime)
    {
        big_wind_count = 0;
        is_big_wind = 1;
    }
    if(breeze_wind_count * 50.0f /1000.0f >= GlobalVariable.ConfigPara.LowStopTime)
    {
        breeze_wind_count = 0;
        is_big_wind = 0;
        
    }
    return is_big_wind;
}
static void IdelAction()
{
    GlobalVariable.WorkMode.Target = GlobalVariable.AstronomyPara.AstronomicalTargetAngle;
}

static void AutoTrackerAction()
{
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
}

static void AutoWindAction()
{
    if(GlobalVariable.Motor[0].ActualAngle - 90.0f > 3.0f)
    {
        /* w */
        GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.WWindProtectionAngle;
    }
    else if (GlobalVariable.Motor[0].ActualAngle - 90.0f < -3.0f)
    {
        GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.EWindProtectionAngle;
    }
    else
    {
        GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.EWindProtectionAngle;
    }
}

static void AutoBatSOCLowAction()
{
    GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.BackAngle;
}

static void AutoRainAction()
{
    static unsigned short delay_count = 0;
    delay_count++;
    
    if(delay_count )
    
}

static void AutoSnowAction()
{
    if(GlobalVariable.Motor[0].ActualAngle - 90 > 3.0)
    {
        /* w */
        GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.WSoftLimitAngle - 2;
    }
    else if (GlobalVariable.Motor[0].ActualAngle - 90 < -3.0)
    {
        GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.ESoftLimitAngle + 2;
    }
    else
    {
        GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.WSoftLimitAngle - 2;
    }
}

static void AutoAIAction()
{
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
    if(GlobalVariable.AIPara.AIModeDelayCount ++ > 18000)
    {
        GlobalVariable.AIPara.AIModeDelayCount = 0;
        GlobalVariable.WorkMode.WorkMode = 0x00000000 | AUTO_MODE;
    }
}

static void ManualEastAction()
{
    GlobalVariable.WorkMode.Target = GlobalVariable.ConfigPara.ESoftLimitAngle+2;
}

static void ManualWestAction()
{
    GlobalVariable.WorkMode.Target = GlobalVariable.ConfigPara.WSoftLimitAngle-2;
}
    
static void IdelMode()
{
    IdelAction();
    GlobalVariable.Motor[0].MotorEnable = 0;
}
static void AutoMode()
{
    unsigned char work_mode;
    
    work_mode = GlobalVariable.WorkMode.WorkMode & 0xFF;
    
    if(GlobalVariable.WarningAndFault.TimeLost == 0
     &&GlobalVariable.WarningAndFault.RTCError == 0
     &&GlobalVariable.WarningAndFault.BatNoCom == 0
     &&GlobalVariable.WarningAndFault.BatError == 0) /*电源部分需要修改*/
   {
       /*风速判断*/
       if((WindCheck() == 1)||(work_mode == AUTO_WIND_MODE))
       {
           /*wind*/
           AutoWindAction();
           GlobalVariable.Motor[0].MotorEnable = 1;
       }
       else if(GlobalVariable.WarningAndFault.BatSOCLow != 0)
       {
           /*BatSOCLow*/
           AutoBatSOCLowAction();
           GlobalVariable.Motor[0].MotorEnable = 1;
       }
       else if(work_mode == AUTO_AI_MODE)
       {
           /*AI-MODE*/
           AutoAIAction();
           GlobalVariable.Motor[0].MotorEnable = 1;
       }
       else if(work_mode == AUTO_RAIN_MODE)
       {
           /*Rain mode*/
           AutoRainAction();
           GlobalVariable.Motor[0].MotorEnable = 1;
       }
       else if(work_mode == AUTO_SNOW_MODE)
       {
           /*Snow mode*/
           AutoSnowAction();
           GlobalVariable.Motor[0].MotorEnable = 1;
       }
       else if(work_mode == AUTO_MODE)
       {
           AutoTrackerAction();
           GlobalVariable.Motor[0].MotorEnable = 1;
       }
       else
       {
           IdelAction();
           GlobalVariable.Motor[0].MotorEnable = 0;
       }
   }
   else
   {
       IdelAction();
       GlobalVariable.Motor[0].MotorEnable = 0;
   }
}

static void ManualMode()
{
    unsigned char work_mode;
    work_mode = GlobalVariable.WorkMode.WorkMode & 0xFF;

    if(work_mode == MANUAL_EAST_MODE)
    {
        ManualEastAction();
        GlobalVariable.Motor[0].MotorEnable = 1;
    }
    else if (work_mode == MANUAL_WEST_MODE)
    {
        ManualWestAction();
        GlobalVariable.Motor[0].MotorEnable = 1;
    }
    else
    {
        IdelAction();
        GlobalVariable.Motor[0].MotorEnable = 0;
    }
}
static void AngleCailbrationMode()
{
    static unsigned char delay_count = 0;
    float temp_correction_value = 0.0f;
    if(GlobalVariable.WorkMode.WorkMode == (0x00000000 | ANGLE_CALIBRATION_MODE))
    {
        GlobalVariable.Motor[0].MotorEnable = 0;
        delay_count ++;
        if(delay_count < 60)
        {
          return;   /*delay until stable*/
        }
        delay_count = 0;
        if(GlobalVariable.WarningAndFault.T0AngleNoChange == 0
         &&GlobalVariable.WarningAndFault.T0AngleSensorNoCom == 0)
        {
            temp_correction_value = 90 - GlobalVariable.Motor[0].ActualAngle;
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
    else
    {
        delay_count = 0;
        GlobalVariable.WorkMode.WorkMode = 0; 
    }
    
}

/***********************************************************************************************************************
                                            Public function
***********************************************************************************************************************/
void WorkModeMg()
{
    if((GlobalVariable.WorkMode.WorkMode & 0x20) == 0x20)
    {
        AutoMode();
    }
    else if((GlobalVariable.WorkMode.WorkMode & 0x10) == 0x10)
    {
        ManualMode();
    }
    else if((GlobalVariable.WorkMode.WorkMode & 0x40) == 0x40)
    {
        AngleCailbrationMode();
    }
    else
    {
        IdelMode();
    }

}
