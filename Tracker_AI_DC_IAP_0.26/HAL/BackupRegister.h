#ifndef _BACKUP_REGISTER_H_
#define _BACKUP_REGISTER_H_

void BKPInit(void);
int  BKPReadRTCUpdateFlag(void);
void BKPWriteRTCUpdateFlag(void);
#endif
