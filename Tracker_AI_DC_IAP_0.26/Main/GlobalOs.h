#ifndef _GLOBAL_OS_H_ 
#define _GLOBAL_OS_H_

#include "os.h"
/***************************************************************************************************
                                    OSSEM
***************************************************************************************************/
#define Power_switch_FLAGS_VALUE   0X00		
#define Power_switch_PV_FLAG       0x01
#define Power_switch_BAT_FLAG      0x02
#define Power_switch_clear_FLAG    0x03
extern OS_FLAG_GRP	Power_switch_Flags;  //create a event flag

#endif
