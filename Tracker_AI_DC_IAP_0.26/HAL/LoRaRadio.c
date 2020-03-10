#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"


/***********************************************************************************************************************
                                                Private function
***********************************************************************************************************************/
static void WaitOnBusy()
{
/*Used for block execution waiting for low state on radio BUSY pin*/
    
}
/***********************************************************************************************************************
                                                Public function
***********************************************************************************************************************/
void RadioWriteCommand(RadioCommand_t command, unsigned char *buffer, unsigned short size)
{
    unsigned short send_count;
    
    WaitOnBusy();
    
    RadioUart.putc(command);
    if(size > 0)
    {
        RadioUart.putc(size);
        for (send_count = 0; send_count < size; send_count++)
        {
            RadioUart.putc(buffer[i]);
        }
    }
    
    if(command != RADIO_SET_SLEEP)
    {
        WaitOnBusy();
    }
}

int RadioReadCommand(RadioCommand_t command, unsigned char *buffer, unsigned short size)
{
    WaitOnBusy();
    
    