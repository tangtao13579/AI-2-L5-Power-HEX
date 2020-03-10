#include "ASProtocol.h"
#include "HWProtocol.h"
#include "GlobalDefine.h"
#include "ProtocolAnalysis.h"
extern GlobalVariableDef GlobalVariable;

static void   (*(*ASp))(unsigned char R_or_RW, unsigned short *value);
static void   (*(*HWp))(unsigned char R_or_RW, unsigned short *value);

void ProtocolInit()
{
    ASProtocolInit();
    ASp = GetASProtocl();
	
	HWProtocolInit();
    HWp = GetHWProtocl();
}
void LEDFlash(void);
unsigned short ProtocolAnalysis(unsigned char *read_buffer, unsigned char *send_buffer, unsigned short read_num_of_bytes)
{
    unsigned short start_register;
    unsigned short stop_register;

    unsigned short num_of_register;
    unsigned short reply_num_of_bytes;
    unsigned short temp_data;
    unsigned short i;
    
    switch(read_buffer[1])
    {
        case 0x03:
		{
            start_register = read_buffer[2];
            start_register <<= 8;
            start_register |= read_buffer[3];
            num_of_register = read_buffer[4];
            num_of_register <<= 8;
            num_of_register |= read_buffer[5];
            stop_register = start_register + num_of_register - 1;
			if(stop_register<40000)
			{
				if(stop_register >= 113)
				{
					stop_register = 113;
					num_of_register = stop_register - start_register + 1;
				}
			}
            send_buffer[0] = GlobalVariable.ConfigPara.ComID;
            send_buffer[1] = 0x03;
            send_buffer[2] = num_of_register * 2 ;
            if((start_register<= 113) && (stop_register) <= 113)
            {	
                for(i = start_register; i <= stop_register;i++)
                {
                    temp_data = 0;
                    if(*(ASp + i) != (void *)0)
                    {
                        (*(*(ASp + i)))(0,&temp_data); 		   /*Process*/
                        send_buffer[(i-start_register)*2+3] = (temp_data>>8)&0xFF;
                        send_buffer[(i-start_register)*2+4] = temp_data&0xFF;
                    }
                }
                reply_num_of_bytes = send_buffer[2] +3;
            }
            else if((start_register>=40000) && (stop_register <= 40025))
            {	
				for(i = start_register; i <= stop_register;i++)
                {
                    temp_data = 0;
                    if(*(HWp + (i-40000)) != (void *)0)
                    {
                        (*(*(HWp + (i-40000))))(0,&temp_data); 			/*Process*/
                        send_buffer[(i-start_register)*2+3] = (temp_data>>8)&0xFF;
                        send_buffer[(i-start_register)*2+4] = temp_data&0xFF;
                    }
                }
                reply_num_of_bytes = send_buffer[2] +3;
            }
			else
			{
				reply_num_of_bytes = 0;
			}
            break;
		}
		case 0x06:
		{
            start_register = read_buffer[2];
            start_register <<= 8;
            start_register |= read_buffer[3];
            send_buffer[0] = GlobalVariable.ConfigPara.ComID;
            send_buffer[1] = 0x06;
            send_buffer[2] = read_buffer[2];
            send_buffer[3] = read_buffer[3];
            send_buffer[4] = read_buffer[4];
            send_buffer[5] = read_buffer[5];
            
			if(start_register>= 40000)
            {

				temp_data = 0;
				if(*(HWp + (start_register-40000)) != (void *)0)
				{
					temp_data = read_buffer[4];
					temp_data <<= 8;
					temp_data |= read_buffer[5];
					(*(*(HWp + (start_register-40000))))(1,&temp_data);
				}
                reply_num_of_bytes = 6;
            }
            else
            {
                reply_num_of_bytes = 0;
            }
            break;
		}
        case 0x10:
		{
            if(read_num_of_bytes <= 8)
            {
                reply_num_of_bytes = 0;
                break;
            }
            start_register = read_buffer[2];
            start_register <<= 8;
            start_register |= read_buffer[3];
            num_of_register = read_buffer[4];
            num_of_register <<= 8;
            num_of_register |= read_buffer[5];
            stop_register = start_register+num_of_register-1;
            
            send_buffer[0] = GlobalVariable.ConfigPara.ComID;
            send_buffer[1] = 0x10;
            send_buffer[2] = read_buffer[2];
            send_buffer[3] = read_buffer[3];
            send_buffer[4] = read_buffer[4];
            send_buffer[5] = read_buffer[5];
            if((start_register<= 113) && (stop_register) <= 113)
            {
                for(i = start_register; i <= stop_register;i++)
                {
                    temp_data = 0;
                    if(*(ASp + i) != (void *)0)
                    {
                        temp_data = read_buffer[(i-start_register)*2+7];
                        temp_data <<= 8;
                        temp_data |= read_buffer[(i-start_register)*2+8];
                        (*(*(ASp + i)))(1,&temp_data);
                    }
                }
                reply_num_of_bytes = 6;
            }
			else if((start_register>= 40000) && (stop_register) <= 40025)
            {
                for(i = start_register; i <= stop_register;i++)
                {
                    temp_data = 0;
                    if(*(HWp + (i-40000)) != (void *)0)
                    {
                        temp_data = read_buffer[(i-start_register)*2+7];
                        temp_data <<= 8;
                        temp_data |= read_buffer[(i-start_register)*2+8];
                        (*(*(HWp + (i-40000))))(1,&temp_data);
                    }
                }
                reply_num_of_bytes = 6;			//LEDFlash();
            }
            else
            {
                reply_num_of_bytes = 0;
            }
            break;
		}
        case 0x64:
		{
            temp_data = read_buffer[2];
            temp_data <<= 8;
            temp_data |= read_buffer[3];
            if((temp_data == GlobalVariable.FixePara.DeviceType) && (GlobalVariable.WorkMode.SystemStatus == 0))
            {
                GlobalVariable.WorkMode.SystemStatus  = 1;   /*Flash wipe*/
                GlobalVariable.IAPUpdateSys.UpdateTimeOut = 0;
                GlobalVariable.IAPUpdateSys.PackNumber = 0;
            }
            reply_num_of_bytes = 0;
            break;
		}
        case 0x65:
		{
            if((read_num_of_bytes == 262) && (GlobalVariable.WorkMode.SystemStatus == 2))
            {
                GlobalVariable.IAPUpdateSys.UpdateTimeOut = 0;
                temp_data = read_buffer[2];
                temp_data <<= 8;
                temp_data |= read_buffer[3];
                /*PackNumber is the expected packet number*/
                if(GlobalVariable.IAPUpdateSys.PackNumber == temp_data)    
                {
                    for(i = 0; i < 256; i++)
                    {
                        GlobalVariable.IAPUpdateSys.UpdateBuffer[i] = read_buffer[i+4];
                    }
                    GlobalVariable.WriteFlag.UpdatePackWrite = 1;
                }
            }
            reply_num_of_bytes = 0;
            break;
		}
        case 0x66:
		{
            if((read_num_of_bytes == 6) && (GlobalVariable.WorkMode.SystemStatus == 2))
            {
                GlobalVariable.IAPUpdateSys.UpdateTimeOut = 0;
                temp_data = read_buffer[2];
                temp_data <<= 8;
                temp_data |= read_buffer[3];
                if(GlobalVariable.IAPUpdateSys.PackNumber == temp_data)
                {
                    GlobalVariable.WorkMode.SystemStatus = 3;
                }
            }
            reply_num_of_bytes = 0;
            break;
		}
        default:
		{
            reply_num_of_bytes = 0;
            break;
		}
    }

    return reply_num_of_bytes;
}
