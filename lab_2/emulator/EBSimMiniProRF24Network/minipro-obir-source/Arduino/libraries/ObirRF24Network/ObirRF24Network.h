#ifndef __OBIR_RF24NETWORK_H__
#define __OBIR_RF24NETWORK_H__

#define uint8_t unsigned char 
#define uint16_t unsigned short 
#define uint32_t unsigned long 


/*************************** RADIO support *******************************/
#define MAX_RADIO_PACKET                    128
//
#define OFFSET_RADIO_STATE                  (0)                                     //1B
#define OFFSET_RADIO_CHANNEL                (OFFSET_RADIO_STATE+1)                  //1B
#define OFFSET_RADIO_NODE_ADDRESS_H         (OFFSET_RADIO_CHANNEL+1)                //1B
#define OFFSET_RADIO_NODE_ADDRESS_L         (OFFSET_RADIO_NODE_ADDRESS_H+1)         //1B
#define OFFSET_RADIO_MESSAGE_TO_SEND_LEN    (OFFSET_RADIO_NODE_ADDRESS_L+1)         //1B
#define OFFSET_RADIO_MESSAGE_TO_SEND        (OFFSET_RADIO_MESSAGE_TO_SEND_LEN+1)    //128B
#define OFFSET_RADIO_MESSAGE_RECIEVED_LEN   ((OFFSET_RADIO_MESSAGE_TO_SEND)+(MAX_RADIO_PACKET)) //1B
#define OFFSET_RADIO_MESSAGE_RECIEVED       (OFFSET_RADIO_MESSAGE_RECIEVED_LEN+1)   //128B
//...ostatnie pole
#define OFFSET_REST                         ((OFFSET_RADIO_MESSAGE_RECIEVED)+(MAX_RADIO_PACKET))
//
#define RADIO_PACKET_IDLE                   (0)
#define RADIO_PACKET_SEND                   (1)
#define RADIO_PACKET_DUMP                   (255)

//
void write_radio_space_addr(uint16_t a, uint8_t v);


#include "ObirRF24NetworkHeader.h"
#include "ObirRF24.h"

class ObirRF24Network{
public:
	ObirRF24Network(ObirRF24 &radio);
	ObirRF24Network();
  
	void begin(uint16_t _node_address);
	void begin(uint8_t _channel, uint16_t _node_address);    
  
	uint8_t update(void);
  
	bool available(void);
	uint16_t peek(ObirRF24NetworkHeader& header);
  
	uint16_t read(ObirRF24NetworkHeader& header, void* message, uint16_t maxlen);
  
	bool write(ObirRF24NetworkHeader& header,const void* message, uint16_t len);
	bool write(ObirRF24NetworkHeader& header,const void* message, uint16_t len, uint16_t writeDirect);

	uint8_t error(void);
	uint16_t get_radio_packet_reciever_address(void);

private:
};
#endif // __OBIR_RF24NETWORK_H__

