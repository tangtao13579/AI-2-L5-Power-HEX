#ifndef _LTC4015_REG_DEFS_H_
#define _LTC4015_REG_DEFS_H_

#define LTC4015_ADDR 0xD0

#define LTC4015_KQC       8333.33 /*unit Hz/V */
#define LTC4015_R_SNSB    16.0f      /* unit mΩ */
#define LTC4015_R_SNSI    6       /* unit mΩ */

#define LTC4015_R_NTCBIAS 10.0f   /* unit KΩ*/

/*

*/
#define LTC4015_LO_ALERT_LIMIT_ADDR   0x10
#define LTC4015_LO_ALERT_LIMIT_VALUE  5
#define LTC4015_HI_ALERT_LIMIT_ADDR   0x11
#define LTC4015_HI_ALERT_LIMIT_VALUE  63380


/*
    Configuation settings 15:0
        -suspend_charger   8: = 1
        -run_bsr           5: = 0
        -force_meas_sys_on 4: = 1
        -mppt_en_i2c       3: = 0
        -en_qcount         2: = 0
*/
#define LTC4015_CONFIG_BITS_ADDR  0x14
#define LTC4015_CONFIG_BITS_SUSPEND_CHARGER 0x0110
#define LTC4015_CONFIG_BITS_ENABLE_CHARGER  0x0010

/*
    Input current limit setting 5:0
    = (IIN_LIMIT_SETTING + 1) * 0.5mV/R_SNSI = 1.0A
    IIN_LIMIT_SETTING = 17
*/
#define LTC4015_IIN_LIMIT_SETTING_ADDR   0x15
#define LTC4015_IIN_LIMIT_SETTING_VALUE  0x0011

/*
    UVCLFB input undervoltage limit 7:0
    = (VIN_UVCL_SETTING + 1) * 4.6875mV = 1.2V
    VIN_UVCL_SETTING = 0xFF
    Default value is 0xFF,need not configure.
*/
#define LTC4015_VIN_UVCL_SETTING  0x16

/*
    Maximum charge current target 4:0
    = (ICHARGE_TARGET + 1) * 1mV /R_SNSB = 0.81A
    ICHARGE_TARGET = 12
*/
#define LTC4015_ICHARGE_TARGET_ADDR   0x1A

/*
    Charge voltage target 4:0
    = (VCHARGE_SETTING/80.0 + 3.8125)V/Cell = 4.075V/Cell
    VCHARGE_SETTING = 21
*/
#define LTC4015_VCHARGE_SETTING_ADDR   0x1B
#define LTC4015_VCHARGE_SETTING_VALUE  0x0015

/*
    C_OVER_X_THRESHOLD not used now!
*/
#define LTC4015_C_OVER_X_THRESHOLD 0x1C

/*
    Max_CV_TIME 15:0
    Time in seconds with battery charger in the CV state before 
    timer termination occurs
    Max_CV_TIME = 3600s
*/
#define LTC4015_MAX_CV_TIME_ADDR   0x1D
#define LTC4015_MAX_CV_TIME_VALUE  0x0E10

/*
    MAX_CHARGE_TIME 15:0
    Time in seconds before a max_charge_time fault is declared
    MAX_CHARGE_TIME = 21600s
*/
#define LTC4015_MAX_CHARGE_TIME_ADDR   0x1E
#define LTC4015_MAX_CHARGE_TIME_VALUE  0x5460

#define LTC4015_JEITA_T1  0x1F
#define LTC4015_JEITA_T2  0x20
#define LTC4015_JEITA_T3  0x21
#define LTC4015_JEITA_T4  0x22
#define LTC4015_JEITA_T5  0x23
#define LTC4015_JEITA_T6  0x24

#define LTC4015_VCHARGE_JEITA_6_5 0x25
#define LTC4015_VCHARGE_JEITA_4_3_2 0x26
#define LTC4015_ICHARGE_JEITA_6_5 0x27
#define LTC4015_ICHARGE_JEITA_4_3_2 0x28

/*
    Battery charger configuration settings 2:0
    -en_c_over_x_term       2: = 0
    -en_lead_acid_temp_comp 1: = 0
    -en_jeita               0: = 0
*/
#define LTC4015_CHARGE_CONFIG_BITS_ADDR  0x29
#define LTC4015_CHARGE_CONFIG_BITS_VALUE 0x0000

#define LTC4015_CHARGER_STATE_ADDR  0x34
#define LTC4015_CHARGE_STATUS_ADDR  0x35

#define LTC4015_SYSTEM_STATUS_ADDR  0x39
#define LTC4015_VBAT_ADDR           0x3A
#define LTC4015_VIN_ADDR            0x3B
#define LTC4015_VSYS_ADDR           0x3C
#define LTC4015_IBAT_ADDR           0x3D
#define LTC4015_IIN_ADDR            0x3E
#define LTC4015_DIE_TEMP_ADDR       0x3F
#define LTC4015_NTC_RATIO_ADDR      0x40
#define LTC4015_BSR_ADDR            0x41
#define LTC4015_VBAT_FILT_ADDR      0x47
#define LTC4015_MEAS_SYS_VALID_ADDR 0x4A

#endif
