#include "GlobalDefine.h"
#include "HWProtocol.h"
#include "stm32f10x.h"
#include "core_cm3.h"
#include "BackupRegister.h"

extern GlobalVariableDef GlobalVariable;
/***************************************************************************************************
                                     Private Variable
***************************************************************************************************/
/*R-0, RW-1*/
unsigned short HWRegister=0;
static void (*RWRegisterHW[25])(unsigned char R_or_RW, unsigned short *value)={(void *)0};
/***************************************************************************************************
                                Jump function
***************************************************************************************************/
static void HWNULL(unsigned char R_or_RW, unsigned short *value );
static void RDeviceEdition1_HW0(unsigned char R_or_RW, unsigned short *value );		//0：版本1
static void RDeviceEdition2_HW1(unsigned char R_or_RW, unsigned short *value );		//1：版本2
static void RDeviceType_HW2(unsigned char R_or_RW, unsigned short *value);			//2：类型
static void RWNum_HW3(unsigned char R_or_RW, unsigned short *value );				//3：支架数量
static void RWLongitude_HW4(unsigned char R_or_RW, unsigned short *value );			//4：经度
static void RWLatitude_HW5(unsigned char R_or_RW, unsigned short *value);			//5：纬度
static void RWWorkMode_HW6(unsigned char R_or_RW, unsigned short *value );			//6：工作模式
static void RWRemoteWindSpeed_HW7(unsigned char R_or_RW, unsigned short *value);	//7：风速
static void RWTimeYear_HW10(unsigned char R_or_RW, unsigned short *value);			//10：年
static void RWTimeMonth_HW11(unsigned char R_or_RW, unsigned short *value);			//11：月
static void RWTimeDay_HW12(unsigned char R_or_RW, unsigned short *value);			//12：日
static void RWTimeHour_HW13(unsigned char R_or_RW, unsigned short *value);			//13：时
static void RWTimeMinute_HW14(unsigned char R_or_RW, unsigned short *value);		//14：分
static void RWTimeSecond_HW15(unsigned char R_or_RW, unsigned short *value);		//15：秒																		//8：风向
static void RTargetAngle_HW17(unsigned char R_or_RW, unsigned short *value);		//17：目标角度
static void RWarningAndFault1_HW20(unsigned char R_or_RW, unsigned short *value );	//20：支架状态
static void RWarningAndFault2_HW21(unsigned char R_or_RW, unsigned short *value );	//21：支架故障																				//18：起停控制
static void RRealAngle_HW23(unsigned char R_or_RW, unsigned short *value);			//23：支架转角


/***************************************************************************************************
                                Jump table
***************************************************************************************************/
void HWProtocolInit()
{
    RWRegisterHW[0]   = RDeviceEdition1_HW0;
    RWRegisterHW[1]   = RDeviceEdition2_HW1;
    RWRegisterHW[2]   = RDeviceType_HW2;
    RWRegisterHW[3]   = RWNum_HW3;
    RWRegisterHW[4]   = RWLongitude_HW4;
    RWRegisterHW[5]   = RWLatitude_HW5;
    RWRegisterHW[6]   = RWWorkMode_HW6;
    RWRegisterHW[7]   = RWRemoteWindSpeed_HW7;
    RWRegisterHW[8]   = HWNULL;
    RWRegisterHW[9]   = HWNULL;
    RWRegisterHW[10]  = RWTimeYear_HW10;    
    RWRegisterHW[11]  = RWTimeMonth_HW11;
    RWRegisterHW[12]  = RWTimeDay_HW12;
    RWRegisterHW[13]  = RWTimeHour_HW13;
    RWRegisterHW[14]  = RWTimeMinute_HW14;
	RWRegisterHW[15]  = RWTimeSecond_HW15;
	RWRegisterHW[16]  = HWNULL;
    RWRegisterHW[17]  = RTargetAngle_HW17;
	RWRegisterHW[18]  = HWNULL;
	RWRegisterHW[19]  = HWNULL;
	RWRegisterHW[20]  = RWarningAndFault1_HW20;
	RWRegisterHW[21]  = RWarningAndFault2_HW21;
    RWRegisterHW[23]  = RRealAngle_HW23;
	RWRegisterHW[24]  = HWNULL;
}

void *GetHWProtocl()
{
    return RWRegisterHW;
}

static void HWNULL(unsigned char R_or_RW, unsigned short *value )
{
	if(R_or_RW == 0)
    {
        *value = HWRegister;
    }
    else if(R_or_RW == 1)
    {
        HWRegister=*value;
    }
}

static void RDeviceEdition1_HW0(unsigned char R_or_RW, unsigned short *value )
{
    if(R_or_RW == 0)
    {
        *value = ((unsigned short)'V')*0x0100+'0';
    }
}
static void RDeviceEdition2_HW1(unsigned char R_or_RW, unsigned short *value )
{
    if(R_or_RW == 0)
    {
        *value = ((unsigned short)'2')*0x0100+'9';	//0.29
    }
}

static void RDeviceType_HW2(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = GlobalVariable.FixePara.DeviceType;
    }
}
static void RWNum_HW3(unsigned char R_or_RW, unsigned short *value )
{
    if(R_or_RW == 0)
    {
        *value = 1;
    }
}

static void RWLongitude_HW4(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = (unsigned short)((int)(GlobalVariable.ConfigPara.Longitude*100));
    }
    else if(R_or_RW == 1)
    {
        GlobalVariable.ConfigParaBuffer.Longitude = (float)((signed short)(*value))/100.0;
    }
}

static void RWLatitude_HW5(unsigned char R_or_RW, unsigned short *value)
{
	if(R_or_RW == 0)
    {
        *value = (unsigned short)((int)(GlobalVariable.ConfigPara.Latitude*100));
    }
    else if(R_or_RW == 1)
    {
        GlobalVariable.ConfigParaBuffer.Latitude = (float)((signed short)(*value))/100.0;
    }
}

static void RWWorkMode_HW6(unsigned char R_or_RW, unsigned short *value )
{
    if(R_or_RW == 0)
    {
		switch(GlobalVariable.WorkMode.WorkMode)
		{
			case MAINTENANCE_MODE:	//维护
				*value=0x0000;
				break;
			case AUTO_TRACKER_MODE:	//自动跟踪
				*value=0x0001;
				break;
			case MANUAL_EAST_MODE:	//手动向东
			case MANUAL_WEST_MODE:	//手动向西
			case AUTO_AI_MODE:
				*value=0x0002;
				break;
			case AUTO_WIND_MODE:	//大风
				*value=0x0003;
				break;
			case AUTO_SNOW_MODE:	//大雪
				*value=0x0004;
				break;
			case AUTO_RAIN_MODE:	//大雨
				*value=0x0005;
				break;			
			case 0x00:				//停
				*value=0x0006;
				break;
			default:
				*value=0x0000;
				break;
		}
    }
    else if(R_or_RW == 1)
    {
		switch(*value)
		{
			case 0x0000:	//维护
				GlobalVariable.WorkMode.WorkMode=MAINTENANCE_MODE;
				break;
			case 0x0001:	//自动跟踪
				GlobalVariable.WorkMode.WorkMode=AUTO_TRACKER_MODE;
				break;
			case 0x0002:	//手动
				GlobalVariable.WorkMode.WorkMode=AUTO_AI_MODE;			//MANUAL_MODE
				GlobalVariable.AIPara.AIRemoteAngle=GlobalVariable.Motor[0].ActualAngle;
				break;
			case 0x0003:	//大风
				GlobalVariable.WorkMode.WorkMode=AUTO_WIND_MODE;
				break;
			case 0x0004:	//大雪
				GlobalVariable.WorkMode.WorkMode=AUTO_SNOW_MODE;
				break;
			case 0x0005:	//大雨
				GlobalVariable.WorkMode.WorkMode=AUTO_RAIN_MODE;
				break;			
			case 0x0006:	//停
				GlobalVariable.WorkMode.WorkMode=0x00;
				break;
			default:
				GlobalVariable.WorkMode.WorkMode=0x00;
				break;
		}
    }
}

static void RWRemoteWindSpeed_HW7(unsigned char R_or_RW, unsigned short *value)
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

static void RWTimeYear_HW10(unsigned char R_or_RW, unsigned short *value)
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
static void RWTimeMonth_HW11(unsigned char R_or_RW, unsigned short *value)
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
static void RWTimeDay_HW12(unsigned char R_or_RW, unsigned short *value)
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
static void RWTimeHour_HW13(unsigned char R_or_RW, unsigned short *value)
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
static void RWTimeMinute_HW14(unsigned char R_or_RW, unsigned short *value)
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
static void RWTimeSecond_HW15(unsigned char R_or_RW, unsigned short *value)
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

static void RTargetAngle_HW17(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = (unsigned short)((int)(GlobalVariable.WorkMode.Target*100))-9000;
    }
	else if(R_or_RW == 1)
	{
		GlobalVariable.AIPara.AIRemoteAngle=90.0+((float)((short)(*value)))/100.0;
	}
}
static void RWarningAndFault1_HW20(unsigned char R_or_RW, unsigned short *value )
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.WarningAndFault)+1);
    }
}
static void RWarningAndFault2_HW21(unsigned char R_or_RW, unsigned short *value )
{
    if(R_or_RW == 0)
    {
        *value = *(((unsigned short *)&GlobalVariable.WarningAndFault));
    }
}

static void RRealAngle_HW23(unsigned char R_or_RW, unsigned short *value)
{
    if(R_or_RW == 0)
    {
        *value = (unsigned short)((int)(GlobalVariable.Motor[0].ActualAngle*100))-9000;
    }
}



