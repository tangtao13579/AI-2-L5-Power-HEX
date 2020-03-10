#ifndef _ADC_SAMPLE_H_
#define _ADC_SAMPLE_H_

void  ADCInit(void);
void  ADCConvert(void);
float GetMotorCurrent(void);
float GetPVBuckerVoltage(void);
float GetPVStringUpCur(void);
float GetPVStringDownCur(void);
float GetEmergencyVoltage(void);

#endif
