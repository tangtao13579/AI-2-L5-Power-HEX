#include "BatteryPower.h"
#include "PVPower.h"
#include "GlobalDefine.h"
#include "GlobalOs.h"

#define TASK_POWER_PERIOD 100 /*unit ms*/


extern GlobalVariableDef GlobalVariable;

static BatteryChargeStateDef ChargeState = {0};
static unsigned char is_heating = 0;
/***************************************************************************************************
                                Private functions
***************************************************************************************************/
static void GetBatteryInfo(void);
static void ChargeCmd(int StartCharge);

static void GetBatteryInfo()
{
    int return_value;
    static unsigned char  count = 0;
    static unsigned short soclow_delay_count = 0;
    static unsigned short sochigh_delay_count = 0;
    static unsigned short battery_error_clear_delay_count = 0;
    static unsigned short battery_error_delay_count = 0;
/*Get Battery info*/
    return_value = 0;
    return_value += GetBatteryVoltage(&GlobalVariable.PowerPara.BatteryVoltage);
    return_value += GetBatteryCurrent(&GlobalVariable.PowerPara.BatteryCurrent);
    return_value += GetBatteryTemp(&GlobalVariable.PowerPara.BatteryTemperature);
    return_value += GetBatteryChargerState(&ChargeState);
    
/* Check Battery Error */
    /** Unable to communicate with battery: BatNoCom **/
    if(return_value == 0)
    {
        count = 0;
        GlobalVariable.WarningAndFault.BatNoCom = 0;
    }
    else
    {
        if(count ++ > 5)
        {
           count = 5;
           GlobalVariable.WarningAndFault.BatNoCom = 1;
           GlobalVariable.PowerPara.BatteryVoltage = 0;
           GlobalVariable.PowerPara.BatteryCurrent = 0;
        }
    }
    /**Battery Error: 1.BatteryVotageLow, 2.BatMissing, 3.BatShort**/
    if((ChargeState.BatMissingFault != 0)
     ||(ChargeState.BatShortFault != 0)
     ||(GlobalVariable.PowerPara.BatteryVoltage <= 22.0f))
    {
        battery_error_clear_delay_count = 0;
        battery_error_delay_count++;
        if(battery_error_delay_count >= 2 * 1000/TASK_POWER_PERIOD)/*force restart*/
        {
            battery_error_delay_count = 0;
            GlobalVariable.WarningAndFault.BatError = 1;
        }
    }
    else
    {
        if(GlobalVariable.PowerPara.BatteryVoltage >= 23.5f)
        {
            battery_error_delay_count = 0;
            battery_error_clear_delay_count++;
            if(battery_error_clear_delay_count >= 10 * 1000/TASK_POWER_PERIOD) /* 10s */
            {
                battery_error_clear_delay_count = 0;
                GlobalVariable.WarningAndFault.BatError = 0;
            }
        }
    }
    
/* Check Battery capacity */
	if(GlobalVariable.Motor[0].MotorRunningState == 0)
	{
		if(GlobalVariable.PowerPara.BatteryVoltage <= 24.5f)
		{
			sochigh_delay_count = 0;
			soclow_delay_count++;
			if(soclow_delay_count >= 60*1000/TASK_POWER_PERIOD)/*30s*/
			{
				soclow_delay_count = 0;
				GlobalVariable.WarningAndFault.BatSOCLow = 1;
			}
		}
		else if(GlobalVariable.PowerPara.BatteryVoltage >= 26.5f)
		{
			soclow_delay_count = 0;
			sochigh_delay_count++;
			if(sochigh_delay_count >= 30*1000/TASK_POWER_PERIOD)/*30s*/
			{
				sochigh_delay_count = 0;
				GlobalVariable.WarningAndFault.BatSOCLow = 0;
			}
		}
		else
		{
			sochigh_delay_count = 0;
			soclow_delay_count = 0;
		}
	}

/* Calculate battery SOC */
    GlobalVariable.PowerPara.BatterySOC = GetBatterySOC(GlobalVariable.PowerPara.BatteryVoltage);
}

static void ChargeCmd(int StartCharge)
{
    static unsigned char heating_state = 0;
    
/*Suspend charger*/
    if(StartCharge != 1) 
    {
        if(ChargeState.ChargerSuspend == 0)
        {
            ChargerSuspend();
        }
        HeatingCmd(0);
        is_heating = 0;
        
        return;
    }
    
/*Charge*/
    switch(heating_state)
    {
        case 0: /*Determine if heating is needed*/
            if(GlobalVariable.PowerPara.BatteryTemperature <= 5.0f)
            {
                if(GlobalVariable.PowerPara.PVBuckerVoltage >= 30.0f)/*pvbuck good*/
                {
                    heating_state = 1;  /*heat + no charge*/
                }
                else
                {
                    heating_state = 3; /*no heat + no charge*/
                }
            }
            else
            {
                heating_state = 2;  /*No heat + charge*/
            }
            break;
        case 1:  /*Need heating*/
            if(ChargeState.ChargerSuspend == 0)
            {
                ChargerSuspend();
            }
            if(GlobalVariable.PowerPara.BatteryTemperature >= 8.0f
            || GlobalVariable.PowerPara.PVBuckerVoltage <= 28.0f)
            {
                HeatingCmd(0);
                is_heating = 0;
                heating_state = 0;
            }
            else
            {
                HeatingCmd(1);
                is_heating = 1;
                
                heating_state = 1;
            }
            break;
        case 2:   /*No heating required*/
            HeatingCmd(0);
            is_heating = 0;
        
            if(ChargeState.ChargerSuspend == 1)
            {
                ChargerActive();
            }
            if(ChargeState.MaxChargeTimeFault == 1)
            {
                ChargerReset();
            }
            
            heating_state = 0;
            break;
        case 3:
            HeatingCmd(0);
            is_heating = 0;
        
            if(ChargeState.ChargerSuspend == 0)
            {
                ChargerSuspend();
            }
            heating_state = 0;
            break;
        default:
            heating_state = 0;
        break;
    }
    
}
/***********************************************************************************************************************
                                                    Public functions
***********************************************************************************************************************/
void PowerInit()
{
    BatteryPowerInit();
    PVPowerInit();
    
    /*The alarm is set to 1 before power-on to battery detection is completed, 
    in order to prevent the motor from rotating during this period of time.*/
    GlobalVariable.WarningAndFault.BatError = 1;
    
    GlobalVariable.PowerPara.BatteryTemperature = 25.0f;
}

/*100ms*/
void PowerMangement()
{
	  OS_ERR err;
	  static unsigned char charge_delay_count = 0;
	
/* Get and update battery information  */
    GetBatteryInfo();

/*Upadate charge state*/
    if((is_heating == 1) && ((ChargeState.ChargerSuspend == 1) || (ChargeState.Term == 1)))
    {                                     
        GlobalVariable.PowerPara.ChargeState.Heating = 1;
        GlobalVariable.PowerPara.ChargeState.Term = 0;
        
        GlobalVariable.PowerPara.ChargeState.CCCV = 0;
        GlobalVariable.PowerPara.ChargeState.PreCharge = 0;
    }
    else
    {
        GlobalVariable.PowerPara.ChargeState.Heating = 0;
        GlobalVariable.PowerPara.ChargeState.CCCV = ChargeState.CCCV;
        GlobalVariable.PowerPara.ChargeState.PreCharge = ChargeState.PreCharge;
        if((ChargeState.ChargerSuspend == 1) || (ChargeState.Term == 1))
        {
            GlobalVariable.PowerPara.ChargeState.Term = 1;
        }
        else
        {
            GlobalVariable.PowerPara.ChargeState.Term = 0;
        }
    }
    GlobalVariable.PowerPara.ChargeState.BatMissingFault = ChargeState.BatMissingFault;
    GlobalVariable.PowerPara.ChargeState.BatShortFault = ChargeState.BatShortFault;
    GlobalVariable.PowerPara.ChargeState.MaxChargeTimeFault = ChargeState.MaxChargeTimeFault;
    
/* Battery charging and heating */
    if(GlobalVariable.WarningAndFault.BatError == 1)
    {
        charge_delay_count = 0;
        OpenPVPower();
        if(GlobalVariable.PowerPara.PVBuckerVoltage >= 30.0f)
        {
            ChargeCmd(1);
        }
        else
        {
            ChargeCmd(0);
        }
        return;
    }
    
    if((GlobalVariable.Time.TodayTime >= (GlobalVariable.AstronomyPara.SunRiseTime - 2.0f))
     &&(GlobalVariable.Time.TodayTime <= (GlobalVariable.AstronomyPara.SunSetTime + 2.0f))
     &&(GlobalVariable.Motor[0].MotorControlDir == 0))
    {
        charge_delay_count++;
        if(charge_delay_count >= 2 * 1000 / TASK_POWER_PERIOD)
        {
            charge_delay_count = 0;
            OpenPVPower();
            if(GlobalVariable.PowerPara.PVBuckerVoltage >= 30.0f)
            {
                ChargeCmd(1);
            }
            else
            {
                ChargeCmd(0);
            }
        }
    }
    else
    {
        charge_delay_count = 0;
        ChargeCmd(0);
    }
		
		OSFlagPend((OS_FLAG_GRP*)&T0AngleNoChange_first_Flags,
				       (OS_FLAGS	)T0AngleNoChange_first_step1_FLAG,
		     	     (OS_TICK     )0,
				       (OS_OPT	    )OS_OPT_PEND_FLAG_SET_ALL+OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_NON_BLOCKING,
				       (CPU_TS*     )0,
				       (OS_ERR*	    )&err);
		if(err == OS_ERR_NONE)
		{
			  if(GlobalVariable.WarningAndFault.BatError ==0)
				{
			      ClosePVPower();                                         //switch to BAT
			  }
			  OSFlagPost((OS_FLAG_GRP*)&T0AngleNoChange_first_Flags,      //post a event flag step2
								   (OS_FLAGS	  )T0AngleNoChange_first_step2_FLAG,
								   (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					         (OS_ERR*	  )&err);
		}
		OSFlagPend((OS_FLAG_GRP*)&T0AngleNoChange_first_Flags,
				       (OS_FLAGS	)T0AngleNoChange_first_step3_FLAG,
		     	     (OS_TICK     )0,
				       (OS_OPT	    )OS_OPT_PEND_FLAG_SET_ALL+OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_NON_BLOCKING,
				       (CPU_TS*     )0,
				       (OS_ERR*	    )&err);
		if(err == OS_ERR_NONE)
		{
			  OpenPVPower();                                              //switch to PV
		}
		OSFlagPend((OS_FLAG_GRP*)&Motor_continuous_Flags,
				       (OS_FLAGS	)Motor_continuous_step1_FLAG,
		     	     (OS_TICK     )0,
				       (OS_OPT	    )OS_OPT_PEND_FLAG_SET_ALL+OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_NON_BLOCKING,
				       (CPU_TS*     )0,
				       (OS_ERR*	    )&err);
		if(err == OS_ERR_NONE)
		{
			  if(GlobalVariable.WarningAndFault.BatError ==0)
				{
			      ClosePVPower();                                         //switch to BAT
			  }
			  OSFlagPost((OS_FLAG_GRP*)&Motor_continuous_Flags,           //post a event flag step2
								   (OS_FLAGS	  )Motor_continuous_step2_FLAG,
								   (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					         (OS_ERR*	  )&err);
		}
		OSFlagPend((OS_FLAG_GRP*)&Motor_continuous_Flags,
				       (OS_FLAGS	)Motor_continuous_step3_FLAG,
		     	     (OS_TICK     )0,
				       (OS_OPT	    )OS_OPT_PEND_FLAG_SET_ALL+OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_NON_BLOCKING,
				       (CPU_TS*     )0,
				       (OS_ERR*	    )&err);
		if(err == OS_ERR_NONE)
		{
			  OpenPVPower();                                              //switch to PV
		}
}

