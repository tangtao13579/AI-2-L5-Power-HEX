#include <math.h>
#include "GlobalDefine.h"
#include "AstronomicalAlgorithms.h"

#define  COMPEN       1
#define  PI           3.1415926
#define  SHRINK_ANGLE 3.0f

#define  Radians(x)   ((x)/180*PI)
#define  Degrees(x)   ((x)/PI*180)

extern GlobalVariableDef GlobalVariable;

/***********************************************************************************************************************
                                                Private functions
***********************************************************************************************************************/
static int GetDayOfYear(int year, int month, int day)
{
    int dayOfYear = day;
    int compensate = -2;

    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        compensate = -1;

    if (month < 3)
    {
        dayOfYear += (month == 1 ? 0 : 31);
    }
    else if (month <= 7)
    {
        dayOfYear += (month - 1) * 30 + month / 2 + compensate;
    }
    else
    {
        dayOfYear += 214 + compensate + (month - 8) * 30 + (month - 7) / 2;
    }

    return dayOfYear;
}

static float Mod(float divided, int divisor)
{
    float result = 0.0;
    int temp = 0;

    if (divided >= 0)
    {
        temp = rint(divided);
        result = (temp % divisor) + (divided - rint(divided));
    }
    else
    {
        result = fabsf(divided);
        temp = rint(result);
        temp = temp % divisor;
        
        if( (temp+(result-rint(result))) == 0 )
        {
            result = 0;
        }
        else
        {
            temp = (int)(result / divisor) + 1;
            result = temp * divisor - fabs(divided);
        }
    }
    return result;
}


void CalcSolarZenithAngleAndAzimuthAngle()
{
    float temp = 0.0;

    int i = 0;
    int dayOfDay = 0;
    int dayOfYearFrom1900 = 0;
    
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;

    float longitude = 0.0;    // 经度
    float latitude = 0.0;     // 纬度
    float timeZone = 0.0;     // 时区
    
    float TW_E = 0.0;
    float TW_F = 0.0;
    float TW_G = 0.0;
    float TW_I = 0.0;
    float TW_J = 0.0;
    float TW_K = 0.0;
    float TW_L = 0.0;
    float TW_M = 0.0;

    float TW_P = 0.0;
    float TW_Q = 0.0;
    float TW_R = 0.0;

    float TW_T = 0.0;
    float TW_U = 0.0;
    float TW_V = 0.0;
    float TW_W = 0.0;
    float TW_X = 0.0;
    float TW_Y = 0.0;
    float TW_Z = 0.0;
    
    float TW_AB = 0.0;
    float TW_AC = 0.0;
    float TW_AD = 0.0;
    float TW_AE = 0.0;
    float TW_AE_POW  = 0.0;
    float TW_AE_POW3 = 0.0;
    float TW_AE_POW5 = 0.0;
    float TW_AF = 0.0;
    float TW_AG = 0.0;
    float TW_AH = 0.0;

    //==========================================================================
    year = GlobalVariable.Time.Year;
    month = GlobalVariable.Time.Month;
    day = GlobalVariable.Time.Day;

    dayOfDay = GetDayOfYear(year, month, day);

    for (i = 1900; i < year; i++)
    {
        dayOfYearFrom1900 += (i % 400 == 0 ? 366 : (i % 4 == 0 ? (i % 100 != 0 ? 366 : 365) : 365));//GetDayOfYear(i, 12, 31);
    }
    dayOfYearFrom1900 += dayOfDay;

    //======================================================================================
    longitude = GlobalVariable.ConfigPara.Longitude;    // 经度
    latitude = GlobalVariable.ConfigPara.Latitude;      // 纬度
    timeZone = GlobalVariable.ConfigPara.TimeZone;      // 时区
    
    hour = GlobalVariable.Time.Hour;
    minute = GlobalVariable.Time.Minute;
    second = GlobalVariable.Time.Second;

    //======================================================================================
    TW_E = second;
    TW_E = TW_E / 60.0 + minute;
    TW_E = (TW_E / 60.0 + hour) / 24;

    TW_F = dayOfYearFrom1900 + COMPEN;
    TW_F += 2415018.5 + TW_E - timeZone / 24;

    TW_G = (TW_F - 2451545) / 36525;
    
    TW_I = 280.46646 + TW_G * (36000.76983 + TW_G * 0.0003032);
    TW_I = Mod(TW_I, 360);

    TW_J = 357.52911 + TW_G * (35999.05029 - 0.0001537 * TW_G);

    TW_K = 0.016708634 - TW_G * (0.000042037 + 0.0000001267 * TW_G);
    
    TW_L = sinf(Radians(TW_J))*(1.914602-TW_G*(0.004817+0.000014*TW_G))
        + sinf(Radians(2 * TW_J)) * (0.019993 - 0.000101 * TW_G) + sinf(Radians(3 * TW_J)) * 0.000289;

    TW_M = TW_I + TW_L;


    temp = 125.04 - 1934.136 * TW_G;

    TW_P = TW_M - 0.00569 - 0.00478 * sinf(Radians(temp)); 

    TW_Q = 23 + (26 + ((21.448 - TW_G * (46.815 + TW_G * (0.00059 - TW_G * 0.001813)))) / 60) / 60;

    TW_R = TW_Q + 0.00256 * cosf(Radians(temp)); 

    TW_T = Degrees(asinf(sinf(Radians(TW_R)) * sinf(Radians(TW_P))));

    TW_U = tanf(Radians(TW_R / 2)) * tanf(Radians(TW_R / 2));

    temp = TW_U * sinf(2 * Radians(TW_I)) - 2 * TW_K * sinf(Radians(TW_J))
        + 4 * TW_K * TW_U * sinf(Radians(TW_J)) * cosf(2 * Radians(TW_I))
        - 0.5 * TW_U * TW_U * sinf(4 * Radians(TW_I))
        - 1.25 * TW_K * TW_K * sinf(2 * Radians(TW_J));

    TW_V = 4 * Degrees(temp); 

    TW_W = Degrees(acosf(cosf(Radians(90.833))/(cosf(Radians(latitude))*cosf(Radians(TW_T)))
        -tanf(Radians(latitude))*tanf(Radians(TW_T))));
            
    TW_X = (720 - 4 * longitude - TW_V + timeZone * 60) / 1440;

    // 日出时间
    TW_Y = TW_X - TW_W * 4 / 1440;

    // 日落时间
    TW_Z = TW_X + TW_W * 4 / 1440;
    

    TW_AB = TW_E * 1440 + TW_V + 4 * longitude - 60 * timeZone;
    TW_AB = Mod(TW_AB, 1440);

    TW_AC = (TW_AB < 0) ? (TW_AB / 4 - 180) : (TW_AB / 4 - 180);

    TW_AD = Degrees(acosf(sinf(Radians(latitude)) * sinf(Radians(TW_T))
        + cosf(Radians(latitude)) * cosf(Radians(TW_T)) * cosf(Radians(TW_AC))));

    TW_AE = 90 - TW_AD;
    TW_AE_POW = tanf(Radians(TW_AE));
    TW_AE_POW3 = TW_AE_POW * TW_AE_POW * TW_AE_POW;
    TW_AE_POW5 = TW_AE_POW3 * TW_AE_POW * TW_AE_POW;
    
    TW_AF = ((TW_AE > 85) ? 0 : (TW_AE > 5 ? 58.1 / tanf(Radians(TW_AE)) - 0.07 / TW_AE_POW3
        + 0.000086 / TW_AE_POW5 : (TW_AE > -0.575 ? 1735 + TW_AE * (-518.2 + TW_AE * (103.4 + TW_AE * (-12.79 + TW_AE * 0.711)))
        : -20.772 / tanf(Radians(TW_AE))))) / 3600;

    // 太阳高度角(修正值)
    TW_AG = TW_AE + TW_AF;

    // 太阳方位角
    TW_AH = (TW_AC>0?Mod(Degrees(acosf((sinf(Radians(latitude))*cosf(Radians(TW_AD))-sinf(Radians(TW_T)))
        /(cosf(Radians(latitude))*sinf(Radians(TW_AD)))))+180,360):
        Mod(540-Degrees(acosf((sinf(Radians(latitude))*cosf(Radians(TW_AD))-sinf(Radians(TW_T)))
        /(cosf(Radians(latitude))*sinf(Radians(TW_AD))))),360));

    GlobalVariable.AstronomyPara.SunRiseTime = (float)(TW_Y * 24);    // 日出时间
    GlobalVariable.AstronomyPara.SunSetTime = (float)(TW_Z * 24);     // 日落时间

    GlobalVariable.AstronomyPara.AzimuthAngle = (float)TW_AH;     // 太阳方位角
    
    if(TW_AG >= 89.0)
    {
        GlobalVariable.AstronomyPara.ElevationAngle = 89.0;
    }
    else if(TW_AG < - 89.0)
    {
        GlobalVariable.AstronomyPara.ElevationAngle = -89.0;
    }
    else
    {
        GlobalVariable.AstronomyPara.ElevationAngle = (float)TW_AG;   // 太阳高度角
    } 
}

float GetAstronomicalTargetAngle()
{
    float vertical_component_angle = 0.0;
    
    if(GlobalVariable.AstronomyPara.ElevationAngle >= 0.1) /* Daytime */
    {
        vertical_component_angle = atan2f(tanf(Radians(GlobalVariable.AstronomyPara.ElevationAngle)),\
                                          cosf(Radians(GlobalVariable.AstronomyPara.AzimuthAngle - 90)));
        vertical_component_angle = Degrees(vertical_component_angle);
        
        if(GlobalVariable.AstronomyPara.AzimuthAngle <= 180.0) /* Morning */
        {
            if(vertical_component_angle > GlobalVariable.ConfigPara.TerrainSlope
            && vertical_component_angle > SHRINK_ANGLE)
            {
                if(GlobalVariable.ConfigPara.PostSpacing * sinf(Radians(vertical_component_angle - GlobalVariable.ConfigPara.TerrainSlope))
                    >= GlobalVariable.ConfigPara.PVModuleWidth)
                {
                    return vertical_component_angle;
                }
                else /* Inv tracking */
                {
                    return (vertical_component_angle + 90 - Degrees(asinf(GlobalVariable.ConfigPara.PostSpacing
                                                                  * sinf(Radians(vertical_component_angle - GlobalVariable.ConfigPara.TerrainSlope))
                                                                  / GlobalVariable.ConfigPara.PVModuleWidth)));
                }
            }
            else
            {
                return GlobalVariable.ConfigPara.BackAngle;
            }
        }
        else /* Afternoon */
        {
            if(vertical_component_angle < (180 + GlobalVariable.ConfigPara.TerrainSlope)
            && vertical_component_angle < (180 - SHRINK_ANGLE))
            {
                if(sinf(Radians(180 - vertical_component_angle + GlobalVariable.ConfigPara.TerrainSlope )) >= (GlobalVariable.ConfigPara.PVModuleWidth/GlobalVariable.ConfigPara.PostSpacing))
                {
                    return vertical_component_angle;
                }
                else /* Inv tracking */
                {
                    return (vertical_component_angle - 90 + Degrees(asinf(GlobalVariable.ConfigPara.PostSpacing
                                                          * sinf(Radians(vertical_component_angle - GlobalVariable.ConfigPara.TerrainSlope))
                                                          / GlobalVariable.ConfigPara.PVModuleWidth)));
                }
            }
            else
            {
                return GlobalVariable.ConfigPara.BackAngle;
            }
        }
    }
    else /* Night */
    {
        return GlobalVariable.ConfigPara.BackAngle;
    }
}
