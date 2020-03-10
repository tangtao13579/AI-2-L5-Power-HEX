#ifndef  __WIRELESS_H
#define  __WIRELESS_H

#include "global_define.h"


#define  Wireless485_TX_EN() GPIO_SetBits(GPIOA, GPIO_Pin_8)
#define  Wireless485_RX_EN() GPIO_ResetBits(GPIOA, GPIO_Pin_8)

void Wireless_Configuration(void);
unsigned char Data_Analyze(COMMU *com, unsigned char ID);
unsigned short GetCRC16CheckCode(unsigned char *buf, unsigned short nEnd);

unsigned char Data_transparent(void);


#endif  // __WIRELESS_H
