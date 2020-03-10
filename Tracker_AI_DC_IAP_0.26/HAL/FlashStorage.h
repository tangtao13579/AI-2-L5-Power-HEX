#ifndef _FLASH_STORAGE_H_
#define _FLASH_STORAGE_H_

void WriteWorkModeToFlash(unsigned int WorkMode);
int ReadWorkModeFromFlash(unsigned int *out);

void WriteConfigParaToFlash(unsigned short *data, int datasize);
int  ReadConfigParaFromFlash(unsigned short *data, int datasize);

void EraseNewSystemFlash(void);
void WriteNewSystemToFlash(unsigned short *data, unsigned short pack_number);
void WriteIAPFlagToFlash(unsigned int num_of_halfword);
#endif
