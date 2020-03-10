#ifndef _MOTOR_CONTROL_H_
#define _MOTOR_CONTROL_H_

#define MAX_MOTOR_NUM 1
void MotorInit(void);
void MotorTurnEast(unsigned char motor_ID);
void MotorTurnWest(unsigned char motor_ID);
void MotorAllStop(void);
void MotorStop(unsigned char motor_ID);
unsigned char GetMotorRunningState(unsigned char motor_ID);
unsigned char GetMotorDirState(unsigned char motor_ID);
#endif
