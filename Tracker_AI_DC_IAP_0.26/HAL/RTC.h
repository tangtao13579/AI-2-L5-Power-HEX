#ifndef _RTC_H_
#define _RTC_H_

void RTCInit(void);
void GetTime(unsigned short *second,unsigned short *minute,unsigned short *hour,unsigned short *day,unsigned short *month,unsigned short *year);             
void SetTime(unsigned short second, unsigned short minute, unsigned short hour,unsigned short day,unsigned short month,unsigned short year);          
#endif
