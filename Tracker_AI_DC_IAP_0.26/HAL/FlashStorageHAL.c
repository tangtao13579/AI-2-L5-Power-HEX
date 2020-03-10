#include "stm32f10x_flash.h"

#define FlASH_PAGE_SIZE        ((uint16_t)0x800)
#define FlASH_START_ADDR       ((uint32_t)0x08000000)

#define NEW_SYSTEM_START_ADDR  ((uint32_t)0x0802A000)
#define NEW_SYSTEM_STOP_ADDR   ((uint32_t)0x08051FFF)

#define CONFIG_PARA_START_ADDR ((uint32_t)0x0807A000)
#define CONFIG_PARA_STOP_ADDR  ((uint32_t)0x0807A7FF)

#define WORKMODE_START_ADDR    ((uint32_t)0x0807A800)
#define WORKMODE_STOP_ADDR     ((uint32_t)0x0807AFFF)

#define IAP_FLAG_ADDR          ((uint32_t)0x0807F000)
#define WRITEFLAG              ((uint16_t)0xAAAA)


/***************************************************************************************************
                            Private functions
***************************************************************************************************/
static int ScanLastWorkModeAddr(uint16_t **Addr);

static int ScanLastWorkModeAddr(uint16_t **Addr)
{
    uint16_t *temp_addr;
    uint16_t *StartAddr = (uint16_t *)WORKMODE_START_ADDR;
    uint16_t *StopAddr  = ((uint16_t *)WORKMODE_STOP_ADDR) - 3;
    
    for(temp_addr = StartAddr; temp_addr < StopAddr; temp_addr += 3)
    {
        if((*temp_addr == WRITEFLAG) && (*(temp_addr + 3) != WRITEFLAG))
        {
            *Addr = temp_addr;
            return 0;
        }
    }
    return -1;
}


/***************************************************************************************************
                                Public functions
***************************************************************************************************/
void WriteWorkModeToFlash(unsigned int WorkMode)
{
    int      temp_return_value;
    uint16_t *temp_addr;
    uint16_t *StartAddr = (uint16_t *)WORKMODE_START_ADDR;
    uint16_t *StopAddr  = ((uint16_t *)WORKMODE_STOP_ADDR) - 6;
    
    temp_return_value = ScanLastWorkModeAddr(&temp_addr);
    temp_addr += 3;
    if((temp_return_value == -1)||(temp_addr >= StopAddr)) /*Erase page*/
    {
        FLASH_Unlock();
        FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
        FLASH_ErasePage(WORKMODE_START_ADDR);
        while(FLASH_GetStatus() == FLASH_BUSY);
        FLASH_Lock();
        
        temp_addr = StartAddr;
    }
    
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
    FLASH_ProgramHalfWord((uint32_t)temp_addr, 0xAAAA);
    temp_addr ++;
    FLASH_ProgramHalfWord((uint32_t)temp_addr, *((unsigned short *)&WorkMode));
    temp_addr ++;
    FLASH_ProgramHalfWord((uint32_t)temp_addr, *(((unsigned short *)&WorkMode)+1));
    while(FLASH_GetStatus() == FLASH_BUSY);
    FLASH_Lock();
}
int ReadWorkModeFromFlash(unsigned int *out)
{
    uint16_t *temp_addr;
    int      temp_return_value;
    
    temp_return_value = ScanLastWorkModeAddr(&temp_addr);
    if(temp_return_value == 0)
    {
        *out = 0x0000FFFF & (*(temp_addr+2));
        *out = *out << 16;
        *out &= 0xFFFF0000;
        *out |= 0x0000FFFF & (*(temp_addr+1));
        return 0;
    }
    return -1;
}
void WriteConfigParaToFlash(unsigned short *data, int datasize)/*datasize unit short*/
{
    int i;
    
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
    FLASH_ErasePage(CONFIG_PARA_START_ADDR);
    while(FLASH_GetStatus() == FLASH_BUSY);
    FLASH_ProgramHalfWord((uint32_t)CONFIG_PARA_START_ADDR, 0xAAAA);
    for(i = 0; i < datasize; i++)
    {
        FLASH_ProgramHalfWord((uint32_t)CONFIG_PARA_START_ADDR + 2 + 2*i, *(data + i));
        while(FLASH_GetStatus() == FLASH_BUSY);
    }

    FLASH_Lock();
}

int ReadConfigParaFromFlash(unsigned short *data, int datasize)
{
    int i;
    if(*(uint16_t *)CONFIG_PARA_START_ADDR == 0xAAAA)
    {
        for(i = 0;i < datasize;i++)
        {
            *(data + i) = *((uint16_t *)CONFIG_PARA_START_ADDR + i + 1);
        }
        return 0;
    }
    return -1;
}
void EraseNewSystemFlash()
{
    unsigned int i;
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
    for(i = 0; i < 80; i ++ )
    {
        FLASH_ErasePage(NEW_SYSTEM_START_ADDR + i * FlASH_PAGE_SIZE);
        while(FLASH_GetStatus() == FLASH_BUSY);
    }
    FLASH_ErasePage(IAP_FLAG_ADDR);
    while(FLASH_GetStatus() == FLASH_BUSY);
    FLASH_Lock();
}
void WriteNewSystemToFlash(unsigned short *data, unsigned short pack_number)
{
    unsigned int i;
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
    for(i = 0;i < 128; i++)
    {
        FLASH_ProgramHalfWord(NEW_SYSTEM_START_ADDR + 256*pack_number + 2*i, *(data + i));
        while(FLASH_GetStatus() == FLASH_BUSY);
    }
    FLASH_Lock();
}
void WriteIAPFlagToFlash(unsigned int num_of_halfword)
{
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
    FLASH_ProgramHalfWord(IAP_FLAG_ADDR, WRITEFLAG);
    while(FLASH_GetStatus() == FLASH_BUSY);
    FLASH_ProgramWord(IAP_FLAG_ADDR + 2, num_of_halfword);
    while(FLASH_GetStatus() == FLASH_BUSY);
    FLASH_Lock();
}
