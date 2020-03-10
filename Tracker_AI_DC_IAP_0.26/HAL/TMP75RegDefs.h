#ifndef _TMP75_REG_DEFS_H_
#define _TMP75_REG_DEFS_H_

#define TMP75_ADDR 0x92

#define TMP75_TMP_REG_ADDR 0x00

/*
D7:OneShot  -0
D6:R1       -1
D5:R0       -1
D4:F1       -1
D3:F0       -1
D2:POL      -0
D1:TM       -1
D0:ShutDown -0
*/
#define TMP75_CONFIG_REG_ADDR  0x01
#define TMP75_CONFIG_REG_VALUE 0x7A


#define TMP75_TLOW_REG_ADDR  0x02
#define TMP75_THIGH_REG_ADDR 0x03


#endif
