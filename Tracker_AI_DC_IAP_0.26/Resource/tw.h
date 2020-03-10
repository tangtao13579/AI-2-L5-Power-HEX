#ifndef  __TW_H
#define  __TW_H

#include "global_define.h"

#define  PODU

#define  PI   3.1415926

#define  Radians(x)   ((x)/180*PI)
#define  Degrees(x)   ((x)/PI*180)
#define  Abs(x)       ((x)>0?(x):((x)*(-1)))

#define  COMPEN       1


#define  INV_T_SHRINK_ANGLE        10.0     // Äæ¸ú×ÙÊÕËõ½Ç¶È




int GetDayOfYear(int year, int month, int day);
double Mod(double divided, int divisor);
void CalcSolarZenithAngleAndAzimuthAngle(Solar *solar, TIME *time);
float GetTargetAngle(Solar solar, AArray arr, float inAngle);

#endif // __TW_H

