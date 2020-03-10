#ifndef _MOTOR_PROTECTION_
#define _MOTOR_PROTECTION_

void DetectOverCurrent(void);
void DetectSoftLimit(void);
void DetectMotorReverseAngleNoChange(void);
void DetectEmergencyStop(void);
#endif
