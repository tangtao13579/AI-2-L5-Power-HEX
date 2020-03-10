#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"
#include "ModbusFrame.h"

#define TX_EN_485() {}
#define RX_EN_485() {}
#define Power_on_LoRa()  GPIO_ResetBits(GPIOA, GPIO_Pin_15)
#define Power_off_LoRa() GPIO_SetBits(GPIOA, GPIO_Pin_15)
/***************************************************************************************************
                                Private variable declaration
***************************************************************************************************/
typedef struct 
{
    unsigned short RxDataCnt;
    unsigned short TxDataCnt;
    unsigned char  TimerEn;
    unsigned char  TimeOutCnt;
    unsigned char  RxBuf[300];
    unsigned char  TxBuf[300];
    void (* SendByDMA)(unsigned short data_size);
    
}ComParaDef;

/***************************************************************************************************
                                  Private variable definition
***************************************************************************************************/
static ComParaDef ComPara[2] = {0};
static ComParaDef * PortList[2] = {(void * )0};

/***************************************************************************************************
                                    Private functions
***************************************************************************************************/

static void UART4Init(void);
static void DMA2C5Send(unsigned short data_size);
static unsigned short CRC16Check(unsigned char *buf, unsigned short num_of_bytes);

static void UART4Init()
{
    GPIO_InitTypeDef   GPIOInitStruc;
    USART_InitTypeDef  USARTInitStruc;
    DMA_InitTypeDef    DMAInitStruc;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2,ENABLE);
    
    DMAInitStruc.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
    DMAInitStruc.DMA_MemoryBaseAddr     = (uint32_t)&(ComPara[0].TxBuf);
    DMAInitStruc.DMA_DIR                = DMA_DIR_PeripheralDST;
    DMAInitStruc.DMA_BufferSize         = 128;
    DMAInitStruc.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMAInitStruc.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMAInitStruc.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMAInitStruc.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    DMAInitStruc.DMA_Mode               = DMA_Mode_Normal;
    DMAInitStruc.DMA_Priority           = DMA_Priority_Medium;
    DMAInitStruc.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel5, &DMAInitStruc);
    DMA_ITConfig(DMA2_Channel5, DMA_IT_TC,DISABLE);
    DMA_Cmd(DMA2_Channel5, DISABLE);
    
    /* UART4 Tx */
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_10;
    GPIOInitStruc.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIOInitStruc.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIOInitStruc);
    
    /* UART4 Rx */
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_11;
    GPIOInitStruc.GPIO_Mode  = GPIO_Mode_IPU;
    GPIOInitStruc.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIOInitStruc);
    
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  
  /* Power PIN-Hardware power:reset*/
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_15;
    GPIOInitStruc.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIOInitStruc.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIOInitStruc);
    Power_on_LoRa();
		
    USARTInitStruc.USART_BaudRate            = 9600;
    USARTInitStruc.USART_WordLength          = USART_WordLength_8b;
    USARTInitStruc.USART_StopBits            = USART_StopBits_1;
    USARTInitStruc.USART_Parity              = USART_Parity_No;
    USARTInitStruc.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USARTInitStruc.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART4, &USARTInitStruc);
    
    USART_ClearITPendingBit(UART4, USART_IT_RXNE);
    USART_ITConfig(UART4,USART_IT_RXNE,ENABLE);
    USART_ClearITPendingBit(UART4, USART_IT_TC);
    USART_ITConfig(UART4,USART_IT_TC,ENABLE);
    
    USART_DMACmd(UART4,USART_DMAReq_Tx, ENABLE);
    
    USART_Cmd(UART4,ENABLE);
}



static void DMA2C5Send(unsigned short data_size)
{
    TX_EN_485();
    DMA_Cmd(DMA2_Channel5, DISABLE);
    DMA2_Channel5->CNDTR = data_size;
    DMA_Cmd(DMA2_Channel5, ENABLE);
}

static unsigned short CRC16Check(unsigned char *buf, unsigned short num_of_bytes)
{
    unsigned short i     = 0;
    unsigned char  j     = 0;
    unsigned short w_crc = 0xFFFF;
    
    for(i = 0; i < num_of_bytes; i++)
    {
        w_crc ^= buf[i];
        for(j = 0; j < 8; j++)
        {
            if(w_crc & 1)
            {
                w_crc >>= 1;
                w_crc ^= 0xA001;
            }
            else
            {
                w_crc >>= 1;
            }
        }
    }
    
    return w_crc;
}
/***************************************************************************************************
                                Public functions
***************************************************************************************************/
void ModbusPortInit(unsigned char port_number)
{
    switch (port_number)
    {
        case 0:
            UART4Init();
            PortList[0] = &ComPara[0];
            PortList[0]->SendByDMA = DMA2C5Send;
            break;
        case 1: /*Reserved for expansion*/
            UART4Init();
            PortList[1] = &ComPara[1];
            PortList[1]->SendByDMA = DMA2C5Send;
            break;
        default:
            break;
    }
        
}

void LoRaModulePowerOff(void)
{
	Power_off_LoRa();
}

void LoRaModulePowerOn(void)
{
	Power_on_LoRa();
}

void ModbusSend(unsigned char port_number, unsigned short num_of_byte, unsigned char *SendBuffer)
{
    unsigned short i;
    unsigned short crc_value;
	TX_EN_485();
	for(i=0;i<50000;i++){__nop();__nop();__nop();__nop();__nop();}
	for(i=0;i<50000;i++){__nop();__nop();__nop();__nop();__nop();}
    for(i = 0; i < num_of_byte; i++ )
    {
        PortList[port_number]->TxBuf[i] = *(SendBuffer + i);
    }
    crc_value = CRC16Check(PortList[port_number]->TxBuf,num_of_byte );
    PortList[port_number]->TxBuf[num_of_byte] = (crc_value)&0xFF;
    PortList[port_number]->TxBuf[num_of_byte+1] = (crc_value>>8)&0xFF;
    (*(PortList[port_number]->SendByDMA))(num_of_byte+2); /* Send */
}

int ModbusRead(unsigned char port_number, unsigned short *num_of_byte, unsigned char *ReadBuffer)
{
    unsigned short crc_value;
    unsigned short i;
    
    if(PortList[port_number]->TimerEn == 0xAB)
    {
        if(PortList[port_number]->TimeOutCnt++ >= 6)   /* Receive a frame */
        {
            PortList[port_number]->TimerEn = 0x56;
            PortList[port_number]->TimeOutCnt = 0;
            
            *num_of_byte = PortList[port_number]->RxDataCnt;
            PortList[port_number]->RxDataCnt = 0;
            crc_value = CRC16Check(PortList[port_number]->RxBuf,*num_of_byte);
            if(crc_value == 0)
            {
                for (i = 0; i < *num_of_byte; i++)
                {
                    *(ReadBuffer + i) = PortList[port_number]->RxBuf[i];
                    PortList[port_number]->RxBuf[i] = 0xFF;
                }
                return 0;
            }
        }
    }
    else
    {
        PortList[port_number]->TimeOutCnt = 0;
    }
    return -1;
}


/***************************************************************************************************
                                    IRQHandler
***************************************************************************************************/
void UART4_IRQHandler()
{
    if(USART_GetITStatus(UART4,USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
        PortList[0]->TimeOutCnt = 0;
        PortList[0]->TimerEn = 0xAB;
        if(PortList[0] ->RxDataCnt < 300)
        {
            PortList[0]->RxBuf[PortList[0] ->RxDataCnt++] = UART4->DR;
        }
        else
        {
            PortList[0]->TimerEn = 0x56;
            PortList[0]->RxDataCnt = 0;
            PortList[0]->RxBuf[PortList[0] ->RxDataCnt] = UART4->DR;
        }
    }
    else if(USART_GetITStatus(UART4,USART_IT_TC) != RESET)
    {
        USART_ClearITPendingBit(UART4, USART_IT_TC);
        PortList[0]->RxDataCnt = 0;
        PortList[0]->TimeOutCnt = 0;
        PortList[0]->TimerEn = 0x56;
        
        DMA_Cmd(DMA2_Channel5, DISABLE);
        RX_EN_485();
    }
}


