#ifndef _ICM20602_REG_DEFS_
#define _ICM20602_REG_DEFS_

#define ICM_WHO_AM_I_ADDR      0x75
#define ICM_WHO_AM_I_VALUE     0x12

/* DC bias */
#define ICM_XG_OFFS_USRH_ADDR  0x13
#define ICM_XG_OFFS_USRL_ADDR  0x14
#define ICM_YG_OFFS_USRH_ADDR  0x15
#define ICM_YG_OFFS_USRL_ADDR  0x16
#define ICM_ZG_OFFS_USRH_ADDR  0x17
#define ICM_ZG_OFFS_USRL_ADDR  0x18

#define ICM_XA_OFFSET_H_ADDR   0x77
#define ICM_XA_OFFSET_L_ADDR   0x78
#define ICM_YA_OFFSET_H_ADDR   0x7A
#define ICM_YA_OFFSET_L_ADDR   0x7B
#define ICM_ZA_OFFSET_H_ADDR   0x7D
#define ICM_ZA_OFFSET_L_ADDR   0x7E

/* 
    Divides the internal sample rates
    sample_rate = internal_sample_rate / (1 + smplrt_div)
    internal_sample_rate = 1KHz
    ODRs = 100Hz
    smplrt_div = 99, 0x63
*/
#define ICM_SMPLRT_DIV_ADDR    0x19
#define ICM_SMPLRT_DIV_VALUE1   0x63  /*100 Hz*/
#define ICM_SMPLRT_DIV_VALUE2   0x00  /*1000 Hz*/

/*
    [2:0]DLPF_CFG. The gyrosope and temperature sensor are filtered
    according to the value of DLPF_CFG and FCHOICE_B
    Internal sample rate = 1KHz
    FCHOICE_B = 0
    DLPF_CFG  = 3
*/
#define ICM_CONFIG_ADDR        0x1A
#define ICM_CONFIG_VALUE       0x03

/*
[7]   XG_ST:0
[6]   YG_ST:0
[5]   ZG_ST:0
[4:3] FS_SEL:0b00 , 250dps
[2]   reserved
[1:0] FCHOICE_B:00
*/
#define ICM_GYRO_CONFIG_ADDR   0x1B
#define ICM_GYRO_CONFIG_VALUE  0x00
#define ICM_GYRO_LSBDPS_VALUE  131.07f
#define ICM_TEMP_LSBC_VALUE    326.8f

/*
[7] XA_ST:0
[6] YA_ST:0
[5] ZA_ST:0
[4:3] ACCEL_FS_SEL:0b00, 2g
[2:0] Reserved 
*/
#define ICM_ACCEL_CONFIG_ADDR  0x1C
#define ICM_ACCEL_CONFIG_VALUE 0x00
#define ICM_ACCEL_LSBG_VALUE   16383.75f

/*
[5:4] DEC2_CFG:0b01 ,Average 8 samples
[3]   ACCEL_FCHOICE_B:0
[2:0] A_DLPF_CFG:0b10 ,1KHz
*/
#define ICM_ACCEL_CONFIG2_ADDR  0x1D
#define ICM_ACCEL_CONFIG2_VALUE 0x02

/*
    Gyrosope low power mode 
*/
#define ICM_GYRO_LP_CFG_ADDR   0x1E
#define ICM_GYRO_LP_CFG_VALUE  0x00

/*
[7] INT_LEVEL        :1, logic level for INT pin is active low
[6] INT_OPEN         :1, INT pin open drain
[5] LATCH_INT_EN     :0, INT pin level held until interrupt status is cleared
[4] INT_RD_CLEAD     :1, Interrupt status is cleared if any read operation is performed
[3] FSYNC_INT_LEVEL  :0,
[2] FSYNC_INT_MODE_EN:0, Disabled FSYNC
[1:0] Reserved
*/
#define ICM_INT_PIN_CFG_ADDR   0x37
#define ICM_INT_PIN_CFG_VALUE  0xD0

/*
Interrupt enable
*/
#define ICM_INT_ENABLE_ADDR    0x38
#define ICM_INT_ENABLE_VALUE   0x00
/*
Interrupt status
*/
#define INT_STATUS_ADDR        0x3A
/* out */
#define ICM_ACCEL_XOUT_H_ADDR  0x3B
#define ICM_ACCEL_XOUT_L_ADDR  0x3C
#define ICM_ACCEL_YOUT_H_ADDR  0x3D
#define ICM_ACCEL_YOUT_L_ADDR  0x3E
#define ICM_ACCEL_ZOUT_H_ADDR  0x3F
#define ICM_ACCEL_ZOUT_L_ADDR  0x40

#define ICM_TEMP_OUT_H_ADDR    0x41
#define ICM_TEMP_OUT_L_ADDR    0x42

#define ICM_GYRO_XOUT_H_ADDR   0x43
#define ICM_GYRO_XOUT_L_ADDR   0x44
#define ICM_GYRO_YOUT_H_ADDR   0x45
#define ICM_GYRO_YOUT_L_ADDR   0x46
#define ICM_GYRO_ZOUT_H_ADDR   0x47
#define ICM_GYRO_ZOUT_L_ADDR   0x48

#define ICM_SIGNAL_PATH_RESET_ADDR  0x68

/*
[7]  -Reserved
[6]  -FIFO_EN: 1-ENABLE, 0-DISABLE
[5-3]-Reserved
[2]  -FIFO_RST:1-Reset,Auto cleared
[1]  -Reserved
[0]  -SIG_COND_RST:1-Reset all signal path
*/
#define ICM_USER_CTRL_ADDR          0x6A
#define ICM_USER_CTRL_ENFIFO_SIGRST 0x45

/*
FIFO ENABLE
*/
#define ICM_FIFO_EN_ADDR       0x23
#define ICM_FIFO_RN_VALUE      0X18

/*
FIFO READ WRITE
*/
#define ICM_FIFO_R_W_ADDR      0x74

/*
FIFO COUNT
*/
#define ICM_FIFO_COUNTH        0x72
#define ICM_FIFO_COUNTL        0x73

/*
[7]   DEVICE_RESET:0
[6]   SLEEP       :0
[5]   CYCLE       :0
[4]   GYRO_STANDBY:0
[3]   TEMP_DIS    :0
[2:0] CLKSEL      :0b001,
*/
#define ICM_POWER_MGMT1_ADDR         0x6B
#define ICM_POWER_MGMT1_VALUE1       0x01
#define ICM_POWER_MGMT1_VALUE2       0x41
#define ICM_POWER_MGMT1_DEVICE_RESET 0xC1
/*
[7:6] Reserved
[5]   STBY_XA:0 is on
[4]   STBY_YA:0 is on
[3]   STBY_ZA:0 is on
[2]   STBY_XG:0 is on
[1]   STBY_YG:0 is on
[0]   STBY_ZG:0 is on
*/
#define ICM_POWER_MGMT2_ADDR         0x6C
#define ICM_POWER_MGMT2_VALUE1       0x00
#define ICM_POWER_MGMT2_VALUE2       0x38
#endif
