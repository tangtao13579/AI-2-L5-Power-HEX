#include "GlobalDefine.h"
#include "ASProtocol.h"
#include "stm32f10x.h"
#include "core_cm3.h"
#include "BackupRegister.h"

extern GlobalVariableDef GlobalVariable;
/***************************************************************************************************
                                     Private Variable
***************************************************************************************************/
/*R-0, RW-1*/
static void (* RWRegister[114])(unsigned char R_or_RW, unsigned short *value)={(void *)0};
/***************************************************************************************************
                                Jump function
***************************************************************************************************/

static void RDeviceEdition(unsigned char R_or_RW, unsigned short *value );
static void RWWorkMode1(unsigned char R_or_RW, unsigned short *value );
static void RWWorkMode2(unsigned char R_or_RW, unsigned short *value );
static void RWarningAndFault1(unsigned char R_or_RW, unsigned short *value );
static void RWarningAndFault2(unsigned char R_or_RW, unsigned short *value );
static void RDeviceType(unsigned char R_or_RW, unsigned short *value);
static void RWChannelEnable(unsigned char R_or_RW, unsigned short *value);
static void RElevationAngle1(unsigned char R_or_RW, unsigned short *value);
static void RElevationAngle2(unsigned char R_or_RW, unsigned short *value);
static void RAzimuthAngle1(unsigned char R_or_RW, unsigned short *value);
static void RAzimuthAngle2(unsigned char R_or_RW, unsigned short *value);
static void RTargetAngle1(unsigned char R_or_RW, unsigned short *value);
static void RTargetAngle2(unsigned char R_or_RW, unsigned short *value);
static void RRealAngle1(unsigned char R_or_RW, unsigned short *value);
static void RRealAngle2(unsigned char R_or_RW, unsigned short *value);
static void RWTimeYear(unsigned char R_or_RW, unsigned short *value);
static void RWTimeMonth(unsigned char R_or_RW, unsigned short *value);
static void RWTimeDay(unsigned char R_or_RW, unsigned short *value);
static void RWTimeHour(unsigned char R_or_RW, unsigned short *value);
static void RWTimeMinute(unsigned char R_or_RW, unsigned short *value);
static void RWTimeSecond(unsigned char R_or_RW, unsigned short *value);
static void RWRemoteWindSpeed(unsigned char R_or_RW, unsigned short *value);

static void RWAIRemoteTarget(unsigned char R_or_RW, unsigned short *value);
static void RTemp(unsigned char R_or_RW, unsigned short *value);
static void RPVStringCur(unsigned char R_or_RW, unsigned short *value);

static void RMotorCurrent(unsigned char R_or_RW, unsigned short *value);
static void RWProtectCurrent1(unsigned char R_or_RW, unsigned short *value);
static void RWProtectCurrent2(unsigned char R_or_RW, unsigned short *value);
static void RWLongitude1(unsigned char R_or_RW, unsigned short *value);
static void RWLongitude2(unsigned char R_or_RW, unsigned short *value);
static void RWLatitude1(unsigned char R_or_RW, unsigned short *value);
static void RWLatitude2(unsigned char R_or_RW, unsigned short *value);
static void RWTimeZone1(unsigned char R_or_RW, unsigned short *value);
static void RWTimeZone2(unsigned char R_or_RW, unsigned short *value);
static void RWEWTracingAccuracy1(unsigned char R_or_RW, unsigned short *value);
static void RWEWTracingAccuracy2(unsigned char R_or_RW, unsigned short *value);
static void RPVBuckerVoltage(unsigned char R_or_RW, unsigned short *value);
static void RBatState(unsigned char R_or_RW, unsigned short *value);
static void RBatSOCTemp(unsigned char R_or_RW, unsigned short *value);
static void RBatVoltage(unsigned char R_or_RW, unsigned short *value);
static void RBatCurrent(unsigned char R_or_RW, unsigned short *value);

static void RWComID(unsigned char R_or_RW, unsigned short *value);
static void RWPowerMode(unsigned char R_or_RW, unsigned short *value);
static void RWCleaningDockAngle(unsigned char R_or_RW, unsigned short *value);

static void RWPVModuleWidth1(unsigned char R_or_RW, unsigned short *value);
static void RWPVModuleWidth2(unsigned char R_or_RW, unsigned short *value);
static void RWPostSpacing1(unsigned char R_or_RW, unsigned short *value);
static void RWPostSpacing2(unsigned char R_or_RW, unsigned short *value);
static void RWTerrainSlope1(unsigned char R_or_RW, unsigned short *value);
static void RWTerrainSlope2(unsigned char R_or_RW, unsigned short *value);

static void RWUpWindSpeed(unsigned char R_or_RW, unsigned short *value);
static void RWUpStartTime(unsigned char R_or_RW, unsigned short *value);
static void RWLowWindSpeed(unsigned char R_or_RW, unsigned short *value);
static void RWLowStopTime(unsigned char R_or_RW, unsigned short *value);
static void RWESoftLimitAngle1(unsigned char R_or_RW, unsigned short *value);
static void RWESoftLimitAngle2(unsigned char R_or_RW, unsigned short *value);
static void RWWSoftLimitAngle1(unsigned char R_or_RW, unsigned short *value);
static void RWWSoftLimitAngle2(unsigned char R_or_RW, unsigned short *value);
static void RWBackAngle(unsigned char R_or_RW, unsigned short *value);    
static void RWEWindProtectionAngle(unsigned char R_or_RW, unsigned short *value);
static void RWWWindProtectionAngle(unsigned char R_or_RW, unsigned short *value);

static void RDeviceEdition(unsigned char R_or_RW, unsigned short *value )
{
    if(R_or_RW == 0)
    {
        *value = GlobalVariable.FixePara.DeviceEdition;
    }
}

static void RWWorkMode1(unsigned char R_or_RW, unsigned short *value )
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.WorkMode.WorkMode)+1);
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.WorkModeBuffer.WorkMode)+1) = *value;
    }
}

static void RWWorkMode2(unsigned char R_or_RW, unsigned short *value )
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.WorkMode.WorkMode));
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.WorkModeBuffer.WorkMode)) = *value;
        if((GlobalVariable.WorkModeBuffer.WorkMode & 0xFF) == 0xFF)
        {
            NVIC_SystemReset();
        }
        else if ((GlobalVariable.WorkModeBuffer.WorkMode & 0xFF) == AUTO_AI_MODE)
        {
            if((GlobalVariable.WorkMode.WorkMode & 0xFF) == AUTO_TRACKER_MODE)
            {
                GlobalVariable.WorkMode.WorkMode = GlobalVariable.WorkModeBuffer.WorkMode;
				GlobalVariable.AIPara.AIRemoteAngle=GlobalVariable.Motor[0].ActualAngle;
            }
        }
        else
        {
            GlobalVariable.WorkMode.WorkMode = GlobalVariable.WorkModeBuffer.WorkMode;
        }
    }
}
static void RWarningAndFault1(unsigned char R_or_RW, unsigned short *value )
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.WarningAndFault)+1);
    }
}
static void RWarningAndFault2(unsigned char R_or_RW, unsigned short *value )
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.WarningAndFault));
    }
}
static void RDeviceType(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = GlobalVariable.FixePara.DeviceType;
    }
}
static void RWChannelEnable(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = 0x0007 & GlobalVariable.ConfigPara.ChannelEnable;
    }
    else if(R_or_RW == 1)
    {
        GlobalVariable.ConfigPara.ChannelEnable = 0x0007 & (*value);
        GlobalVariable.WriteFlag.ConfigParaWrite = 1;
    }
}

static void RElevationAngle1(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.AstronomyPara.ElevationAngle)+1);
    }
}
static void RElevationAngle2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.AstronomyPara.ElevationAngle));
    }
}
static void RAzimuthAngle1(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.AstronomyPara.AzimuthAngle)+1);
    }
}
static void RAzimuthAngle2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.AstronomyPara.AzimuthAngle));
    }
}
static void RTargetAngle1(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.WorkMode.Target)+1);
    }
}
static void RTargetAngle2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.WorkMode.Target));
    }
}
static void RRealAngle1(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.Motor[0].ActualAngle)+1);
    }
}
static void RRealAngle2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.Motor[0].ActualAngle));
    }
}
static void RWTimeYear(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = GlobalVariable.Time.Year;
    }
    else if(R_or_RW == 1)
    {
        GlobalVariable.TimeBuffer.Year = *value;
        GlobalVariable.WriteFlag.TimeWrite = 1;
    }
}
static void RWTimeMonth(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = GlobalVariable.Time.Month;
    }
    else if(R_or_RW == 1)
    {
        GlobalVariable.TimeBuffer.Month = *value;
        GlobalVariable.WriteFlag.TimeWrite = 1;
    }
}
static void RWTimeDay(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = GlobalVariable.Time.Day;
    }
    else if(R_or_RW == 1)
    {
        GlobalVariable.TimeBuffer.Day = *value;
        GlobalVariable.WriteFlag.TimeWrite = 1;
    }    
}
static void RWTimeHour(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = GlobalVariable.Time.Hour;
    }
    else if(R_or_RW == 1)
    {
        GlobalVariable.TimeBuffer.Hour = *value;
        GlobalVariable.WriteFlag.TimeWrite = 1;
    }
}
static void RWTimeMinute(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = GlobalVariable.Time.Minute;
    }
    else if(R_or_RW == 1)
    {
        GlobalVariable.TimeBuffer.Minute = *value;
        GlobalVariable.WriteFlag.TimeWrite = 1;
    }
}
static void RWTimeSecond(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = GlobalVariable.Time.Second;
    }
    else if(R_or_RW == 1)
    {
        GlobalVariable.TimeBuffer.Second = *value;
        BKPWriteRTCUpdateFlag();
        GlobalVariable.WriteFlag.TimeWrite = 1;
    }
}

static void RWRemoteWindSpeed(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = GlobalVariable.Weather.RemoteWindSpeed;
    }
    else if(R_or_RW == 1)
    {
        GlobalVariable.Weather.RemoteWindSpeed = *value;
    }
}

static void RWAIRemoteTarget(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = (unsigned short)GlobalVariable.AIPara.AIRemoteAngle;
    }
    else if(R_or_RW == 1)
    {
        GlobalVariable.AIPara.AIRemoteAngle = (float)*value;
        GlobalVariable.AIPara.AIModeDelayCount = 0;
    }
}
static void RPVStringCur(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = (unsigned char)(GlobalVariable.PowerPara.PVStringUpCur * 10.0f);
        *value = *value << 8;
        *value &= 0xFF00;
        *value |= (unsigned char)(GlobalVariable.PowerPara.PVStringDownCur * 10.0f);
    }
}
static void RTemp(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = 0;
        *value = *(unsigned short *)&GlobalVariable.Tmp.BoardTmp;
    }
}
static void RMotorCurrent(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = (short)(GlobalVariable.Motor[0].Motorcurrent * 10);
    }
}
static void RWProtectCurrent1(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.MotorOverCurrentValue)+1);
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.MotorOverCurrentValue)+1) = *value;
    }
}

static void RWProtectCurrent2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.MotorOverCurrentValue));
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.MotorOverCurrentValue)) = *value;
        if((GlobalVariable.ConfigParaBuffer.MotorOverCurrentValue > 0.19f) 
			&& (GlobalVariable.ConfigParaBuffer.MotorOverCurrentValue < 6.1f))
        {
            GlobalVariable.ConfigPara.MotorOverCurrentValue = GlobalVariable.ConfigParaBuffer.MotorOverCurrentValue;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
    }
}

/* Longitude */
static void RWLongitude1(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.Longitude)+1);
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.Longitude)+1) = *value;
    }
}
static void RWLongitude2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.Longitude));
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.Longitude)) = *value;
        
        if(GlobalVariable.ConfigParaBuffer.Longitude >= -180.0
            &&GlobalVariable.ConfigParaBuffer.Longitude <= 180.0 )
        {
            GlobalVariable.ConfigPara.Longitude = GlobalVariable.ConfigParaBuffer.Longitude;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
    }
}

/* Latitude */
static void RWLatitude1(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.Latitude)+1);
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.Latitude)+1) = *value;
    }
}
static void RWLatitude2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.Latitude));
    }
    else if(R_or_RW == 1)
    {
        
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.Latitude)) = *value;
        
        if(GlobalVariable.ConfigParaBuffer.Latitude >= -90.0 
            && GlobalVariable.ConfigParaBuffer.Latitude <= 90.0)
        {
            GlobalVariable.ConfigPara.Latitude = GlobalVariable.ConfigParaBuffer.Latitude;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
    }
}
static void RWTimeZone1(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.TimeZone)+1);
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.TimeZone)+1) = *value;
    }
}
static void RWTimeZone2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.TimeZone));
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.TimeZone)) = *value;
        
        if(GlobalVariable.ConfigParaBuffer.TimeZone >= -12.0
            && GlobalVariable.ConfigParaBuffer.TimeZone <= 12.0)
        {
            GlobalVariable.ConfigPara.TimeZone = GlobalVariable.ConfigParaBuffer.TimeZone;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
    }
}
static void RWEWTracingAccuracy1(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
         *value = *(((unsigned short *)&GlobalVariable.ConfigPara.EWTracingAccuracy)+1);
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.EWTracingAccuracy)+1) = *value;
    }
}
static void RWEWTracingAccuracy2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.EWTracingAccuracy));
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.EWTracingAccuracy)) = *value;
        
        if(GlobalVariable.ConfigParaBuffer.EWTracingAccuracy >= 0.98
            && GlobalVariable.ConfigParaBuffer.EWTracingAccuracy <= 5.1)
        {
            GlobalVariable.ConfigPara.EWTracingAccuracy = GlobalVariable.ConfigParaBuffer.EWTracingAccuracy;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
    }
}

static void RPVBuckerVoltage(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = (unsigned short)(GlobalVariable.PowerPara.PVBuckerVoltage * 10);
    }
}
static void RBatState(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = 0;
        *value |= *(unsigned char*)(&GlobalVariable.PowerPara.ChargeState);
    }
}
static void RBatSOCTemp(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = 0x00FF & (unsigned char)(GlobalVariable.PowerPara.BatterySOC*100);
        *value = *value << 8;
        *value &= 0xFF00;
        *value |= ((signed char)(GlobalVariable.PowerPara.BatteryTemperature) & 0x00FF);
    }
}
static void RBatVoltage(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = (unsigned short)(GlobalVariable.PowerPara.BatteryVoltage*10);
    }
}
static void RBatCurrent(unsigned char R_or_RW, unsigned short *value)
{
    signed short temp = 0;
    if(R_or_RW == 0)
    {
        temp = (signed short)(GlobalVariable.PowerPara.BatteryCurrent*100);
        *value = *((unsigned short *)&temp);
    }
}
static void RWComID(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.ComID));
    }
    else if(R_or_RW == 1)
    {        
        if(*value >= 1 && *value < 255)
        {
            GlobalVariable.ConfigPara.ComID =(unsigned char) *value;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
    }
}
static void RWPowerMode(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = GlobalVariable.ConfigPara.PowerMode;
    }
    else if(R_or_RW == 1)
    {
        if(*value <= 3)
        {
            GlobalVariable.ConfigPara.PowerMode = *value;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        } 
    }
}

static void RWCleaningDockAngle(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = GlobalVariable.ConfigPara.CleaningDockAngle;
    }
    else if(R_or_RW == 1)
    {
        if(*value >= 30 && *value <= 150)
        {
            GlobalVariable.ConfigPara.CleaningDockAngle = *value;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        } 
    }
}

static void RWMotorOrientation(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = GlobalVariable.ConfigPara.MotorOrientation;
    }
    else if(R_or_RW == 1)
    {
			  if(*value == 0 || *value == 1)  //0：电机朝东； 1：电机朝西
        {
            GlobalVariable.ConfigPara.MotorOrientation = *value;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        } 
    }
}

static void RWPVModuleWidth1(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.PVModuleWidth)+1);
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.PVModuleWidth)+1) = *value;
    }
}
static void RWPVModuleWidth2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.PVModuleWidth));
    }
    else if(R_or_RW == 1)
    {
         *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.PVModuleWidth)) = *value;
        
        if(GlobalVariable.ConfigParaBuffer.PVModuleWidth >= 0.2
            && GlobalVariable.ConfigParaBuffer.PVModuleWidth <= 100.0 )
        {
            GlobalVariable.ConfigPara.PVModuleWidth = GlobalVariable.ConfigParaBuffer.PVModuleWidth;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
    }
}
static void RWPostSpacing1(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.PostSpacing)+1);
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.PostSpacing)+1) = *value;
    }
}
static void RWPostSpacing2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.PostSpacing));
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.PostSpacing)) = *value;
        
        if(GlobalVariable.ConfigParaBuffer.PostSpacing >= GlobalVariable.ConfigPara.PVModuleWidth
        && GlobalVariable.ConfigParaBuffer.PostSpacing <= 100.0 )
        {
            GlobalVariable.ConfigPara.PostSpacing = GlobalVariable.ConfigParaBuffer.PostSpacing;
        }
        else
        {
            GlobalVariable.ConfigPara.PostSpacing = GlobalVariable.ConfigPara.PVModuleWidth - 0.1;
        }
        GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        
    }
}
static void RWTerrainSlope1(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.TerrainSlope)+1);
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.TerrainSlope)+1) = *value;
    }
}
static void RWTerrainSlope2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.TerrainSlope));
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.TerrainSlope)) = *value;
        
        if(GlobalVariable.ConfigParaBuffer.TerrainSlope >= -45.0f 
            && GlobalVariable.ConfigParaBuffer.TerrainSlope <= 45.0f)
        {
            GlobalVariable.ConfigPara.TerrainSlope = GlobalVariable.ConfigParaBuffer.TerrainSlope;
            
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
    }
}


static void RWUpWindSpeed(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.UpWindSpeed));
    }
    else if(R_or_RW == 1)
    {
        if(*value <= 40)
        {
            *(((unsigned short *)&GlobalVariable.ConfigPara.UpWindSpeed)) = *value;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
    }
}
static void RWUpStartTime(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.UpStartTime));
    }
    else if(R_or_RW == 1)
    {
        if(*value <= 1000)
        {
           *(((unsigned short *)&GlobalVariable.ConfigPara.UpStartTime)) = *value;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1; 
        }
    }
}
static void RWLowWindSpeed(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.LowWindSpeed));
    }
    else if(R_or_RW == 1)
    {
        if(*value <= (GlobalVariable.ConfigPara.UpWindSpeed - 1))
        {
            *(((unsigned short *)&GlobalVariable.ConfigPara.LowWindSpeed)) = *value;
        }
        else
        {
            *(((unsigned short *)&GlobalVariable.ConfigPara.LowWindSpeed)) = GlobalVariable.ConfigPara.UpWindSpeed - 1;
        }
        GlobalVariable.WriteFlag.ConfigParaWrite = 1;
    }
}
static void RWLowStopTime(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.LowStopTime));
    }
    else if(R_or_RW == 1)
    {
        if(*value <= 1000)
        {
            *(((unsigned short *)&GlobalVariable.ConfigPara.LowStopTime)) = *value;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
    }
}

static void RWESoftLimitAngle1(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.ESoftLimitAngle)+1);
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.ESoftLimitAngle)+1) = *value;
    }
}
static void RWESoftLimitAngle2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.ESoftLimitAngle));
    }
    else if(R_or_RW == 1)
    {   
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.ESoftLimitAngle)) = *value;
        if(GlobalVariable.ConfigParaBuffer.ESoftLimitAngle >= 29.9 
            && GlobalVariable.ConfigParaBuffer.ESoftLimitAngle  < 60.1 )
        {
            GlobalVariable.ConfigPara.ESoftLimitAngle = GlobalVariable.ConfigParaBuffer.ESoftLimitAngle;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
        
    }
}
static void RWWSoftLimitAngle1(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.WSoftLimitAngle)+1);
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.WSoftLimitAngle)+1) = *value;
    }
}
static void RWWSoftLimitAngle2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.WSoftLimitAngle));
    }
    else if(R_or_RW == 1)
    {
        *(((unsigned short *)&GlobalVariable.ConfigParaBuffer.WSoftLimitAngle)) = *value;
        if(GlobalVariable.ConfigParaBuffer.WSoftLimitAngle > 119.9
            && GlobalVariable.ConfigParaBuffer.WSoftLimitAngle <= 150.1)
        {
            GlobalVariable.ConfigPara.WSoftLimitAngle = GlobalVariable.ConfigParaBuffer.WSoftLimitAngle;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
        
    }
}
static void RWBackAngle(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.BackAngle));
    }
    else if(R_or_RW == 1)
    {
        if(*value >= 59.9 && *value <= 120.1)
        {
            *(((unsigned short *)&GlobalVariable.ConfigPara.BackAngle)) = *value;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
    }
}
static void RWEWindProtectionAngle(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.EWindProtectionAngle));
    }
    else if(R_or_RW == 1)
    {
        if(*value >= 60 && *value <= 90)
        {
           *(((unsigned short *)&GlobalVariable.ConfigPara.EWindProtectionAngle)) = *value;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1; 
        } 
    }
}
static void RWWWindProtectionAngle(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.ConfigPara.WWindProtectionAngle));
    }
    else if(R_or_RW == 1)
    {
        if(*value >= 90 && *value <= 120)
        {
            *(((unsigned short *)&GlobalVariable.ConfigPara.WWindProtectionAngle)) = *value;
            GlobalVariable.WriteFlag.ConfigParaWrite = 1;
        }
    }
}

/***************************************************************************************************
                                Jump table
***************************************************************************************************/
void ASProtocolInit()
{
    RWRegister[0]   = RDeviceEdition;
    RWRegister[1]   = RWWorkMode1;
    RWRegister[2]   = RWWorkMode2;
    RWRegister[3]   = RWarningAndFault1;
    RWRegister[4]   = RWarningAndFault2;
    RWRegister[5]   = RDeviceType;
    RWRegister[6]   = RWChannelEnable;
    RWRegister[7]   = RElevationAngle1;
    RWRegister[8]   = RElevationAngle2;
    RWRegister[9]   = RAzimuthAngle1;
    RWRegister[10]  = RAzimuthAngle2;
    
    RWRegister[11]  = RTargetAngle1;
    RWRegister[12]  = RTargetAngle2;
    RWRegister[13]  = RRealAngle1;
    RWRegister[14]  = RRealAngle2;
    
    RWRegister[23]  = RWTimeYear;
    RWRegister[24]  = RWTimeMonth;
    RWRegister[25]  = RWTimeDay;
    RWRegister[26]  = RWTimeHour;
    RWRegister[27]  = RWTimeMinute;
    RWRegister[28]  = RWTimeSecond;
    
    RWRegister[33]  = RWRemoteWindSpeed;
    RWRegister[34]  = RWAIRemoteTarget;
    
    RWRegister[38]  = RPVStringCur;
    RWRegister[39]  = RTemp;
    RWRegister[40]  = RMotorCurrent;
    
    RWRegister[43]  = RWProtectCurrent1;
    RWRegister[44]  = RWProtectCurrent2;
    
    RWRegister[45]  = RWLongitude1;
    RWRegister[46]  = RWLongitude2;
    RWRegister[47]  = RWLatitude1;
    RWRegister[48]  = RWLatitude2;
    RWRegister[49]  = RWTimeZone1;
    RWRegister[50]  = RWTimeZone2;
    
    RWRegister[51]  = RWEWTracingAccuracy1;
    RWRegister[52]  = RWEWTracingAccuracy2;
    
    RWRegister[55]  = RPVBuckerVoltage;
    RWRegister[56]  = RBatState;
    RWRegister[57]  = RBatSOCTemp;
    RWRegister[58]  = RBatVoltage;
    RWRegister[59]  = RBatCurrent;
    
    RWRegister[81]  = RWComID;
    
    RWRegister[82]  = RWPowerMode;
    RWRegister[84]  = RWCleaningDockAngle;
		
		RWRegister[85]  = RWMotorOrientation;
    
    RWRegister[89]  = RWPVModuleWidth1;
    RWRegister[90]  = RWPVModuleWidth2;
    RWRegister[91]  = RWPostSpacing1;
    RWRegister[92]  = RWPostSpacing2;
    RWRegister[93]  = RWTerrainSlope1;
    RWRegister[94]  = RWTerrainSlope2;

    RWRegister[99]  = RWUpWindSpeed;
    RWRegister[100] = RWUpStartTime;
    RWRegister[101] = RWLowWindSpeed;
    RWRegister[102] = RWLowStopTime;
    
    RWRegister[103] = RWESoftLimitAngle1;
    RWRegister[104] = RWESoftLimitAngle2;
    RWRegister[105] = RWWSoftLimitAngle1;
    RWRegister[106] = RWWSoftLimitAngle2;
    
    RWRegister[111] = RWBackAngle;
    RWRegister[112] = RWEWindProtectionAngle;
    RWRegister[113] = RWWWindProtectionAngle;

}
void *GetASProtocl()
{
    return RWRegister;
}






