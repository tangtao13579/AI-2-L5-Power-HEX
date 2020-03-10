#include "GlobalDefine.h"
#include "WorkModeControl.h"


extern GlobalVariableDef GlobalVariable;

static void (*WorkModeFunction)(unsigned int work_mode);
/***************************************************************************************************
							Private functions
***************************************************************************************************/
static void IdleMode(unsigned int work_mode);
static void AutoTrackeMode(unsigned int work_mode);
static void ManualEastMode(unsigned int work_mode);
static void ManualWestMode(unsigned int work_mode);
static void AutoRainMode(unsigned int work_mode);
static void AutoSnowMode(unsigned int work_mode);
static void AutoWindMode(unsigned int work_mode);
static void AngleCalibrationMode(unsigned int work_mode);
    
static void IdleMode(unsigned int work_mode)
{
	GlobalVariable.Motor[0].MotorEnable = 0;
    GlobalVariable.WorkMode.WorkMode = 0;
	switch(work_mode & 0xFF)
	{
		case AUTO_MODE:
			WorkModeFunction = AutoTrackeMode;
			break;
		case MANUAL_EAST_MODE:
			WorkModeFunction = ManualEastMode;
			break;
		case MANUAL_WEST_MODE:
			WorkModeFunction = ManualWestMode;
			break;	
		case AUTO_RAIN_MODE:
			WorkModeFunction = AutoRainMode;
			break;
		case AUTO_SNOW_MODE:
			WorkModeFunction = AutoSnowMode;
			break;
		case AUTO_WIND_MODE:
			WorkModeFunction = AutoWindMode;
            break;
        case ANGLE_CALIBRATION_MODE:
            WorkModeFunction = AngleCalibrationMode;
			break;
		default:
            break;
	}
}
static void AutoTrackeMode(unsigned int work_mode)
{
	if((work_mode & 0xFF) == AUTO_MODE)
	{
        GlobalVariable.WorkMode.WorkMode = 0x00000000 | AUTO_MODE;
        
		if(GlobalVariable.WarningAndFault.TimeLost == 0
		&&GlobalVariable.WarningAndFault.RTCError == 0
		&&GlobalVariable.WarningAndFault.BatNoCom == 0
		&&GlobalVariable.WarningAndFault.BatError == 0)
		{
            GlobalVariable.Motor[0].MotorEnable = 1;
			if(GlobalVariable.WarningAndFault.BatSOCLow == 0)
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
			else
			{
				GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.BackAngle;
			}
		}
	}
	else
	{
        GlobalVariable.Motor[0].MotorEnable = 0;
		WorkModeFunction = IdleMode;
	}
}
static void ManualEastMode(unsigned int work_mode)
{
	if((work_mode & 0xFF) == MANUAL_EAST_MODE)
	{
        GlobalVariable.WorkMode.WorkMode = 0x00000000 | MANUAL_EAST_MODE;
        GlobalVariable.Motor[0].MotorEnable = 1;
		GlobalVariable.WorkMode.Target = GlobalVariable.ConfigPara.ESoftLimitAngle+2;
	}
	else
	{
		WorkModeFunction = IdleMode;
	}
}
static void ManualWestMode(unsigned int work_mode)
{
	if((work_mode & 0xFF) == MANUAL_WEST_MODE)
	{
        GlobalVariable.WorkMode.WorkMode = 0x00000000 | MANUAL_WEST_MODE;
        
        GlobalVariable.Motor[0].MotorEnable = 1;
        GlobalVariable.WorkMode.Target = GlobalVariable.ConfigPara.WSoftLimitAngle-2;
	}
	else
	{
		WorkModeFunction = IdleMode;
	}
}
static void AutoRainMode(unsigned int work_mode)
{
	if((work_mode & 0xFF) == AUTO_RAIN_MODE)
	{
        GlobalVariable.WorkMode.WorkMode = 0x00000000 | AUTO_RAIN_MODE;
        
		if(GlobalVariable.WarningAndFault.TimeLost == 0
		&&GlobalVariable.WarningAndFault.RTCError == 0
		&&GlobalVariable.WarningAndFault.BatNoCom == 0
		&&GlobalVariable.WarningAndFault.BatError == 0)
		{
			if(GlobalVariable.WarningAndFault.BatSOCLow == 0)
			{
				/*  */
			}
			else
			{
				GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.BackAngle;
			}
		}
	}
	else
	{
		WorkModeFunction = IdleMode;
	}
}
static void AutoSnowMode(unsigned int work_mode)
{
	if((work_mode & 0xFF) == AUTO_SNOW_MODE)
	{
        GlobalVariable.WorkMode.WorkMode = 0x00000000 | AUTO_SNOW_MODE;
        
		if(GlobalVariable.WarningAndFault.TimeLost == 0
		&&GlobalVariable.WarningAndFault.RTCError == 0
		&&GlobalVariable.WarningAndFault.BatNoCom == 0
		&&GlobalVariable.WarningAndFault.BatError == 0)
		{
			if(GlobalVariable.WarningAndFault.BatSOCLow == 0)
			{
				if(GlobalVariable.Motor[0].ActualAngle - 90 > 0.001)
				{
					/* w */
					GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.WSoftLimitAngle - 2;
				}
				else if (GlobalVariable.Motor[0].ActualAngle - 90 > -3.0)
				{
					GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.ESoftLimitAngle + 2;
				}
				else
				{
					GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.WSoftLimitAngle - 2;
				}
			}
			else
			{
				GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.BackAngle;
			}
		}
	}
	else
	{
		WorkModeFunction = IdleMode;
	}
}
static void AutoWindMode(unsigned int work_mode)
{
	if((work_mode & 0xFF) == AUTO_WIND_MODE)
	{
        GlobalVariable.WorkMode.WorkMode = 0x00000000 | AUTO_WIND_MODE;
        
		if(GlobalVariable.WarningAndFault.TimeLost == 0
		&&GlobalVariable.WarningAndFault.RTCError == 0
		&&GlobalVariable.WarningAndFault.BatNoCom == 0
		&&GlobalVariable.WarningAndFault.BatError == 0)
		{
			if(GlobalVariable.Motor[0].ActualAngle - 90 > 0.001)
			{
				/* w */
				GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.WWindProtectionAngle;
			}
			else if (GlobalVariable.Motor[0].ActualAngle - 90 > -3.0)
			{
				GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.EWindProtectionAngle;
			}
			else
			{
				GlobalVariable.WorkMode.Target = (float)GlobalVariable.ConfigPara.WWindProtectionAngle;
			}
		}
	}
	else
	{
		WorkModeFunction = IdleMode;
	}
}

static void AngleCalibrationMode(unsigned int work_mode)
{
    static unsigned char delay_count = 0;
    float temp_correction_value = 0.0f;
    GlobalVariable.Motor[0].MotorEnable = 0;
    
    GlobalVariable.WorkMode.WorkMode = 0x00000000 | ANGLE_CALIBRATION_MODE;
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
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
    }
    GlobalVariable.WorkModeBuffer.WorkMode = 0;
    GlobalVariable.WriteFlag.WorkModeWrite = 1;
    WorkModeFunction = IdleMode;
}
/***************************************************************************************************
								Public Functions
***************************************************************************************************/

void (**GetWorkModeFunction())(unsigned int work_mode)
{
    WorkModeFunction = IdleMode;
	return &WorkModeFunction;
}
