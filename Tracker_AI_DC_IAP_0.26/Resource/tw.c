#include "math.h"

#include "tw.h"



int GetDayOfYear(int year, int month, int day)
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

double Mod(double divided, int divisor)
{
    double result = 0.0;
    int temp = 0;

    if (divided >= 0)
    {
        temp = rint(divided);
        result = (temp % divisor) + (divided - rint(divided));
    }
    else
    {
	result = Abs(divided);
	temp = rint(result);
	temp = temp % divisor;
	
	if( (temp+(result-rint(result))) == 0 )
	{
	    result = 0;
	}
	else
	{
	    temp = (int)(result / divisor) + 1;
	    result = temp * divisor - Abs(divided);
	}
    }
    return result;
}


void CalcSolarZenithAngleAndAzimuthAngle(Solar *solar, TIME *time)
{
    double temp = 0.0;

    int i = 0;
    int dayOfDay = 0;
    int dayOfYearFrom1900 = 0;
    
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;

    double longitude = 0.0;    // 经度
    double latitude = 0.0;     // 纬度
    double timeZone = 0.0;     // 时区
    
    double TW_E = 0.0;
    double TW_F = 0.0;
    double TW_G = 0.0;
    double TW_I = 0.0;
    double TW_J = 0.0;
    double TW_K = 0.0;
    double TW_L = 0.0;
    double TW_M = 0.0;
    //double TW_N = 0.0;
    //double TW_O = 0.0;
    double TW_P = 0.0;
    double TW_Q = 0.0;
    double TW_R = 0.0;
    //double TW_S = 0.0;
    double TW_T = 0.0;
    double TW_U = 0.0;
    double TW_V = 0.0;
    double TW_W = 0.0;
    double TW_X = 0.0;
    double TW_Y = 0.0;
    double TW_Z = 0.0;
    
    //double TW_AA = 0.0;
    double TW_AB = 0.0;
    double TW_AC = 0.0;
    double TW_AD = 0.0;
    double TW_AE = 0.0;
    double TW_AF = 0.0;
    double TW_AG = 0.0;
    double TW_AH = 0.0;

    //==========================================================================
    year = time->Year;
    //year += 2000;
    month = time->Month;
    day = time->Day;

    dayOfDay = GetDayOfYear(year, month, day);

    for (i = 1900; i < year; i++)
    {
        dayOfYearFrom1900 += (i % 400 == 0 ? 366 : (i % 4 == 0 ? (i % 100 != 0 ? 366 : 365) : 365));//GetDayOfYear(i, 12, 31);
    }
    dayOfYearFrom1900 += dayOfDay;

    //======================================================================================
    longitude = solar->Longitude;    // 经度
    latitude = solar->Latitude;      // 纬度
    timeZone = solar->TimeZone;      // 时区
    hour = time->Hour;;
    minute = time->Minute;
    second = time->Second;

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
    
    TW_L = sin(Radians(TW_J))*(1.914602-TW_G*(0.004817+0.000014*TW_G))
        + sin(Radians(2 * TW_J)) * (0.019993 - 0.000101 * TW_G) + sin(Radians(3 * TW_J)) * 0.000289;

    TW_M = TW_I + TW_L;

    //TW_N = TW_J + TW_L;

    //TW_O = (1.000001018 * (1 - TW_K * TW_K)) / (1 + TW_K * cos(Radians(TW_N)));

    temp = 125.04 - 1934.136 * TW_G;

    TW_P = TW_M - 0.00569 - 0.00478 * sin(Radians(temp)); //125.04 - 1934.136 * TW_G));

    TW_Q = 23 + (26 + ((21.448 - TW_G * (46.815 + TW_G * (0.00059 - TW_G * 0.001813)))) / 60) / 60;

    TW_R = TW_Q + 0.00256 * cos(Radians(temp)); //125.04 - 1934.136 * TW_G));

    //TW_S = Degrees(atan2(cos(Radians(TW_R)) * sin(Radians(TW_P)),cos(Radians(TW_P))));

    TW_T = Degrees(asin(sin(Radians(TW_R)) * sin(Radians(TW_P))));

    TW_U = tan(Radians(TW_R / 2)) * tan(Radians(TW_R / 2));

    temp = TW_U * sin(2 * Radians(TW_I)) - 2 * TW_K * sin(Radians(TW_J))
        + 4 * TW_K * TW_U * sin(Radians(TW_J)) * cos(2 * Radians(TW_I))
        - 0.5 * TW_U * TW_U * sin(4 * Radians(TW_I))
        - 1.25 * TW_K * TW_K * sin(2 * Radians(TW_J));

    TW_V = 4 * Degrees(temp); /*TW_U * sin(2 * Radians(TW_I)) - 2 * TW_K * sin(Radians(TW_J))
        + 4 * TW_K * TW_U * sin(Radians(TW_J)) * cos(2 * Radians(TW_I))
        - 0.5 * TW_U * TW_U * sin(4 * Radians(TW_I))
        - 1.25 * TW_K * TW_K * sin(2 * Radians(TW_J)));*/

    TW_W = Degrees(acos(cos(Radians(90.833))/(cos(Radians(latitude))*cos(Radians(TW_T)))
        -tan(Radians(latitude))*tan(Radians(TW_T))));
            
    TW_X = (720 - 4 * longitude - TW_V + timeZone * 60) / 1440;

    // 日出时间
    TW_Y = TW_X - TW_W * 4 / 1440;

    // 日落时间
    TW_Z = TW_X + TW_W * 4 / 1440;
    
    //TW_AA = 8 * TW_W;

    TW_AB = TW_E * 1440 + TW_V + 4 * longitude - 60 * timeZone;
    TW_AB = Mod(TW_AB, 1440);

    TW_AC = (TW_AB < 0) ? (TW_AB / 4 - 180) : (TW_AB / 4 - 180);

    TW_AD = Degrees(acos(sin(Radians(latitude)) * sin(Radians(TW_T))
        + cos(Radians(latitude)) * cos(Radians(TW_T)) * cos(Radians(TW_AC))));

    TW_AE = 90 - TW_AD;

    TW_AF = ((TW_AE > 85) ? 0 : (TW_AE > 5 ? 58.1 / tan(Radians(TW_AE)) - 0.07 / pow(tan(Radians(TW_AE)), 3)
        + 0.000086 / pow(tan(Radians(TW_AE)), 5) : (TW_AE > -0.575 ? 1735 + TW_AE * (-518.2 + TW_AE * (103.4 + TW_AE * (-12.79 + TW_AE * 0.711)))
        : -20.772 / tan(Radians(TW_AE))))) / 3600;

    // 太阳高度角(修正值)
    TW_AG = TW_AE + TW_AF;

    // 太阳方位角
    TW_AH = (TW_AC>0?Mod(Degrees(acos((sin(Radians(latitude))*cos(Radians(TW_AD))-sin(Radians(TW_T)))
        /(cos(Radians(latitude))*sin(Radians(TW_AD)))))+180,360):
        Mod(540-Degrees(acos((sin(Radians(latitude))*cos(Radians(TW_AD))-sin(Radians(TW_T)))
        /(cos(Radians(latitude))*sin(Radians(TW_AD))))),360));

    solar->SunRise = (float)(TW_Y * 24);    // 日出时间
    solar->SunSet = (float)(TW_Z * 24);     // 日落时间

    solar->AzimuthAngle = (float)TW_AH;     // 太阳方位角
    solar->ElevationAngle = (float)TW_AG;   // 太阳高度角
}

//**************************************************************************************************
// Function Name  : GetMotorRunAngle
// Description    : 
// Input          : 投影计算组件应该去跟踪的角度值。规定正北为方位角度0°，正南为180°。  投影算法计算。不是简单的只跟踪方位角。而是一个公式。
// Output         : None
// Return         : None
//**************************************************************************************************
float GetTargetAngle(Solar solar, AArray arr, float inAngle)
{
    double ewAngle,tempAngle;
    //double tempAA;

    if(solar.AzimuthAngle==180.0 || solar.AzimuthAngle==0.0)   //方位角为180°和0°时我们组件就放平了。
    {
        return 90.0;
    }

    if(solar.AzimuthAngle == 90.0)  //用到了TG90°是无穷大，所以要给个不是90°的值。
    {
        solar.AzimuthAngle = 90.1; 
    }
    else if(solar.AzimuthAngle < 90.0)
    {
        solar.AzimuthAngle = 180.0 - solar.AzimuthAngle; 
    }

    if(solar.AzimuthAngle == 270.0)
    {
        solar.AzimuthAngle = 269.9;
    }
    else if(solar.AzimuthAngle > 270.0)  //对称到另一边了。
    {
        solar.AzimuthAngle = 540.0 - solar.AzimuthAngle;
    }

    if(solar.ElevationAngle<=0)
    {
        solar.ElevationAngle = 0.1;
    }

    if(solar.ElevationAngle>=90)
    {
        solar.ElevationAngle = 89.9;
    }

    if(solar.AzimuthAngle<180.0)
    {
        ewAngle = atan(tan(Radians(solar.ElevationAngle))/cos(Radians(solar.AzimuthAngle - 90.0)));
        ewAngle = Degrees(ewAngle);

        if(arr.DownSlope < ewAngle)
        {
            if( (arr.Spacing*tan(Radians(ewAngle-arr.DownSlope))) >= arr.Width )
            {
                tempAngle = ewAngle;    // 正常
            }
            else
            {   // 逆跟
                //tempAngle = 90.0 - Degrees(asin(arr.Length_LR*(1+tan(Radians(tempAA))/tan(Radians(ewAngle))) / arr.Width * sin(Radians(ewAngle)))) + ewAngle;  
                //tempAngle -= inAngle;
                //tempAngle = Degrees(asin(arr.Length_LR/arr.Width*sin(Radians(ewAngle-arr.PoDu)))) + ewAngle - 90.0;
                tempAngle = 90.0 + ewAngle - Degrees(asin(arr.Spacing/arr.Width*sin(Radians(ewAngle-arr.DownSlope))));
                
                /*以下--增加跟踪缩进 */
                if(tempAngle > 90.0 - INV_T_SHRINK_ANGLE)
                {
                    tempAngle = 90.0;
                }
                /*以上--增加跟踪缩进 */
            }   
        }
        else
        {
            tempAngle = 90.0;
        }                    
    }
    else
    {   
        ewAngle = atan(tan(Radians(solar.ElevationAngle))/cos(Radians(270.0 - solar.AzimuthAngle))); 
        ewAngle = 180 - Degrees(ewAngle);

        if(arr.UpSlope < (180.0-ewAngle))
        {
            if ( (arr.Spacing*sin(Radians(180.0-ewAngle-arr.UpSlope))) >= arr.Width )
            {
                tempAngle = ewAngle;    // 正常
            }
            else
            {   // 逆跟      
                //tempAngle = 90.0 - (180.0-ewAngle) + Degrees(asin(arr.Length_LR*(1-tan(Radians(tempAA))/tan(Radians(ewAngle))) / arr.Width * sin(Radians((180.0-ewAngle)))));
                tempAngle = sin(Radians(180.0 - ewAngle - arr.UpSlope));
                tempAngle = asin(arr.Spacing/arr.Width*tempAngle);
                tempAngle = 180.0 - Degrees(tempAngle); // Degrees(asin(arr.Length_LR/arr.Width*sin(Radians(180.0-ewAngle-arr.Length_FB))));
                tempAngle = 90.0 + ewAngle - tempAngle;
                tempAngle -= inAngle;
                tempAngle -= 1.0;
        
                if(tempAngle<90.0)
                {
                    tempAngle = 90.0;
                }
                
                /*以下--增加跟踪缩进 */
                if(tempAngle < 90.0 + INV_T_SHRINK_ANGLE)
                {
                    tempAngle = 90.0;
                }
                /*以上--增加跟踪缩进 */
            }  
        }
        else
        {
            tempAngle = 90.0;
        }

    }

    return ((float)tempAngle);
}
