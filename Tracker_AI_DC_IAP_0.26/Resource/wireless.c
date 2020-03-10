#include "stm32f10x_it.h"
#include "wireless.h"
#include "rtc.h"
#include "bsp.h"
#include "iap.h"
#include "ucos_ii.h"
#include "lora.h"
#include "user485.h"




#define sw16(x)\
((short)(\
(((short)(x)&(short)0x00ffU)<<8)|\
(((short)(x)&(short)0xff00U)>>8)))

extern Tracker tracker;
extern TIME tempTime;
extern Wireless wireless;
extern UFlag flag;
extern unsigned char *pOut[256];
  			
extern char TrVersion[];	//mian.c

unsigned short DeviceType_YG = 0x105D;//用于阳光厂家识别设备类型
unsigned int TrType_YG = 0x00200100;	//系统类型，单轴 东西 直流 1通道 0°

unsigned short TrType = 1;
//天智三路 //调整为一路，将第三路数据传给华为数传
unsigned short TrBracketNum = 3;    //1;	//3;
unsigned short reserved = 0;


extern uint8_t set_buf[50];
extern uint8_t ret_buf[300];
extern uint8_t trans_flag;
extern uint8_t trans_len;
extern uint8_t ret_len;
extern uint8_t trans_ret_flag;

extern U485 r485;
extern uint8_t set_buf[50];


uint16_t *ExComAddList[28]=
{   
    (uint16_t *)&TrVersion,								//40000  软件版本号，V1.02版增加了对华为逆变器的兼容
    ((uint16_t *)&TrVersion)+1,							//40001	
    (uint16_t *)&TrType,								//40002  跟踪器类型 0：斜单 1：东西向平单 2：南北向平单 3：双轴
    (uint16_t *)&TrBracketNum,							//40003  单个控制器控制的支架数量
    &reserved,
    &reserved,
    (uint16_t *)&tracker.ExInterface.Mode,    	 		//40006  工作模式 0：维护模式 1：自动模式 2：手动模式 3：避风模式 4：除雪模式 5：雨天模式
    (uint16_t *)&tracker.ExInterface.Wspeed,			//40007  风速 最小精度1m/s
                
    (uint16_t *)&tracker.ExInterface.Wdirect,			//40008  风向  最小精度1°
    &reserved,
    (uint16_t *)&tracker.SysTime.Year,	      			//40010  年 范围2000~2069
    (uint16_t *)&tracker.SysTime.Month,       			//40011  月     1~12
    (uint16_t *)&tracker.SysTime.Day,	        		//40012	 日     1~31
            
    (uint16_t *)&tracker.SysTime.Hour,   				//40013  时     0~23	 
    (uint16_t *)&tracker.SysTime.Minute,      			//40014  分     0~59
    (uint16_t *)&tracker.SysTime.Second,      			//40015  秒     0~59
    &reserved,
    (uint16_t *)&tracker.ExInterface.TargetAngle,		//40017 目标转角 精度 0.01°
    (uint16_t *)&tracker.ExInterface.Tswtich,			//40018  启停控制 0：正常运行  1：停止	
    &reserved,                                          //40019
    ((uint16_t *)&tracker.ExInterface.Tstatus)+1,		//40020  跟踪轴状态
    (uint16_t *)&tracker.ExInterface.Tstatus,			//40021
    
    
    &reserved,   
    (uint16_t *)&tracker.ExInterface.Cangle1,		 	//40023 通道1转角    范围0~180 精度0.01°
    &reserved,		
    (uint16_t *)&tracker.ExInterface.Cangle2,	 	 	//40025 通道2转角    范围0~180 精度0.01°
    &reserved,	
    (uint16_t *)&tracker.ExInterface.Cangle3,		 	//40027 通道3转角    范围0~180 精度0.01°
};

uint16_t *ExComAddList_YG[30]=
{   
    (uint16_t *)&DeviceType_YG,                         //28999  设备类型,固定值0x105D,用于阳光厂家识别设备类型
    (uint16_t *)&tracker.DeviceID,                      //29000  设备ID 1~255
    (uint16_t *)&TrVersion,                             //29001  版本信息（软件版本号），先发数组低字节
    ((uint16_t *)&TrVersion)+1,                         //29002
    ((uint16_t *)&TrType_YG)+1,                         //29003  系统类型
    (uint16_t *)&TrType_YG,                             //29004
    (uint16_t *)&TrBracketNum,                          //29005  支架配置数
    &reserved,                                          //29006  reserved
    &reserved,                                          //29007  reserved
    (uint16_t *)&tracker.ExInterface.Mode,              //29008  工作模式 0：维护模式 1：自动模式 2：手动模式 3：避风模式 4：除雪模式 5：雨天模式 6：停止模式
    (uint16_t *)&tracker.ExInterface.Wspeed,            //29009  风速 最小精度1m/s
    (uint16_t *)&tracker.ExInterface.Wdirect,           //29010  风向 最小精度1°
    &reserved,                                          //29011  reserved
    (uint16_t *)&tracker.SysTime.Year,                  //29012  年 范围2000~2069
    (uint16_t *)&tracker.SysTime.Month,                 //29013  月     1~12
    (uint16_t *)&tracker.SysTime.Day,                   //29014	 日     1~31
    (uint16_t *)&tracker.SysTime.Hour,                  //29015  时     0~23	 
    (uint16_t *)&tracker.SysTime.Minute,                //29016  分     0~59
    (uint16_t *)&tracker.SysTime.Second,                //29017  秒     0~59
    &reserved,                                          //29018  目标倾角 int16 精度0.01°
    (uint16_t *)&tracker.ExInterface.TargetAngle,       //29019  目标转角 int16 精度0.01°
    &reserved,                                          //29020  reserved
    ((uint16_t *)&tracker.SysErrMessage1.ErrMessage)+1, //29021  跟踪器状态(故障信息)
    (uint16_t *)&tracker.SysErrMessage1.ErrMessage,     //29022
    &reserved,                                          //29023  支架1倾角 int16 范围[0,90]   精度0.01°
    (uint16_t *)&tracker.ExInterface.CAngle1_YG,        //29024  支架1转角 int16 范围[0,180]  精度0.01°
    &reserved,                                          //29025  支架2倾角 int16 范围[0,90]   精度0.01°
    (uint16_t *)&tracker.ExInterface.CAngle2_YG,        //29026  支架2转角 int16 范围[0,180]  精度0.01°
    &reserved,                                          //29027  支架3倾角 int16 范围[0,90]   精度0.01°
    (uint16_t *)&tracker.ExInterface.CAngle3_YG,        //29028  支架4转角 int16 范围[0,180]  精度0.01°
};

//**************************************************************************************************
// Function Name  : Data_Analyze
// Description    : 
// Input          : None
// Output         : None
// Return         : None
//**************************************************************************************************
unsigned char Data_Analyze(COMMU *com, unsigned char ID)
{
    unsigned short i = 0;
    unsigned char value = 0;
    unsigned short crcValue = 0;
    unsigned short startAdd = 0;
    unsigned short dataLen = 0;
    unsigned short endAdd = 0;
    unsigned short temp16Value = 0;
    unsigned short tempPos = 0;
    short tempvalue = 0;
    //TIME tempTime;
    
    int32_t secdiff = 0;
    
    if(com->RxDataCnt>=1)
    {
        if(com->TimeOutCnt++>=30)
        {
            //lora
            USART_ITConfig(USART2,USART_IT_RXNE,DISABLE);
            Reserved485_TX_EN();
            for(i=0;i<com->RxDataCnt;i++)
            {
                USART_SendData(USART2,com->RxBuf[i]);
                while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)!=SET);
            }
            while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET); 
            Reserved485_RX_EN();
            USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
            
            LORA_MD0(0);
            LORA_MD1(0);
            //lora
            
            com->TimeOutCnt = 0;
            value = LONG_NO_CONNECT;
        }

		//if(com->RxDataCnt>=8) // && (com->RxBuf[0]==tracker.DeviceID || com->RxBuf[0]==0))
		if(com->RxDataCnt>=6)
        {
			com->TxBuf[0] = com->RxBuf[0];  // 设备地址
			com->TxBuf[1] = com->RxBuf[1];  // 命令 
	
	        // 检测命令		       
            switch(com->RxBuf[1])    
	        {
				case READ_REG:		// 读寄存器
					if(com->RxBuf[0]==tracker.DeviceID)
					{
						crcValue = GetCRC16CheckCode(com->RxBuf, 6);
						if((crcValue&0xFF)==com->RxBuf[6] && ((crcValue>>8)&0xFF)==com->RxBuf[7])  // CRC校验正确
						{
 							com->TxBuf[2] = com->RxBuf[5] << 1; // 返回数据长度
	
							crcValue = com->RxBuf[2];
							crcValue = ((crcValue << 8) | com->RxBuf[3]) * 2;
							
							startAdd =  com->RxBuf[2];
							startAdd = (((startAdd << 8) | com->RxBuf[3]));
							
							dataLen = com->RxBuf[4];
							dataLen <<= 8;
							dataLen |= com->RxBuf[5];
							
							endAdd = startAdd + dataLen-1;
							
							if(startAdd<28999)
							{
								for(i=0;i<com->TxBuf[2];i++)
								{
									com->TxBuf[3 + i] = *pOut[crcValue + i];
								}
							}
							else if(startAdd >= 28999 && endAdd <= 29028)
							{
								startAdd-=28999;
								for(i=0;i<com->RxBuf[5];i++)
								{
//									if(startAdd+i == 0)//设备类型，厂商识别码，低字节先发
//									{
//										com->TxBuf[2*i+3] = *ExComAddList_YG[startAdd+i];
//										com->TxBuf[2*i+4] = (*ExComAddList_YG[startAdd+i])>>8;
//									}
									//else if(startAdd+i == 2 || startAdd+i == 3)//软件版本号，优先发送 字符数组低字节
									if(startAdd+i == 2 || startAdd+i == 3)//软件版本号，优先发送 字符数组低字节
									{
										com->TxBuf[2*i+3] = *ExComAddList_YG[startAdd+i];
										com->TxBuf[2*i+4] = (*ExComAddList_YG[startAdd+i])>>8;
									}
									else
									{
										com->TxBuf[2*i+3] = (*ExComAddList_YG[startAdd+i])>>8;
										com->TxBuf[2*i+4] = *ExComAddList_YG[startAdd+i];
									}
								}
							}
							
							else if(startAdd >= 40000 && endAdd <= 40027)
							{//开放给外界的数据
								//数据更新
								//tracker.ExInterface.Mode = 0;
								
//								tracker.ExInterface.Wspeed = 0;
//								tracker.ExInterface.Wdirect = 0;
//								tracker.ExInterface.Tswtich = 0;
//								tracker.ExInterface.TargetAngle = 0;
//								tracker.ExInterface.Tstatus = 0;
//								tracker.ExInterface.Cangle1 = 0;
//								tracker.ExInterface.Cangle2 = 0;
//								tracker.ExInterface.Cangle3 = 0;


/*2016-05-30 注释*/
//								if(tracker.SysErrMessage1.BitEM.QJ2_NoCom==1 || flag.BitFlag.M1OverCurrent==1)
//								{//跟踪器状态报警
//									tracker.ExInterface.Tstatus = 2;
//								}
/*2016-05-30 注释 end*/
/*2016-05-30 增加*/	
								//M1 QJ1 -小平单项目用第三路，第一路不报警
//								if(tracker.SysErrMessage1.BitEM.QJ1_NoCom==1 || flag.BitFlag.M0OverCurrent==1)
//								{//跟踪器状态报警
//									tracker.ExInterface.Tstatus |= 1<<0;
//								}
//								else
//								{
//									tracker.ExInterface.Tstatus &= ~(1<<0);
//								}
								//M2 QJ2 -小平单项目用第三路，第二路不报警
//								if(tracker.SysErrMessage1.BitEM.QJ2_NoCom==1 || flag.BitFlag.M1OverCurrent==1)
//								{//跟踪器状态报警
//									tracker.ExInterface.Tstatus |= 1<<1;
//								}
//								else
//								{
//									tracker.ExInterface.Tstatus &= ~(1<<1);
//								}
								// 将第一路数据作为第一路传给华为数采
								if(tracker.SysErrMessage1.BitEM.QJ0_NoCom == 1 || flag.BitFlag.M0OverCurrent == 1 || tracker.SysErrMessage1.BitEM.RemoteTimeError == 1)
								{//跟踪器状态报警
									tracker.ExInterface.Tstatus |= 1<<0; //1<<2;
								}
								else
								{
									tracker.ExInterface.Tstatus &= ~(1<<0); //~(1<<2);
								}
/*2016-05-30 增加 end*/									
								//数据填充
								startAdd-=40000;
								for(i=0;i<com->RxBuf[5];i++)
								{
									if(startAdd+i == 0 || startAdd+i == 1)//软件版本号，优先发送数组 低地址
									{
										com->TxBuf[2*i+3] = *ExComAddList[startAdd+i];
										com->TxBuf[2*i+4] = (*ExComAddList[startAdd+i])>>8;
									}
									else
									{
										//com->TxBuf[3 + i] = *pOut[crcValue + i];
										com->TxBuf[2*i+3] = (*ExComAddList[startAdd+i])>>8;
										com->TxBuf[2*i+4] = *ExComAddList[startAdd+i];
									}
								}
							}
							else
							{//地址超范围
								com->TxBuf[0] = tracker.DeviceID;
								com->TxBuf[1] = READ_REG+0x80;
								com->TxBuf[2] = 2;
								crcValue = GetCRC16CheckCode(com->TxBuf, 3);
								com->TxBuf[3] = crcValue & 0xFF;
								com->TxBuf[4] = (crcValue >> 8) & 0xFF;
		
								com->TxDataCnt = 5;
								
								value = TRACKER_RD_FINISHED;
								if(value!=NO_CONNECT)
								{
									com->TimeOutCnt = 0;
									com->RxDataCnt = 0;

									if(ID==1)
									{
										DMA_Cmd(DMA1_Channel5, DISABLE);
										DMA1_Channel5->CNDTR = BUFFERSIZE;
										DMA_Cmd(DMA1_Channel5, ENABLE);
									}
								}
								return value;
							}
	
							crcValue = GetCRC16CheckCode(com->TxBuf, (com->TxBuf[2] + 3));
							com->TxBuf[com->TxBuf[2] + 3] = crcValue & 0xFF;
							com->TxBuf[com->TxBuf[2] + 4] = (crcValue >> 8) & 0xFF;
	
							com->TxDataCnt = com->TxBuf[2] + 5;
							
							value = TRACKER_RD_FINISHED;
						}
						else
						{
							value = TRACKER_RD_ERROR;
						}
					}
                    break;
				case WRITE_REG:		// 写寄存器
					if(com->RxDataCnt>=(com->RxBuf[6]+9) && (com->RxBuf[0]==tracker.DeviceID || com->RxBuf[0]==0))
                    {
                        crcValue = GetCRC16CheckCode(com->RxBuf,(com->RxBuf[6]+7));
						if((crcValue&0xFF)==com->RxBuf[(com->RxBuf[6]+7)] && ((crcValue>>8)&0xFF)==com->RxBuf[(com->RxBuf[6]+8)])  // CRC校验正确
						{
							com->TxBuf[2] = com->RxBuf[2];  // 寄存器首地址高字节
							com->TxBuf[3] = com->RxBuf[3];  // 寄存器首地址低字节
							com->TxBuf[4] = com->RxBuf[4];  // 寄存器个数高字节
							com->TxBuf[5] = com->RxBuf[5];  // 寄存器个数低字节
                            
							crcValue = com->RxBuf[2];
							crcValue <<= 8;
							crcValue |= com->RxBuf[3];
							
							dataLen = com->RxBuf[4];
							dataLen <<= 8;
							dataLen |= com->RxBuf[5];
							endAdd = crcValue + dataLen-1;
							if(crcValue<28999)
							{
								switch(crcValue)                 // 根据地址写入
								{
								case DEVICE_EDITION_REG_ADDR_BASE: // 设备版本写入
									for(i=0;i<2;i++)
									{
										*pOut[2*DEVICE_EDITION_REG_ADDR_BASE + i] = com->RxBuf[7 + i];
									}
									flag.BitFlag.EditionUpdate = 1;
									break;
								case SYSWORKMODE_REG_ADDR_BASE: 
									for(i=0;i<4;i++)
									{
										*pOut[2*SYSWORKMODE_REG_ADDR_BASE + i] = com->RxBuf[7 + i];
									}
									break;/*
								case SYSWORKMODE_REG_ADDR_BASE:   // 工作模式写入
									switch(com->RxBuf[8])
									{
										case 0x00:   // 停止
										case 0x40:   // 维护模式
										case 0x20:   // 自动模式
										case 0x10:   // 手动模式
										case 0x18:   // 手动向东
										//case 0x1A:   // 手动向东、向南
										//case 0x19:   // 手动向东、向北
										case 0x14:   // 手动向西
										//case 0x16:   // 手动向西、向南
										//case 0x15:   // 手动向西、向北
										//case 0x12:   // 手动向南
										//case 0x11:   // 手动向北
											for(i=0;i<2;i++)
												{
													*pOut[2*SYSWORKMODE_REG_ADDR_BASE + i] = com->RxBuf[7 + i];
												}
												flag.BitFlag.SysWorkModeUpdate = 1;
											break;
										default:
											break;
									}
									break;*/
								case TIME_REG_ADDR_BASE:          // 系统时间更新
									tempTime.Year = com->RxBuf[7];
									tempTime.Year = (tempTime.Year << 8) | com->RxBuf[8];
									tempTime.Month = com->RxBuf[10];
									tempTime.Day = com->RxBuf[12];
									tempTime.Hour = com->RxBuf[14];
									tempTime.Minute = com->RxBuf[16];
									tempTime.Second = com->RxBuf[18];
									flag.BitFlag.TimeUpdate = 1;
									
									//清除初始上电状态
									BKP_WriteBackupRegister(BKP_DR9, 0xAAAA);
									flag.BitFlag.PowerONTimeUpdate =  0;
									
									//上位机更新时间，清除远程错误标志，并开启远程时间更新
									tracker.SysErrMessage1.BitEM.RemoteTimeError = 0;	
									flag.BitFlag.DailyTimeUpdate = 1;
								
									break;/*
								case REMOTE_WINDSPEED_REG_ADDR_BASE: // 传感器选择地址
									for(i=0;i<2;i++)
									{
										*pOut[2*REMOTE_WINDSPEED_REG_ADDR_BASE + i] = com->RxBuf[7 + i];
									}
									//flag.BitFlag.SensorSelectUpdate = 1;
									break;*/
								case MOTOR_PULSE_REG_ADDR_BASE:   // 电机脉冲更新
									//for(i=0;i<8;i++)
									//{
										//*pOut[EW_PULSE_ADDR_BASE + i] = com->RxBuf[7 + i];
							
									//}
									//pulseBuffer[0] = com->RxBuf[7];
									//pulseBuffer[1] = com->RxBuf[11];
									//flag.BitFlag.PulseUpdate = 1;
									break;
								
								case MOTOR_RUN_CURRENT_ADDR_BASE: //电机过流保护电流更新
									for(i=0;i<4;i++)
									{
										*pOut[2*MOTOR_RUN_CURRENT_ADDR_BASE + i] = com->RxBuf[7 + i];
									}
									flag.BitFlag.MotorRunCurUpdate = 1;
									break;

								case LONGITUDE_REG_ADDR_BASE:     // 天文更新
									for(i=0;i<12;i++)
									{
										*pOut[2*LONGITUDE_REG_ADDR_BASE + i] = com->RxBuf[7 + i];
									}
									flag.BitFlag.SolarUpdate = 1;
									break;
								case TWTRACKER_EW_REG_ADDR_BASE:  // 天文算法跟踪精度
									for(i=0;i<8;i++)
									{
										*pOut[2*TWTRACKER_EW_REG_ADDR_BASE + i] = com->RxBuf[7 + i];
									}
									flag.BitFlag.TWJDUpdate = 1;
									break;
								case MOTOR_FUNCA_REG_ADDR_BASE:   // 方程A更新
									for(i=0;i<16;i++)
									{
										*pOut[2*MOTOR_FUNCA_REG_ADDR_BASE + i] = com->RxBuf[7 + i];
									}
									flag.BitFlag.FuncAUpdate = 1;
									break;
								case MOTOR_FUNCB_REG_ADDR_BASE:   // 方程B更新
									for(i=0;i<32;i++)
									{
										*pOut[2*MOTOR_FUNCB_REG_ADDR_BASE + i] = com->RxBuf[7 + i];
									}
									flag.BitFlag.FuncBUpdate = 1;
									break;
								case ID_REG_ADDR_BASE:            // 设备地址更新
									*pOut[2*ID_REG_ADDR_BASE + 0] = com->RxBuf[7];
									*pOut[2*ID_REG_ADDR_BASE + 1] = com->RxBuf[8];
									flag.BitFlag.IDUpdate = 1;
									break;
								case FUNC_AUTO_UPDATE_REG_ADDR_BASE:  // 方程自更新  倾角自更新
									*pOut[2*FUNC_AUTO_UPDATE_REG_ADDR_BASE + 0] = com->RxBuf[7];
									*pOut[2*FUNC_AUTO_UPDATE_REG_ADDR_BASE + 1] = com->RxBuf[8];
									flag.BitFlag.FuncAutoUpdate = 1;
									flag.BitFlag.QJAutoUpdate = 1;
									break;
								case QJ_AUTO_UPDATE_REG_ADDR_BASE:    // 倾角自更新
									if(com->RxBuf[8]!=0)
									{
										flag.BitFlag.QJAutoUpdate = 1;
									}
									break;
//								case SUNSENSOR_ADJUST_EW_BALANCE_REG_ADDR_BASE: // 光感校正
//									for(i=0;i<8;i++)
//									{
//										*pOut[2*SUNSENSOR_ADJUST_EW_BALANCE_REG_ADDR_BASE + i] = com->RxBuf[7 + i];
//									}
//									flag.BitFlag.SunSensorUpdate = 1;
//									break;
                                    

                                
								case ARRAY_REG_ADDR_BASE:   // 组件参数
									for(i=0;i<16;i++)
									{
										*pOut[2*ARRAY_REG_ADDR_BASE + i] = com->RxBuf[7 + i];
									}
									flag.BitFlag.ArrayUpdate = 1;
									break;
								case WIND_PROTECT_REG_ADDR_BASE:  // 大风保护设置
									for(i=0;i<8;i++)
									{
										*pOut[2*WIND_PROTECT_REG_ADDR_BASE + i] = com->RxBuf[7 + i];
									}
									flag.BitFlag.WindProtectUpdate = 1;
									break;
								case SOFT_LIMIT_REG_ADDR_BASE:    // 软限位角度设置
									for(i=0;i<8;i++)
									{
										*pOut[2*SOFT_LIMIT_REG_ADDR_BASE + i] = com->RxBuf[7 + i];
									}
									flag.BitFlag.SoftLimitUpdate = 1;
									break;
								case REMOTE_WINDSPEED_REG_ADDR_BASE: // 远程风速
									*pOut[2*REMOTE_WINDSPEED_REG_ADDR_BASE + 0] = com->RxBuf[7];
									*pOut[2*REMOTE_WINDSPEED_REG_ADDR_BASE + 1] = com->RxBuf[8];
									break;
                                
                                case STAGGERING_PEAK_BOOT_NUM_REG_ADDR_BASE:    // 错峰启动基数
                                    *pOut[2*STAGGERING_PEAK_BOOT_NUM_REG_ADDR_BASE + 0] = com->RxBuf[7];
									*pOut[2*STAGGERING_PEAK_BOOT_NUM_REG_ADDR_BASE + 1] = com->RxBuf[8];
									flag.BitFlag.StaggerNumUpdate = 1;
									break;
                                
                                case BACK_ANGLE_REG_ADDR_BASE:  // 夜返停靠角度设置
                                    *pOut[2*BACK_ANGLE_REG_ADDR_BASE + 0] = com->RxBuf[7];
									*pOut[2*BACK_ANGLE_REG_ADDR_BASE + 1] = com->RxBuf[8];
                                    flag.BitFlag.BackAngleUpdate = 1;
                                    break;
                                
                                case WIND_PROTECT_ANGLE_EAST_REG_ADDR_BASE: //大风停靠角度
                                    // 东
                                    *pOut[2*WIND_PROTECT_ANGLE_EAST_REG_ADDR_BASE + 0] = com->RxBuf[7];
									*pOut[2*WIND_PROTECT_ANGLE_EAST_REG_ADDR_BASE + 1] = com->RxBuf[8];
                                    // 西
                                    *pOut[2*WIND_PROTECT_ANGLE_EAST_REG_ADDR_BASE + 2] = com->RxBuf[9];
									*pOut[2*WIND_PROTECT_ANGLE_EAST_REG_ADDR_BASE + 3] = com->RxBuf[10];
                                    flag.BitFlag.WindProAngleUpdate = 1;
                                
								default:
									break;
								}
							}
							else if(crcValue >= 29008 && endAdd <= 29019)//阳光接口地址(写)
							{
								if(crcValue<=29008 && endAdd>=29008)
								{//工作模式
									//计算数据位置
									tempPos = 29008 - crcValue;
									temp16Value = com->RxBuf[7+2*tempPos];
									temp16Value <<= 8;
									temp16Value |= com->RxBuf[8+2*tempPos];
									
									if(temp16Value==0)
									{//维护模式
										tracker.ExInterface.Mode = 0;
										tracker.SysWorkMode = 0x00000040;
									}
									else if(temp16Value==1)
									{//自动模式
										tracker.ExInterface.Mode = 1;
										tracker.SysWorkMode = 0x00000020;
									}
									else if(temp16Value==2)
									{//手动模式
										tracker.ExInterface.Mode = 2;
										tracker.SysWorkMode = 0x00000010;
									}
									else if(temp16Value==3)
									{//大风
										tracker.ExInterface.Mode = 3;
										tracker.SysWorkMode = 0x00000024;
									}
									else if(temp16Value==4)
									{//除雪
										tracker.ExInterface.Mode = 4;
										tracker.SysWorkMode = 0x00000022;
									}
									else if(temp16Value==5)
									{//雨天
										tracker.ExInterface.Mode = 5;
										tracker.SysWorkMode = 0x00000021;
									}
									else if(temp16Value==6)
									{//停止
										tracker.ExInterface.Mode = 6;
										tracker.SysWorkMode = 0;
									}
									else
									{//数据超范围
										
									}
								}

								if(crcValue<=29009 && endAdd>=29009)
								{//风速
									tempPos = 29009 - crcValue;
									temp16Value = com->RxBuf[7+2*tempPos];
									temp16Value <<= 8;
									temp16Value |= com->RxBuf[8+2*tempPos];
									tracker.ExInterface.Wspeed = temp16Value;
									
									tracker.Sensor.RemoteWindSpeed = temp16Value;
								}
								
								if(crcValue<=29010 && endAdd>=29010)
								{//风向 预留
								}
								
								//if((crcValue<=29012 && endAdd>=29012) || (crcValue<=29017 && endAdd>=29017))
								if((crcValue >= 29012 && crcValue <= 29017)
								   || (endAdd >= 29012 && endAdd <= 29017)
								   || (crcValue < 29012 && endAdd > 29017))
								{//系统时间
									tempTime.Year   = tracker.SysTime.Year;
									tempTime.Month  = tracker.SysTime.Month;
									tempTime.Day    = tracker.SysTime.Day;
									tempTime.Hour   = tracker.SysTime.Hour;
									tempTime.Minute = tracker.SysTime.Minute;
									tempTime.Second = tracker.SysTime.Second;
									
									tempvalue = 29012-crcValue;
									if(tempvalue>=0 && endAdd>=29012)
									{//年
										tempTime.Year = com->RxBuf[7+2*tempvalue];
										tempTime.Year <<= 8;
										tempTime.Year |= com->RxBuf[8+2*tempvalue];
									}
									tempvalue = 29013-crcValue;
									if(tempvalue>=0 && endAdd>=29013)
									{//月
										tempTime.Month = com->RxBuf[7+2*tempvalue];
										tempTime.Month <<= 8;
										tempTime.Month |= com->RxBuf[8+2*tempvalue];
									}
									tempvalue = 29014-crcValue;
									if(tempvalue>=0 && endAdd>=29014)
									{//日
										tempTime.Day = com->RxBuf[7+2*tempvalue];
										tempTime.Day <<= 8;
										tempTime.Day |= com->RxBuf[8+2*tempvalue];
									}
									tempvalue = 29015-crcValue;
									if(tempvalue>=0 && endAdd>=29015)
									{//时
										tempTime.Hour = com->RxBuf[7+2*tempvalue];
										tempTime.Hour <<= 8;
										tempTime.Hour |= com->RxBuf[8+2*tempvalue];
									}
									tempvalue = 29016-crcValue;
									if(tempvalue>=0 && endAdd>=29016)
									{//分
										tempTime.Minute = com->RxBuf[7+2*tempvalue];
										tempTime.Minute <<= 8;
										tempTime.Minute |= com->RxBuf[8+2*tempvalue];
									}
									tempvalue = 29017-crcValue;
									if(tempvalue>=0 && endAdd>=29017)
									{//秒
										tempTime.Second = com->RxBuf[7+2*tempvalue];
										tempTime.Second <<= 8;
										tempTime.Second |= com->RxBuf[8+2*tempvalue];
									}
									
									if(flag.BitFlag.PowerONTimeUpdate == 1)	//断电后新上电
									{
										flag.BitFlag.PowerONTimeUpdate = 0;
										flag.BitFlag.TimeUpdate = 1;
										BKP_WriteBackupRegister(BKP_DR9, 0xAAAA);
									}
									else
									{
										if(flag.BitFlag.DailyTimeUpdate == 1)	//每日更新标志位
										{
											flag.BitFlag.DailyTimeUpdate = 0;
											secdiff = get_seccount(&tempTime) - get_seccount(&tracker.SysTime);
											
											if(secdiff >= -300 && secdiff <= 300)//时间一致,误差5分钟内
											{
												flag.BitFlag.TimeUpdate = 1;//
												tracker.SysErrMessage1.BitEM.RemoteTimeError = 0;//清除remote更新时间错误标志位
												
											}
											else//时间不一致
											{
												tracker.SysErrMessage1.BitEM.RemoteTimeError = 1;//置位remote更新时间错误标志位
											}
										}
									}
								}
								//目标倾角  目标转角  29018  29019
								{
								}								
							}
							else if(crcValue>= 40004 && endAdd<=40015)//华为接口地址（写）
							{//地址40004以后为外部接口
//								unsigned short crcValue = 0;
//								unsigned short dataLen = 0;
//								unsigned short endAdd = 0;
 

								
								//经度纬度超范围   //40004  40005
								{
								}
																
								
								if(crcValue<=40006 && endAdd>=40006)
								{//工作模式
									//计算数据位置
									//crcValue -=40006;
									tempPos = 40006 - crcValue;
									temp16Value = com->RxBuf[7+2*tempPos];
									temp16Value <<= 8;
									temp16Value |= com->RxBuf[8+2*tempPos];
									
									if(temp16Value==0)
									{//维护模式
										tracker.ExInterface.Mode = 0;
										tracker.SysWorkMode = 0x00000040;
									}
									else if(temp16Value==1)
									{//自动模式
										tracker.ExInterface.Mode = 1;
										tracker.SysWorkMode = 0x00000020;										
									}
									else if(temp16Value==2)
									{//手动模式
										tracker.ExInterface.Mode = 2;
										tracker.SysWorkMode = 0x00000010;										
									}
									else if(temp16Value==3)
									{//大风
										tracker.ExInterface.Mode = 3;
										tracker.SysWorkMode = 0x00000024;										
									}
									else if(temp16Value==4)
									{//除雪
										tracker.ExInterface.Mode = 4;
										tracker.SysWorkMode = 0x00000022;										
									}
									else if(temp16Value==5)
									{//雨天
										tracker.ExInterface.Mode = 5;
										tracker.SysWorkMode = 0x00000021;										
									}
									else if(temp16Value==6)
									{//停止
										tracker.ExInterface.Mode = 6;
										tracker.SysWorkMode = 0;
									}
									else if(temp16Value == 7)
									{//阴雨（实际运行维护模式）
										tracker.ExInterface.Mode = 7;
										tracker.SysWorkMode = 0x00000040;
									}
									else
									{//数据超范围
										
									}
								}

								if(crcValue<=40007 && endAdd>=40007)
								{//风速
									tempPos = 40007 - crcValue;
									temp16Value = com->RxBuf[7+2*tempPos];
									temp16Value <<= 8;
									temp16Value |= com->RxBuf[8+2*tempPos];
									tracker.ExInterface.Wspeed = temp16Value;
									
									tracker.Sensor.RemoteWindSpeed = temp16Value;
								}
								
								if(crcValue<=40008 && endAdd>=40008)
								{//风向 预留
								}
								
								//if((crcValue<=40010&&endAdd>=40010) || (crcValue<=40015&&endAdd>=40015))
								if((crcValue >= 40010 && crcValue <= 40015)
								   || (endAdd >= 40010 && endAdd <= 40015)
								   || (crcValue < 40010 && endAdd > 40015))
								{//系统时间
									tempTime.Year = tracker.SysTime.Year;
									tempTime.Month = tracker.SysTime.Month;
									tempTime.Day  =  tracker.SysTime.Day;
									tempTime.Hour = tracker.SysTime.Hour;
									tempTime.Minute = tracker.SysTime.Minute;
									tempTime.Second = tracker.SysTime.Second;
									
									tempvalue = 40010-crcValue;
									if(tempvalue>=0 && endAdd>=40010)
									{//年
										tempTime.Year = com->RxBuf[7+2*tempvalue];
										tempTime.Year <<= 8;
										tempTime.Year |= com->RxBuf[8+2*tempvalue];										 
									}
									tempvalue = 40011-crcValue;
									if(tempvalue>=0 && endAdd>=40011)
									{//月
										tempTime.Month = com->RxBuf[7+2*tempvalue];
										tempTime.Month <<= 8;
										tempTime.Month |= com->RxBuf[8+2*tempvalue];										 
									}
									tempvalue = 40012-crcValue;
									if(tempvalue>=0 && endAdd>=40012)
									{//日
										tempTime.Day = com->RxBuf[7+2*tempvalue];
										tempTime.Day <<= 8;
										tempTime.Day |= com->RxBuf[8+2*tempvalue];										 
									}
									tempvalue = 40013-crcValue;
									if(tempvalue>=0 && endAdd>=40013)
									{//时
										tempTime.Hour = com->RxBuf[7+2*tempvalue];
										tempTime.Hour <<= 8;
										tempTime.Hour |= com->RxBuf[8+2*tempvalue];										 
									}
									tempvalue = 40014-crcValue;
									if(tempvalue>=0 && endAdd>=40014)
									{//分
										tempTime.Minute = com->RxBuf[7+2*tempvalue];
										tempTime.Minute <<= 8;
										tempTime.Minute |= com->RxBuf[8+2*tempvalue];										 
									}
									tempvalue = 40015-crcValue;
									if(tempvalue>=0 && endAdd>=40015)
									{//秒
										tempTime.Second = com->RxBuf[7+2*tempvalue];
										tempTime.Second <<= 8;
										tempTime.Second |= com->RxBuf[8+2*tempvalue];										 
									}
									
									if(flag.BitFlag.PowerONTimeUpdate == 1)	//断电后新上电
									{
										flag.BitFlag.PowerONTimeUpdate = 0;
										flag.BitFlag.TimeUpdate = 1;
										BKP_WriteBackupRegister(BKP_DR9, 0xAAAA);
									}
									else
									{
										if(flag.BitFlag.DailyTimeUpdate == 1)	//每日更新标志位
										{
											flag.BitFlag.DailyTimeUpdate = 0;
											secdiff = get_seccount(&tempTime) - get_seccount(&tracker.SysTime);
											
											if(secdiff >= -300 && secdiff <= 300)//时间一致,误差5分钟内
											{
												flag.BitFlag.TimeUpdate = 1;//
												tracker.SysErrMessage1.BitEM.RemoteTimeError = 0;//清除remote更新时间错误标志位
												
											}
											else//时间不一致
											{
												tracker.SysErrMessage1.BitEM.RemoteTimeError = 1;//置位remote更新时间错误标志位
											}
										}
									}
								}
								//目标倾角  目标转角  40016  40017 
								{
								}
								//启停控制  40018
//								if(crcValue<=40016 && endAdd>=40016)
//								{//启停
//									tempPos = 40016 - crcValue;
//									temp16Value = com->RxBuf[7+2*tempPos];
//									temp16Value <<= 8;
//									temp16Value |= com->RxBuf[8+2*tempPos];
//									tracker.ExInterface.Tswtich = temp16Value;									
//									if(temp16Value==0)
//									{
//										//tracker.ExInterface.Tswtich = temp16Value;
//										tracker.ExInterface.Mode = 1;
//										tracker.SysWorkMode = 0x00000020;											
//									}
//									else
//									{
//										tracker.ExInterface.Mode = 6;
//										tracker.SysWorkMode = 0;
//									}
//								}
							}
							else
							{//地址超范围
								com->TxBuf[0] = tracker.DeviceID;
								com->TxBuf[1] = WRITE_REG+0x80;
								com->TxBuf[2] = 2;
								crcValue = GetCRC16CheckCode(com->TxBuf, 3);
								com->TxBuf[3] = crcValue & 0xFF;
								com->TxBuf[4] = (crcValue >> 8) & 0xFF;
								
								com->TxDataCnt = 5;
								
								value = TRACKER_RD_FINISHED;
								if(value!=NO_CONNECT)
								{
									com->TimeOutCnt = 0;
									com->RxDataCnt = 0;

									if(ID==1)
									{
										DMA_Cmd(DMA1_Channel5, DISABLE);
										DMA1_Channel5->CNDTR = BUFFERSIZE;
										DMA_Cmd(DMA1_Channel5, ENABLE);
									}
								}								
								return value;								
							}


							// 生成CRC校验码
							crcValue = GetCRC16CheckCode(com->TxBuf, 6);
							com->TxBuf[6] = crcValue & 0xFF;
							com->TxBuf[7] = crcValue >> 8;
							
							com->TxDataCnt = 8;
							if(com->RxBuf[0] == 0x00)
							{
								com->TxDataCnt = 0;
							}
							
							value = TRACKER_WR_FINISHED;
						}
						else
						{
							value = TRACKER_WR_ERROR;
						}
					}
					break;
					
				case WRITE_S_REG:	// 写单个寄存器
					
					if(com->RxDataCnt>=8 && (com->RxBuf[0]==tracker.DeviceID || com->RxBuf[0]==0))
					{
						crcValue = GetCRC16CheckCode(com->RxBuf,6);
						if((crcValue&0xFF)==com->RxBuf[6] && ((crcValue>>8)&0xFF)==com->RxBuf[7])  // CRC校验正确
						{						
							//返回命令填充
							com->TxBuf[0] = com->RxBuf[0];  
							com->TxBuf[1] = com->RxBuf[1];
							com->TxBuf[2] = com->RxBuf[2];
							com->TxBuf[3] = com->RxBuf[3];  
							com->TxBuf[4] = com->RxBuf[4];  
							com->TxBuf[5] = com->RxBuf[5]; 
							com->TxBuf[6] = com->RxBuf[6];  
							com->TxBuf[7] = com->RxBuf[7];  
						
							crcValue = com->RxBuf[2];
							crcValue <<= 8;
							crcValue |= com->RxBuf[3];
							
							if(crcValue >= 29008 && crcValue <= 29019)//阳光接口地址（写）
							{
								if(crcValue==29008)
								{
									temp16Value = com->RxBuf[4];
									temp16Value <<= 8;
									temp16Value |= com->RxBuf[5];
									
									if(temp16Value==0)
									{//维护模式
										tracker.ExInterface.Mode = 0;
										tracker.SysWorkMode = 0x00000040;
									}
									else if(temp16Value==1)
									{//自动模式
										tracker.ExInterface.Mode = 1;
										tracker.SysWorkMode = 0x00000020;										
									}
									else if(temp16Value==2)
									{//手动模式
										tracker.ExInterface.Mode = 2;
										tracker.SysWorkMode = 0x00000010;										
									}
									else if(temp16Value==3)
									{//大风
										tracker.ExInterface.Mode = 3;
										tracker.SysWorkMode = 0x00000024;										
									}
									else if(temp16Value==4)
									{//除雪
										tracker.ExInterface.Mode = 4;
										tracker.SysWorkMode = 0x00000022;										
									}
									else if(temp16Value==5)
									{//雨天
										tracker.ExInterface.Mode = 5;
										tracker.SysWorkMode = 0x00000021;										
									}
									else if(temp16Value==6)
									{//停止
										tracker.ExInterface.Mode = 6;
										tracker.SysWorkMode = 0;
									}
									else
									{//数据超范围
										
									}									
								}
								else if(crcValue==29009)
								{//风速
									temp16Value = com->RxBuf[4];
									temp16Value <<= 8;
									temp16Value |= com->RxBuf[5];
									tracker.ExInterface.Wspeed = temp16Value;
									
									tracker.Sensor.RemoteWindSpeed = temp16Value;									
								}
//								else if(crcValue==29010)
//								{//风向预留
//									
//								}
								else if(crcValue>=29012 && crcValue<=29017)
								{
									if(flag.BitFlag.RemoteTimeUpdate_S == 0)
									{
										tempTime.Year   = tracker.SysTime.Year;
										tempTime.Month  = tracker.SysTime.Month;
										tempTime.Day    = tracker.SysTime.Day;
										tempTime.Hour   = tracker.SysTime.Hour;
										tempTime.Minute = tracker.SysTime.Minute;
										tempTime.Second = tracker.SysTime.Second;
									}										
									if(crcValue==29012)
									{//年
									   tempTime.Year = com->RxBuf[4];
									   tempTime.Year <<= 8;
									   tempTime.Year |= com->RxBuf[5];										
									}
									else if(crcValue==29013)
									{//月
									   tempTime.Month = com->RxBuf[4];
									   tempTime.Month <<= 8;
									   tempTime.Month |= com->RxBuf[5];											
									}
									else if(crcValue==29014)
									{//日
									   tempTime.Day = com->RxBuf[4];
									   tempTime.Day <<= 8;
									   tempTime.Day |= com->RxBuf[5];	
									}
									else if(crcValue==29015)
									{//时
									   tempTime.Hour = com->RxBuf[4];
									   tempTime.Hour <<= 8;
									   tempTime.Hour |= com->RxBuf[5];	
									}
									else if(crcValue==29016)
									{//分
									   tempTime.Minute = com->RxBuf[4];
									   tempTime.Minute <<= 8;
									   tempTime.Minute |= com->RxBuf[5];											
									}	
									else if(crcValue==29017)
									{//秒
									   tempTime.Second = com->RxBuf[4];
									   tempTime.Second <<= 8;
									   tempTime.Second |= com->RxBuf[5];											
									}
									
									flag.BitFlag.RemoteTimeUpdate_S = 1;
									//flag.BitFlag.TimeUpdate = 1;									
								}
								else if(crcValue == 29018)
								{//目标转角
									
								}
								else if(crcValue == 29019)
								{//目标倾角
									
								}
							}
						  
							else if(crcValue>=40006 && crcValue<=40015)//华为接口地址（写）
							{
								if(crcValue==40006)
								{
									temp16Value = com->RxBuf[4];
									temp16Value <<= 8;
									temp16Value |= com->RxBuf[5];
									
									if(temp16Value==0)
									{//维护模式
										tracker.ExInterface.Mode = 0;
										tracker.SysWorkMode = 0x00000040;
									}
									else if(temp16Value==1)
									{//自动模式
										tracker.ExInterface.Mode = 1;
										tracker.SysWorkMode = 0x00000020;										
									}
									else if(temp16Value==2)
									{//手动模式
										tracker.ExInterface.Mode = 2;
										tracker.SysWorkMode = 0x00000010;										
									}
									else if(temp16Value==3)
									{//大风
										tracker.ExInterface.Mode = 3;
										tracker.SysWorkMode = 0x00000024;										
									}
									else if(temp16Value==4)
									{//除雪
										tracker.ExInterface.Mode = 4;
										tracker.SysWorkMode = 0x00000022;										
									}
									else if(temp16Value==5)
									{//雨天
										tracker.ExInterface.Mode = 5;
										tracker.SysWorkMode = 0x00000021;										
									}
									else if(temp16Value==6)
									{//停止
										tracker.ExInterface.Mode = 6;
										tracker.SysWorkMode = 0;
									}
									else if(temp16Value == 7)
									{//阴雨（切换到维护模式）
										tracker.ExInterface.Mode = 7;
										tracker.SysWorkMode = 0x00000040;
									}
									else
									{//数据超范围
										
									}									
								}
								else if(crcValue==40007)
								{//风速
									temp16Value = com->RxBuf[4];
									temp16Value <<= 8;
									temp16Value |= com->RxBuf[5];
									tracker.ExInterface.Wspeed = temp16Value;
									
									tracker.Sensor.RemoteWindSpeed = temp16Value;									
								}
//								else if(crcValue==40008)
//								{//风向预留
//									
//								}
								else if(crcValue>=40010 && crcValue<=40015)
								{
									if(flag.BitFlag.RemoteTimeUpdate_S == 0)
									{
										tempTime.Year = tracker.SysTime.Year;
										tempTime.Month = tracker.SysTime.Month;
										tempTime.Day  =  tracker.SysTime.Day;
										tempTime.Hour = tracker.SysTime.Hour;
										tempTime.Minute = tracker.SysTime.Minute;
										tempTime.Second = tracker.SysTime.Second;
									}										
									if(crcValue==40010)
									{//年
									   tempTime.Year = com->RxBuf[4];
									   tempTime.Year <<= 8;
									   tempTime.Year |= com->RxBuf[5];										
									}
									else if(crcValue==40011)
									{//月
									   tempTime.Month = com->RxBuf[4];
									   tempTime.Month <<= 8;
									   tempTime.Month |= com->RxBuf[5];											
									}
									else if(crcValue==40012)
									{//日
									   tempTime.Day = com->RxBuf[4];
									   tempTime.Day <<= 8;
									   tempTime.Day |= com->RxBuf[5];	
									}
									else if(crcValue==40013)
									{//时
									   tempTime.Hour = com->RxBuf[4];
									   tempTime.Hour <<= 8;
									   tempTime.Hour |= com->RxBuf[5];	
									}
									else if(crcValue==40014)
									{//分
									   tempTime.Minute = com->RxBuf[4];
									   tempTime.Minute <<= 8;
									   tempTime.Minute |= com->RxBuf[5];											
									}	
									else if(crcValue==40015)
									{//秒
									   tempTime.Second = com->RxBuf[4];
									   tempTime.Second <<= 8;
									   tempTime.Second |= com->RxBuf[5];											
									}
									
									flag.BitFlag.RemoteTimeUpdate_S = 1;
									
									//flag.BitFlag.TimeUpdate = 1;									
								}
//								else if(crcValue==50013)
//								{//启停
//									temp16Value = com->RxBuf[4];
//									temp16Value <<= 8;
//									temp16Value |= com->RxBuf[5];
//									tracker.ExInterface.Tswtich = temp16Value;									
//									if(temp16Value==0)
//									{
//										//tracker.ExInterface.Tswtich = temp16Value;
//										tracker.ExInterface.Mode = 1;
//										tracker.SysWorkMode = 0x00000020;											
//									}
//									else
//									{
//										tracker.ExInterface.Mode = 6;
//										tracker.SysWorkMode = 0;
//									}									
//									
//								}
								else
								{//无效地址
									com->TxBuf[0] = tracker.DeviceID;
									com->TxBuf[1] = WRITE_S_REG+0x80;
									com->TxBuf[2] = 2;
									crcValue = GetCRC16CheckCode(com->TxBuf, 3);
									com->TxBuf[3] = crcValue & 0xFF;
									com->TxBuf[4] = (crcValue >> 8) & 0xFF;
									
									com->TxDataCnt = 5;
									
									value = TRACKER_RD_FINISHED;
									if(value!=NO_CONNECT)
									{
										com->TimeOutCnt = 0;
										com->RxDataCnt = 0;
									
										if(ID==1)
										{
											DMA_Cmd(DMA1_Channel5, DISABLE);
											DMA1_Channel5->CNDTR = BUFFERSIZE;
											DMA_Cmd(DMA1_Channel5, ENABLE);
										}
									}								
									return value;									 
								}
							}
							else
							{//无效地址
								com->TxBuf[0] = tracker.DeviceID;
								com->TxBuf[1] = WRITE_S_REG+0x80;
								com->TxBuf[2] = 2;
								crcValue = GetCRC16CheckCode(com->TxBuf, 3);
								com->TxBuf[3] = crcValue & 0xFF;
								com->TxBuf[4] = (crcValue >> 8) & 0xFF;
				
								com->TxDataCnt = 5;
								
								value = TRACKER_RD_FINISHED;
								if(value!=NO_CONNECT)
								{
									com->TimeOutCnt = 0;
									com->RxDataCnt = 0;

									if(ID==1)
									{
										DMA_Cmd(DMA1_Channel5, DISABLE);
										DMA1_Channel5->CNDTR = BUFFERSIZE;
										DMA_Cmd(DMA1_Channel5, ENABLE);
									}
								}								
								return value;										
							}	
							com->TxDataCnt = 8;
							if(com->RxBuf[0] == 0x00)
							{
								com->TxDataCnt = 0;
							}
							
							value = TRACKER_WR_FINISHED;
						}
						else
						{
							value = TRACKER_WR_ERROR;							
						}
					}
					break;
                case START_UPDATE:
                    if(com->RxDataCnt==6 && (com->RxBuf[0]==tracker.DeviceID || com->RxBuf[0]==0))
					{
						crcValue = GetCRC16CheckCode(com->RxBuf,4);
						if((crcValue&0xFF)==com->RxBuf[4] && ((crcValue>>8)&0xFF)==com->RxBuf[5])  // CRC校验正确
                        {
                            if((IAP_TZ_DC&0xFF)==com->RxBuf[3] && ((IAP_TZ_DC>>8)&0xFF)==com->RxBuf[2])//设备类型正确
                            {
                                IAP_Enter_Process();
                            
                                com->TxDataCnt = 0;
                            
                                value = IAP_PACKAGE_RECEIVED;
                            }
                            else
                            {
                                value = IAP_DEVICE_TYPE_ERROR;
                            }
                        }
                        else
                        {
                            value = IAP_PACKAGE_ERROR;
                        }
                    }
                    break;
                    
                //lora
                case 0xA5:
                    for(i=0;i<6;i++)
                    {
                        com->TxBuf[i] = com->RxBuf[2+i];
                        com->TxDataCnt = 6;
                        value = TRACKER_RD_FINISHED;
                        com->TimeOutCnt = 0;
                        com->RxDataCnt = 0;
                        if(value!= NO_CONNECT)
                        {
                            com->TimeOutCnt = 0;
                            com->RxDataCnt = 0;
                        }
                    }
                    break;
                    
            }
        }
    }

    //==========================================================================
    if(value!=NO_CONNECT)
    {
        com->TimeOutCnt = 0;
        com->RxDataCnt = 0;

        if(ID==1)
        {
            DMA_Cmd(DMA1_Channel5, DISABLE);
            DMA1_Channel5->CNDTR = BUFFERSIZE;
            DMA_Cmd(DMA1_Channel5, ENABLE);
        }
    }

    return value;
}





unsigned char Data_transparent(void)
{
    unsigned short i = 0;
    unsigned char value;
    
    
    if(r485.Uart.RxDataCnt>=1)
    {
        if(r485.Uart.TimeOutCnt++>30)
        {
            r485.Uart.TimeOutCnt = 0;
            value = LONG_NO_CONNECT;
        }
        
        if(r485.Uart.RxDataCnt>=3)
        {
            if(r485.Uart.RxBuf[0]==0xAF && r485.Uart.RxBuf[1]==0xAF)
            {
                for(i=0;i<=r485.Uart.RxDataCnt;i++)
                    set_buf[i] = r485.Uart.RxBuf[i];
            
                trans_len = r485.Uart.RxDataCnt;
                r485.Uart.RxDataCnt = 0;
                trans_flag = 1;
            }
            else
            {
//                LORA_MD0(1);
//                LORA_MD1(1);
//                OSTimeDly(1);
                value = Data_Analyze(&r485.Uart, 2);
                return value;
            }
        }
    }
    
    if(trans_flag == 1)
    {
        trans_flag = 0;
        trans_ret_flag = 1;
        if(trans_len> 0)
        {
            if(set_buf[0] >= 0xc0 && set_buf[0]<= 0xc5)
            {
                for(i=0;i<trans_len;i++)
                {
                    USART_SendData(USART3, set_buf[i]);
                    while(USART_GetFlagStatus(USART3, USART_FLAG_TXE)!=SET);
                }
                while(USART_GetFlagStatus(USART3,USART_FLAG_TC)!=SET);
                // COM_SentData(&r485,set_buf,trans_len);			
                // COM_SentData(&r485,com->RxBuf,com->RxDataCnt);							 
            }
        }
    }
    return NO_CONNECT;
}


