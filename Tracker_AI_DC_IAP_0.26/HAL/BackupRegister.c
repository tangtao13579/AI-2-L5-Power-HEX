#include "stm32f10x_bkp.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rcc.h"

/***********************************************************************************************************************
                            Public Functions
***********************************************************************************************************************/
void BKPInit()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    BKP_TamperPinCmd(DISABLE);
    BKP_ITConfig(DISABLE);
    BKP_ClearFlag();
    PWR_BackupAccessCmd(DISABLE);
}

int BKPReadRTCUpdateFlag()
{
    if(BKP_ReadBackupRegister(BKP_DR1) == 0xAAAA)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

void BKPWriteRTCUpdateFlag()
{
    PWR_BackupAccessCmd(ENABLE);
    BKP_WriteBackupRegister(BKP_DR1,0xAAAA);
    PWR_BackupAccessCmd(DISABLE);
}
