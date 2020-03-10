#include "stm32f10x_spi.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "LSM6DS3RegDefs.h"
#include "AngleSensor.h"
#include <math.h>

#define _SPI_SetCS()    GPIO_SetBits(GPIOA,GPIO_Pin_4)
#define _SPI_ResetCS()  GPIO_ResetBits(GPIOA,GPIO_Pin_4)

static float Gx_bias_filter;
/***************************************************************************
 Private functions
****************************************************************************/
static void  SPI1Init(void);
static void  ReadRegister(unsigned char addr, unsigned char *out);
static void  WriteRegister(unsigned char addr,unsigned char in);
static float KalmanFilter(float angle_rate,float accel_angle);
static float CalcAngle(float X,float Y,float Z);

static void SPI1Init()
{
    GPIO_InitTypeDef GPIOInitStruc;
    SPI_InitTypeDef  SPI1InitStruc;
    

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 
    
    /* SPI-SCK,MOSI,MISO */
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
    GPIOInitStruc.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIOInitStruc.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIOInitStruc);
    /* SPI-CS */
    GPIOInitStruc.GPIO_Pin   = GPIO_Pin_4;
    GPIOInitStruc.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIOInitStruc.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIOInitStruc);
    GPIO_SetBits(GPIOA,GPIO_Pin_4);
    
    
    SPI1InitStruc.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
    SPI1InitStruc.SPI_Mode              = SPI_Mode_Master;
    SPI1InitStruc.SPI_DataSize          = SPI_DataSize_16b;
    SPI1InitStruc.SPI_CPOL              = SPI_CPOL_High;
    SPI1InitStruc.SPI_CPHA              = SPI_CPHA_2Edge;
    SPI1InitStruc.SPI_NSS               = SPI_NSS_Soft;
    SPI1InitStruc.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI1InitStruc.SPI_FirstBit          = SPI_FirstBit_MSB;
    SPI1InitStruc.SPI_CRCPolynomial     = 7;
    SPI_Init(SPI1,&SPI1InitStruc);
    
    SPI_CalculateCRC(SPI1,DISABLE);
    SPI_Cmd(SPI1,DISABLE);
}
static void ReadRegister(unsigned char addr, unsigned char *out)
{
    unsigned short send_data    = 0;
    unsigned short receive_data = 0;
    unsigned short count        = 0;
    
    send_data |= addr;
    send_data = send_data << 8;
    send_data |= 0x8000;

    SPI_Cmd(SPI1,ENABLE);
    _SPI_ResetCS();
    SPI1->DR = send_data;
    while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) != SET)
    {
        if(count++ >=4500)
        {
            break;
        }
    }
    receive_data = SPI1->DR;
    *out = receive_data & 0xFF;
    _SPI_SetCS();
    SPI_Cmd(SPI1,DISABLE);

}

static void WriteRegister(unsigned char addr,unsigned char in)
{

    unsigned short send_data    = 0;
    unsigned short count        = 0;
    
    send_data |= addr;
    send_data = send_data << 8;
    send_data |= in;
    send_data &= 0x7FFF;
    
    SPI_Cmd(SPI1,ENABLE);
    _SPI_ResetCS();
    SPI1->DR = send_data;
    while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) != SET)
    {
        if(count++ >=4500)
        {
            break;
        }
    }
    send_data  = SPI1->DR;
    _SPI_SetCS();
    SPI_Cmd(SPI1,DISABLE);
}


static float KalmanFilter(float angle_rate,float accel_angle)
{
    const float delta_t = 0.02;
    const float Q = 0.01;
    const float R = 10;
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
static float CalcAngle(float Gx,float Ay,float Az)
{
    float acc_angle;
    Ay = Ay * LSM6DS3_ACC_FS;
    Az = Az * -1 * LSM6DS3_ACC_FS;
    Gx = (Gx - Gx_bias_filter) * LSM6DS3_GYRO_FS;
    acc_angle = atan2f(Ay,Az) * 57.3248 + 90;
    
    return KalmanFilter(Gx,acc_angle);
}

static void LSM6DS3Init()
{
    unsigned char temp;
    unsigned char count;
    short         Gx_bias;
    
    ReadRegister(LSM6DS3_WHO_AM_I_ADDR,&temp);
    while(1)
    {
        ReadRegister(LSM6DS3_WHO_AM_I_ADDR,&temp);
        if(temp == LSM6DS3_WHO_AM_I_VALUE)
        {
            break;  /* wait MEMS start , 20ms*/
        }
        else
        {
            count = 0;
            while(count < 50)
            {
                count ++;
            }
        }
    }
    /*Software reset*/
    WriteRegister(LSM6DS3_CTRL2_G_ADDR,LSM6DS3_CTRL2_G_POWER_DOWN);
    WriteRegister(LSM6DS3_CTRL1_XL_ADDR,LSM6DS3_CTRL2_G_HIGH_ODR);   
    WriteRegister(LSM6DS3_CTRL3_C_ADDR,LSM6DS3_CTRL3_C_RESET);
    while(1)
    {
        temp = 0;
        ReadRegister(LSM6DS3_CTRL3_C_ADDR,&temp);
        if((temp & 0x01) == 0x00)
        {
            break;     /*wait reset, 50us*/
        }
        else
        {
            count = 0;
            while(count < 50)
            {
                count ++;
            }
        }
    }  
    
    /*Config LSM6DS3*/
    /*disable access to embedded functions configuration registers*/
    WriteRegister(LSM6DS3_FUNC_CFG_ACCESS_ADDR, LSM6DS3_FUNC_CFG_ACCESS_VALUE);
    
    /*
    Pitch axis (X) angular rate sign,negative sign
    */
    WriteRegister(LSM6DS3_ORIENT_CFG_G_ADDR,LSM6DS3_ORIENT_CFG_G_VALUE);
    
    /*
    Disable INT pin
    */
    WriteRegister(LSM6DS3_INT1_CTRL_ADDR,LSM6DS3_INT1_CTRL_VALUE);
    WriteRegister(LSM6DS3_INT2_CTRL_ADDR,LSM6DS3_INT2_CTRL_VALUE);
    
    /*
    XL output data rate, 104Hz. (high performance)
    FS_XL = 2g.
    Analog filter BW 50Hz.
    */
    WriteRegister(LSM6DS3_CTRL1_XL_ADDR,LSM6DS3_CTRL1_XL_VALUE);
    
    /*
    Gyroscope output data, 104Hz (high performance)
    Gyroscope full-scale selection. 250dps
    */
    WriteRegister(LSM6DS3_CTRL2_G_ADDR,LSM6DS3_CTRL2_G_VALUE);
    
    /*
    open-drain selection
    Register address automatically incremented
    Little Endian Data selection
    */
    WriteRegister(LSM6DS3_CTRL3_C_ADDR,LSM6DS3_CTRL3_C_VALUE);
    
    /*
    Bandwidth determined by setting BW_XL
    Data-ready mask enable.
    */
    WriteRegister(LSM6DS3_CTRL4_C_ADDR,LSM6DS3_CTRL4_C_VALUE);
    
    /*
    Self-test disabled
    */
    WriteRegister(LSM6DS3_CTRL5_C_ADDR,LSM6DS3_CTRL5_C_VALUE);
    
    /*
    XL high-performance operating mode enabled
    */
    WriteRegister(LSM6DS3_CTRL6_C_ADDR,LSM6DS3_CTRL6_C_VALUE);
    
    /*
    Gyro high-performance operating mode enabled;
    HPF disable
    */
    WriteRegister(LSM6DS3_CTRL7_G_ADDR,LSM6DS3_CTRL7_G_VALUE);
    /*
     Accelerometer low-pass filter LPF2 enable
     High-pass ODR_XL/9
    */
    WriteRegister(LSM6DS3_CTRL8_XL_ADDR,LSM6DS3_CTRL8_XL_VALUE);
    
    /*
    SLOP_FDS Enable
    */
    WriteRegister(LSM6DS3_TAP_CFG_ADDR,LSM6DS3_TAP_CFG_VALUE);
    /*
    Accelerometer output enable
    */
    WriteRegister(LSM6DS3_CTRL9_XL_ADDR,LSM6DS3_CTRL9_XL_VALUE);
    
    /*
    Gyroscope output enable
    */
    WriteRegister(LSM6DS3_CTRL10_G_ADDR,LSM6DS3_CTRL10_G_VALUE);
    
    /* Remove gyro DC bias */
    count = 0;
    Gx_bias_filter = 0;
    while(1)
    {
        temp = 0;
        ReadRegister(LSM6DS3_STATUS_REG_ADDR,&temp);
        if((temp & 0x02) == 0x02)
        {
            Gx_bias = 0;
            ReadRegister(LSM6DS3_OUTX_H_G,&temp);
            Gx_bias |= temp;
            ReadRegister(LSM6DS3_OUTX_L_G,&temp);
            Gx_bias = Gx_bias << 8;
            Gx_bias |= temp;
            
            Gx_bias_filter += (float)Gx_bias;
            count ++;
        }
        if(count > 5)
        {
            Gx_bias_filter /= count;  /*Discard sampling*/
            break;
        }
    }
    count = 0;
    while(1)
    {
        temp = 0;
        ReadRegister(LSM6DS3_STATUS_REG_ADDR,&temp);
        if((temp & 0x02) == 0x02)
        {
            Gx_bias = 0;
            ReadRegister(LSM6DS3_OUTX_H_G,&temp);
            Gx_bias |= temp;
            ReadRegister(LSM6DS3_OUTX_L_G,&temp);
            Gx_bias = Gx_bias << 8;
            Gx_bias |= temp;
            
            Gx_bias_filter += (float)Gx_bias;
            count ++;
        }
        if(count > 30)
        {
            Gx_bias_filter /= count;
            break;
        }
    }
}


/*********************************************************************************
 Interface functions
**********************************************************************************/
int AngleSensorInit()
{
    unsigned char temp;
    
    SPI1Init();
    
    /* Config LSM6DS3*/
    LSM6DS3Init();
    
    ReadRegister(LSM6DS3_CTRL9_XL_ADDR,&temp);
    if(temp == LSM6DS3_CTRL9_XL_VALUE)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int GetAngle(unsigned char AngleSensorID, float * angle)
{
     unsigned char temp;
    short         Gx = 0;
    short         Ay = 0;
    short         Az = 0;
     /* Get Gx-axis*/
    ReadRegister(LSM6DS3_OUTX_H_G,&temp);
    Gx |= temp;
    ReadRegister(LSM6DS3_OUTX_L_G,&temp);
    Gx = Gx << 8;
    Gx |= temp;
    /* Get Y-axis*/
    ReadRegister(LSM6DS3_OUTY_H_XL,&temp);
    Ay |= temp;
    ReadRegister(LSM6DS3_OUTY_L_XL,&temp);
    Ay = Ay << 8;
    Ay |= temp;
    /* Get Z-axis*/
    ReadRegister(LSM6DS3_OUTZ_H_XL,&temp);
    Az |= temp;
    ReadRegister(LSM6DS3_OUTZ_L_XL,&temp);
    Az = Az << 8;
    Az |= temp;

    ReadRegister(LSM6DS3_CTRL9_XL_ADDR,&temp);
    if(temp == LSM6DS3_CTRL9_XL_VALUE)
    {
        *angle = CalcAngle((float)Gx,(float)Ay,(float)Az) ;
        return 0;
    }
    else
    {
        return -1;
    }
    
}

int AngleSensorLowPowerMode(unsigned char AngleSensorID)
{
    return 0;
}
int AngleSensorNormalMode(unsigned char AngleSensorID)
{
    return 0;
}
