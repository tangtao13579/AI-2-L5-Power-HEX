#ifndef _POWERCONTROL_H_
#define _POWERCONTROL_H_

typedef struct
{
    unsigned char ChargerSuspend     :1;
    unsigned char PreCharge          :1;
    unsigned char CCCV               :1;
    unsigned char Term               :1;
    
    unsigned char MaxChargeTimeFault :1;
    unsigned char BatMissingFault    :1;
    unsigned char BatShortFault      :1;
    unsigned char Reserved           :1;
    
}BatteryChargeStateDef;

int   BatteryPowerInit(void);
void  ChargerSuspend(void);
void  ChargerActive(void);
void  ChargerReset(void);
int   GetBatteryChargerState(BatteryChargeStateDef *charge_state);
int   GetBatteryVoltage(float * Vbat);
int   GetBatteryCurrent(float *Ibat);
float GetBatterySOC(float Vbat);
void  HeatingCmd(int isOpen);
int   GetBatteryTemp(float *Temp);
#endif

