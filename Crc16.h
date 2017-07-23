#ifndef __Crc16_h__
#define __Crc16_h__


int Crc16(unsigned char *crc,unsigned char *puchMsg, int usDataLen);
int CHK_Crc16(unsigned char *crc,  unsigned char *puchMsg,  int usDataLen);

#endif
