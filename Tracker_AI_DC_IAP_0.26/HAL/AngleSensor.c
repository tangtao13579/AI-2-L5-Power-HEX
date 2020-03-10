#include "stm32f10x_spi.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "ICM20602RegDefs.h"
#include "AngleSensor.h"
#include <math.h>

#define _SPI2_SetCS()    GPIO_SetBits(GPIOB,GPIO_Pin_12)
#define _SPI2_ResetCS()  GPIO_ResetBits(GPIOB,GPIO_Pin_12)

static float angle;
static int   angle_state = 0;
static unsigned char frame_count = 0;
static float YA_filter;
static float ZA_filter;
static float XG_filter;
static short XG_DC_BIAS;
/***************************************************************************
 Private functions
****************************************************************************/
static void  SPI2Init(void);
static void  ReadRegister(unsigned char addr, unsigned char *out);
static void  WriteRegister(unsigned char addr,unsigned char in);
static float InvSqrt(float number); 
static void  CalcAngle(void);
static void  GetRawData(void);
static void  InitMEMSToWork(void);
static void  RemoveXGDCBias(void);
static float KalmanFilter(float angle_rate,float accel_angle);
static void SPI2Init()
{
    GPIO_InitTypeDef GPIOInitStruc;
    SPI_InitTypeDef  SPI2InitStruc;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
    
    /* SPI-SCK,MOSI,MISO */
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
    GPIOInitStruc.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIOInitStruc.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&GPIOInitStruc);
    /* SPI-CS */
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_12;
    GPIOInitStruc.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIOInitStruc.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&GPIOInitStruc);
    GPIO_SetBits(GPIOB,GPIO_Pin_12);
    
    
    SPI2InitStruc.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
    SPI2InitStruc.SPI_Mode              = SPI_Mode_Master;
    SPI2InitStruc.SPI_DataSize          = SPI_DataSize_16b;
    SPI2InitStruc.SPI_CPOL              = SPI_CPOL_High;
    SPI2InitStruc.SPI_CPHA              = SPI_CPHA_2Edge;
    SPI2InitStruc.SPI_NSS               = SPI_NSS_Soft;
    SPI2InitStruc.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI2InitStruc.SPI_FirstBit          = SPI_FirstBit_MSB;
    SPI2InitStruc.SPI_CRCPolynomial     = 7;
    SPI_Init(SPI2,&SPI2InitStruc);
    
    SPI_CalculateCRC(SPI2,DISABLE);
    SPI_Cmd(SPI2,DISABLE);
}

static void ReadRegister(unsigned char addr, unsigned char *out)
{
    unsigned short send_data    = 0;
    unsigned short receive_data = 0;
    unsigned short count        = 0;
    
    send_data |= addr;
    send_data = send_data << 8;
    send_data |= 0x8000;

    SPI_Cmd(SPI2,ENABLE);
    _SPI2_ResetCS();
    SPI2->DR = send_data;
    while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE) != SET)
    {
        if(count++ >=4500)
        {
            break;
        }
    }
    receive_data = SPI2->DR;
    *out = receive_data & 0xFF;
    _SPI2_SetCS();
    SPI_Cmd(SPI2,DISABLE);

}
static void ReadRegisters(unsigned char addr,unsigned char len, unsigned char *out)
{
    unsigned short send_data    = 0;
    unsigned short receive_data = 0;
    unsigned short count        = 0;
    
    send_data |= addr;
    send_data = send_data << 8;
    send_data |= 0x8000;

    SPI_Cmd(SPI2,ENABLE);
    _SPI2_ResetCS();
    SPI2->DR = send_data;
    while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE) != SET)
    {
        if(count++ >=4500)
        {
            break;
        }
    }
    receive_data = SPI2->DR;
    *out = receive_data & 0xFF;
    _SPI2_SetCS();
    SPI_Cmd(SPI2,DISABLE);

}

static void WriteRegister(unsigned char addr,unsigned char in)
{

    unsigned short send_data    = 0;
    unsigned short count        = 0;
    
    send_data |= addr;
    send_data = send_data << 8;
    send_data |= in;
    send_data &= 0x7FFF;
    
    SPI_Cmd(SPI2,ENABLE);
    _SPI2_ResetCS();
    SPI2->DR = send_data;
    while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE) != SET)
    {
        if(count++ >=4500)
        {
            break;
        }
    }
    send_data  = SPI2->DR;
    _SPI2_SetCS();
    SPI_Cmd(SPI2,DISABLE);
}

static float InvSqrt(float number)
{
    long i;
    float x2,y;
    const float threehalfs = 1.5f;
    
    x2 = number * 0.5f;
    y  = number;
    i  = *(long*)&y;
    i  = 0x5F3759DF - (i>>1);
    y  = *(float *)&i;
    y  = y * ( threehalfs - ( x2 * y * y ) ); /* 1st iteration*/
    y  = y * ( threehalfs - ( x2 * y * y ) ); /* 2st iteration*/
    y  = y * ( threehalfs - ( x2 * y * y ) ); /* 3st iteration*/
    return y;
}
static float KalmanFilter(float angle_rate,float accel_angle)
{
    const float delta_t = 0.1;
    const float Q = 0.001;
    const float R = 20;
    static float alpha_prior = 0.0;
    static float beta_prior  = 0.0;
    static float alpha_post = 90.0;
    static float beta_post  = 0.0;
    
    static float p_prior_1 = 0;
    static float p_prior_2 = 0;
    static float p_prior_3 = 0;
    static float p_prior_4 = 0;
    
    static float p_post_1 = 1;
    static float p_post_2 = 0;
    static float p_post_3 = 0;
    static float p_post_4 = 1;
    
    static float K1 = 0;
    static float K2 = 0;
    
    alpha_prior = alpha_post - (delta_t * beta_post) + (delta_t * angle_rate);
    beta_prior  = beta_post;
    
    p_prior_1 = p_post_1 - (delta_t * p_post_3) - (delta_t * p_post_2) + (delta_t * delta_t * p_post_4) + Q;
    p_prior_2 = p_post_2 - (delta_t * p_post_4);
    p_prior_3 = p_post_3 - (delta_t * p_post_4);
    p_prior_4 = p_post_4 + Q;
    
    K1 = p_prior_1 / (p_prior_1 + R);
    K2 = p_prior_3 / (p_prior_1 + R);
    
    alpha_post = alpha_prior + K1 * (accel_angle - alpha_prior);
    beta_post  = beta_prior + K2 * (accel_angle - alpha_prior);
    
    p_post_1 = (1 - K1) * p_prior_1;
    p_post_2 = (1 - K1) * p_prior_2;
    p_post_3 = -1 * K2 * p_prior_1 + p_prior_3;
    p_post_4 = -1 * K2 * p_prior_2 + p_prior_4;
    
    return alpha_post;
}
static void CalcAngle()
{
    float accel_angle;
    if(frame_count < 10)
    {
        return;
    }
    YA_filter /= (frame_count*ICM_ACCEL_LSBG_VALUE);
    ZA_filter /= (frame_count*ICM_ACCEL_LSBG_VALUE);
    XG_filter /= (frame_count*ICM_GYRO_LSBDPS_VALUE);
    
    YA_filter *= -1;
    ZA_filter *= -1;
    XG_filter *= -1;
    
    accel_angle = atan2f(YA_filter,ZA_filter) * 57.3248 + 90;
    angle = KalmanFilter(XG_filter,accel_angle);
    
    frame_count = 0;
    YA_filter = 0;
    ZA_filter = 0;
    XG_filter = 0;
}

static void GetRawData1()
{
    unsigned char  temp;
    unsigned short count;
    int            frame_byte_count;

    short          YA_raw,ZA_raw,XG_raw;
    float          YA,ZA,XG;
    
    /* Get fifo count */
    count = 0;
    ReadRegister(ICM_FIFO_COUNTH,&temp);
    count |= temp;
    ReadRegister(ICM_FIFO_COUNTL,&temp);
    count = count << 8;
    count |= temp;
    if(count <= 5*14) /* One frame contains 14 bytes */
    {
       return; /*Too little data*/
    }
    
    /*Get raw data*/
    frame_byte_count = count;
    count = 0;  /*count frame*/
    YA = 0;
    ZA = 0;
    XG = 0;
    while(1)
    {
        frame_byte_count -= 14;
        if(frame_byte_count <= 0)
        {
            break; /*The remaining data is not enough for one frame*/
        }
        count ++;
        
        YA_raw = 0;
        ZA_raw = 0;
        XG_raw = 0;
        /*Discard XA*/
        ReadRegister(ICM_FIFO_R_W_ADDR,&temp);
        ReadRegister(ICM_FIFO_R_W_ADDR,&temp);
        /*Get YA*/
        ReadRegister(ICM_FIFO_R_W_ADDR,&temp);
        YA_raw |= temp;
        ReadRegister(ICM_FIFO_R_W_ADDR,&temp);
        YA_raw = YA_raw << 8;
        YA_raw |= temp;
        /*Get ZA*/
        ReadRegister(ICM_FIFO_R_W_ADDR,&temp);
        ZA_raw |= temp;
        ReadRegister(ICM_FIFO_R_W_ADDR,&temp);
        ZA_raw = ZA_raw << 8;
        ZA_raw |= temp;
        /*Discard TEMP*/
        ReadRegister(ICM_FIFO_R_W_ADDR,&temp);
        ReadRegister(ICM_FIFO_R_W_ADDR,&temp);
        /*Get XG*/
        ReadRegister(ICM_FIFO_R_W_ADDR,&temp);
        XG_raw |= temp;
        ReadRegister(ICM_FIFO_R_W_ADDR,&temp);
        XG_raw = XG_raw << 8;
        XG_raw |= temp;
        /*Discard YG*/
        ReadRegister(ICM_FIFO_R_W_ADDR,&temp);
        ReadRegister(ICM_FIFO_R_W_ADDR,&temp);
        /*Discard ZG*/
        ReadRegister(ICM_FIFO_R_W_ADDR,&temp);
        ReadRegister(ICM_FIFO_R_W_ADDR,&temp);
        
        YA += (float)YA_raw/ICM_ACCEL_LSBG_VALUE;
        XG += (float)XG_raw/ICM_GYRO_LSBDPS_VALUE;
    }
    YA /= count;
    XG /= count;
    ReadRegister(ICM_WHO_AM_I_ADDR,&temp);
    if(temp == ICM_WHO_AM_I_VALUE)
    {
        angle_state =  0;
    }
    else
    {
        angle_state = -1;
    }
}
static void GetRawData()
{    
    unsigned char temp;
    short YA_raw;
    short ZA_raw;
    short XG_raw;
float tempp;
    
    frame_count ++ ;
    /*YA*/
    ReadRegister(ICM_ACCEL_YOUT_H_ADDR,&temp);
    YA_raw |= temp;
    ReadRegister(ICM_ACCEL_YOUT_L_ADDR,&temp);
    YA_raw = YA_raw << 8;
    YA_raw |= temp;
    YA_filter += (float)YA_raw;
    /*ZA*/
    ReadRegister(ICM_ACCEL_ZOUT_H_ADDR,&temp);
    ZA_raw |= temp;
    ReadRegister(ICM_ACCEL_ZOUT_L_ADDR,&temp);
    ZA_raw = ZA_raw << 8;
    ZA_raw |= temp;
    ZA_filter += (float)ZA_raw;
    /*XG*/
    ReadRegister(ICM_GYRO_XOUT_H_ADDR,&temp);
    XG_raw |= temp;
    ReadRegister(ICM_GYRO_XOUT_L_ADDR,&temp);
    XG_raw = XG_raw << 8;
    XG_raw |= temp;
    tempp = (float)XG_raw/ICM_GYRO_LSBDPS_VALUE;
    XG_filter += (float)(XG_raw+XG_DC_BIAS);
}
static void RemoveXGDCBias()
{
    unsigned char temp,count;
    short         XGDCBias,temp_bias;  /*Gyro X_out DC bias*/
    float tempp;
    /*Device reset*/
    WriteRegister(ICM_POWER_MGMT1_ADDR,ICM_POWER_MGMT1_DEVICE_RESET);
    while(1)
    {
        count = 0;
        while(count < 72)
        {
            count++;   /*wait reset done*/
        }
        ReadRegister(ICM_POWER_MGMT1_ADDR,&temp);
        if((temp & 0x80) == 0x00)
        {
            break; /*reset is done*/
        }
    }
    
    /* Config ICM20602 for remove DC bias */
    WriteRegister(ICM_SMPLRT_DIV_ADDR,ICM_SMPLRT_DIV_VALUE2);         /*sample rate 1Khz*/
    WriteRegister(ICM_CONFIG_ADDR,ICM_CONFIG_VALUE);                  /*sample rate 1Khz for gyro*/  
    WriteRegister(ICM_GYRO_CONFIG_ADDR,ICM_GYRO_CONFIG_VALUE);        /*gyro FS_SEL = 250dps*/

    WriteRegister(ICM_GYRO_LP_CFG_ADDR,ICM_GYRO_LP_CFG_VALUE);        /*gyro is not low power mode*/
    WriteRegister(ICM_INT_PIN_CFG_ADDR,ICM_INT_PIN_CFG_VALUE);        /*Int pin active high*/
    WriteRegister(ICM_POWER_MGMT2_ADDR,ICM_POWER_MGMT2_VALUE2);       /*close acc*/
    WriteRegister(ICM_POWER_MGMT1_ADDR,ICM_POWER_MGMT1_VALUE1);       /*MEMS from sleep to work*/
    
    /*Discard the first 50 samples*/
    for(count = 0; count < 100; count++)
    {
        while(1)
        {
            ReadRegister(INT_STATUS_ADDR,&temp);
            if((temp & 0x01) != 0)
            {
                break;
            }
        }
    }
    for(count = 0; count < 100; count++)
    {
        while(1)
        {
            ReadRegister(INT_STATUS_ADDR,&temp);  /*wait data ready*/
            if((temp & 0x01) != 0)
            {
                break;
            }
        }
        /*XG*/
        temp_bias = 0;
        ReadRegister(ICM_GYRO_XOUT_H_ADDR,&temp);
        temp_bias |= temp;
        ReadRegister(ICM_GYRO_XOUT_L_ADDR,&temp);
        temp_bias = temp_bias << 8; 
        temp_bias |= temp;
        tempp = (float)temp_bias/ICM_GYRO_LSBDPS_VALUE;
        XGDCBias -= temp_bias; 
    }
    XGDCBias /= 100;
    XG_DC_BIAS = XGDCBias;
    tempp = (float)XGDCBias/ICM_GYRO_LSBDPS_VALUE;
    /*Write DC bias*/
//    temp = (XGDCBias & 0xFF);
//    WriteRegister(ICM_XG_OFFS_USRL_ADDR, temp);
//    temp = (XGDCBias >> 8) & 0xFF;
//    WriteRegister(ICM_XG_OFFS_USRH_ADDR, temp);
}
static void InitMEMSToWork()
{
     /* Config ICM20602 */
    WriteRegister(ICM_POWER_MGMT1_ADDR,ICM_POWER_MGMT1_VALUE2);       /*MEMS from work to sleep*/
    WriteRegister(ICM_SMPLRT_DIV_ADDR,ICM_SMPLRT_DIV_VALUE1);         /*sample rate 100Hz*/
    WriteRegister(ICM_CONFIG_ADDR,ICM_CONFIG_VALUE);                  /*sample rate 1KHz for gyro*/  
    WriteRegister(ICM_GYRO_CONFIG_ADDR,ICM_GYRO_CONFIG_VALUE);        /*gyro FS_SEL = 250dps*/
    WriteRegister(ICM_ACCEL_CONFIG_ADDR,ICM_ACCEL_CONFIG_VALUE);      /*accel FS_SEL = 2g*/
    WriteRegister(ICM_ACCEL_CONFIG2_ADDR,ICM_ACCEL_CONFIG2_VALUE);    /*sample rate 1KHz for accel*/ 
    WriteRegister(ICM_GYRO_LP_CFG_ADDR,ICM_GYRO_LP_CFG_VALUE);        /*gyro is not low power mode*/
    WriteRegister(ICM_INT_PIN_CFG_ADDR,ICM_INT_PIN_CFG_VALUE);        /*Int pin active high*/
    WriteRegister(ICM_POWER_MGMT2_ADDR,ICM_POWER_MGMT2_VALUE1);       /*enable YA ZA XG*/    
    WriteRegister(ICM_POWER_MGMT1_ADDR,ICM_POWER_MGMT1_VALUE1);       /*MEMS from sleep to work*/
}
/***************************************************************************************************

***************************************************************************************************/
int AngleSensorInit()
{
    unsigned char temp;
    
    SPI2Init();
    
    RemoveXGDCBias();
    
    InitMEMSToWork();

    /*Verify that the communication is working properly*/
    ReadRegister(ICM_WHO_AM_I_ADDR,&temp);
    if(temp == ICM_WHO_AM_I_VALUE)
    { 
        return 0;
    }
    else
    {
        return -1;
    }
}

int GetAngle(unsigned char AngleSensorID, float * a)
{

    GetRawData();
    CalcAngle();
    *a = angle;
    return angle_state;
}
