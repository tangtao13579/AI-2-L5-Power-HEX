#ifndef _ANGLE_SENSOR_H_
#define _ANGLE_SENSOR_H_

int   AngleSensorInit(void);
int   GetAngle(unsigned char AngleSensorID, float * angle);
int   AngleSensorLowPowerMode(unsigned char AngleSensorID);
int   AngleSensorNormalMode(unsigned char AngleSensorIDvoid);

#endif
