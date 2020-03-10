#ifndef _MODBUS_FRAME_H_
#define _MODBUS_FRAME_H_

void ModbusPortInit(unsigned char port_number);
void ModbusSend(unsigned char port_number, unsigned short num_of_byte, unsigned char *SendBuffer);
int ModbusRead(unsigned char port_number, unsigned short *num_of_byte, unsigned char *ReadBuffer);

#endif
