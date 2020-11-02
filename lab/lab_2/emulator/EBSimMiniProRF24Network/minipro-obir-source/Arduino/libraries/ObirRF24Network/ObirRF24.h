#ifndef __OBIR_RF24_H__
#define __OBIR_RF24_H__

#define uint8_t unsigned char 
#define uint16_t unsigned short 
#define uint32_t unsigned long 

#include "ObirRF24NetworkHeader.h"

class ObirRF24{
public:
	ObirRF24(uint16_t _cepin, uint16_t _cspin);
	void begin();
};

#endif // __OBIR_RF24_H__