#ifndef _PROTOCOL_ANANLYSIS_H_
#define _PROTOCOL_ANANLYSIS_H_
void ProtocolInit(void);
unsigned short ProtocolAnalysis(unsigned char *read_buffer, unsigned char *send_buffer, unsigned short read_num_of_bytes);

#endif
