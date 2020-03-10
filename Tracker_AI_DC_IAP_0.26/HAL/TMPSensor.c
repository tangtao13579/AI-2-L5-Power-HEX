#include "IIC.h"
#include "TMP75RegDefs.h"

static IIC_IO_Def _TMP75IO;

/***********************************************************************************************************************
                                            Private Function
***********************************************************************************************************************/
static int TMP75Init(void);

static int TMP75Init()
{
    return IICWriteByte(&_TMP75IO,TMP75_ADDR,TMP75_CONFIG_REG_ADDR,TMP75_CONFIG_REG_VALUE);
}


/***********************************************************************************************************************
                                            Public Function
***********************************************************************************************************************/
void TMPSensorInit()
{
    _TMP75IO.SCL_group = GPIOB;
    _TMP75IO.SCL_pin   = GPIO_Pin_6;
    _TMP75IO.SCL_RCC   = RCC_APB2Periph_GPIOB;
    
    _TMP75IO.SDA_group = GPIOB;
    _TMP75IO.SDA_pin   = GPIO_Pin_7;
    _TMP75IO.SDA_RCC   = RCC_APB2Periph_GPIOB;
    
    IICInit(&_TMP75IO);
    TMP75Init();
}

int GetTMP(signed short *tmp)
{
    signed short temp;
    unsigned char swap;
    int          is_OK = 0;
    is_OK = IICRead(&_TMP75IO,TMP75_ADDR,TMP75_TMP_REG_ADDR,(unsigned short *)&temp);
    if(is_OK == 0)
    {
        swap = *((unsigned char *)&temp + 1);
        temp = temp << 8;
        temp &= 0xFF00;
        temp |= swap;
        
        *tmp = temp/256;/*temp *128/16/2048*/
    }
    return is_OK;
}
