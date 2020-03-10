#ifndef _GLOBAL_DEFINE_H_ 
#define _GLOBAL_DEFINE_H_

#define MAINTENANCE_MODE            0x40
#define ANGLE_CALIBRATION_MODE      0x80

#define AUTO_MODE                   0x20
#define AUTO_TRACKER_MODE           0x20
#define AUTO_RAIN_MODE              0x21
#define AUTO_SNOW_MODE              0x22
#define AUTO_WIND_MODE              0x24
#define AUTO_AI_MODE                0x28
#define AUTO_BATSOCLOW_MODE         0x25

#define MANUAL_MODE                 0x10
#define MANUAL_EAST_MODE            0x18
#define MANUAL_WEST_MODE            0x14

typedef struct
{
    unsigned T0OverCurrent      :1;
    unsigned T0Reversed         :1;
    unsigned T0ESoftLimit       :1;
    unsigned T0WSoftLimit       :1;
    unsigned T0AngleSensorNoCom :1;
    unsigned T0AngleNoChange    :1;
    unsigned T0HardLimit        :1;
    unsigned Reserved           :1;
    
    unsigned Reserved1          :8;
    unsigned Reserved2          :8;
    
    unsigned RTCError           :1;
    unsigned RemoteTimeError    :1;
    unsigned EMSOpen            :1;
    unsigned Reserved3          :1;
    unsigned BatSOCLow          :1;
    unsigned BatError           :1;
    unsigned TimeLost           :1;
    unsigned BatNoCom           :1;

}WarningAndFaultDef;

typedef struct
{
    unsigned short Year;
    unsigned short Month;
    unsigned short Day;
    unsigned short Hour;
    unsigned short Minute;
    unsigned short Second;
    float          TodayTime;
}TimeDef;

typedef struct
{
    unsigned char NeedLeadAngle;     /*Set 1 to indicate that you need to lead angle*/
    unsigned char MotorEnable;
    unsigned char MotorRunningState;
    unsigned char MotorActualDir;    /*0-stop, 1-east, 2-West*/
    unsigned char MotorControlDir;   /*0-stop, 1-east, 2-West; Control direction changes before the actual direction*/
    float         ActualAngle;
    float         Motorcurrent;
    
}MotorDef;

typedef struct
{
    unsigned char Heating            :1;
    unsigned char PreCharge          :1;
    unsigned char CCCV               :1;
    unsigned char Term               :1;
    
    unsigned char MaxChargeTimeFault :1;
    unsigned char BatMissingFault    :1;
    unsigned char BatShortFault      :1;
    unsigned char Reserved           :1;

}ChargeSateDef;

typedef struct
{
    float          PVStringUpCur;
    float          PVStringDownCur;
    float          PVBuckerVoltage;
    float          EmergencyVoltage;
    float          BatteryCurrent;
    float          BatteryVoltage;
    float          BatterySOC;
    float          BatteryTemperature;
    ChargeSateDef  ChargeState;
    
}PowerParaDef;

typedef struct
{
    float SunRiseTime;
    float SunSetTime;
    
    float ElevationAngle;
    float AzimuthAngle;
    
    float AstronomicalTargetAngle;
    
}AstronomyParaDef;

typedef struct
{
    unsigned short RemoteWindSpeed;
}WeatherDef;

typedef struct
{
    signed short BoardTmp;
    signed short AngleSensorTmp;
}TmpDef;

typedef struct
{
    unsigned int PreWorkMode;   /* Previous WorkMode */
    unsigned int WorkMode;      /* Only valid if SystemStatus equal to 0*/
    unsigned int SystemStatus;  /* 0-normal, 1-Flash wipe, 2-Update, 3-Update finish*/
    float        Target;
    
}WorkModeDef;

typedef struct
{
    unsigned int   UpdateTimeOut;
    unsigned short PackNumber;
    unsigned char  UpdateBuffer[300];
}IAPUpdateSysDef;

typedef struct
{
    float          AngleCorrectionValue;
    
    float          Longitude;
    float          Latitude;
    float          TimeZone;
    float          EWTracingAccuracy;
    
    unsigned short ComID;
    unsigned short ChannelEnable;
    
    float          MotorOverCurrentValue;
    
    unsigned short PowerMode;
    
    float          PVModuleWidth;
    float          PostSpacing;
    float          TerrainSlope;           /* East is high and the angle is positive */
    
    unsigned short UpWindSpeed;            /* unit m/s */
    unsigned short UpStartTime;            /* unit s */
    unsigned short LowWindSpeed;           /* unit m/s*/
    unsigned short LowStopTime;            /* unit s */
    
    float          ESoftLimitAngle;
    float          WSoftLimitAngle;
    unsigned short BackAngle;
    unsigned short EWindProtectionAngle;
    unsigned short WWindProtectionAngle;
    unsigned short CleaningDockAngle;
    
}ConfigParaDef;

typedef struct
{
    float        AIRemoteAngle;
    unsigned int AIModeDelayCount;
}AIParaDef;
typedef struct
{
    unsigned short DeviceEdition;
    unsigned short DeviceType;
    
}FixedParaDef;

typedef struct 
{
    unsigned ConfigParaWrite :1;
    unsigned TimeWrite       :1;
    unsigned UpdatePackWrite :1;

}WriteFlagDef;


/***********************************************************************************************************************
                                            Global define
***********************************************************************************************************************/
typedef struct
{
    /* Parameter area */
    ConfigParaDef      ConfigPara;
    WorkModeDef        WorkMode;
    AIParaDef          AIPara;
    TmpDef             Tmp;
    TimeDef            Time;
    WeatherDef         Weather;
    IAPUpdateSysDef    IAPUpdateSys;
    FixedParaDef       FixePara;
    MotorDef           Motor[1];
    PowerParaDef       PowerPara;
    AstronomyParaDef   AstronomyPara;
    WarningAndFaultDef WarningAndFault;
    
    WriteFlagDef       WriteFlag;
    
    /* Parameter update buffer */
    ConfigParaDef      ConfigParaBuffer;
    TimeDef            TimeBuffer;
    WorkModeDef        WorkModeBuffer;
    
}GlobalVariableDef;

#endif

