#ifndef __OBIR_RF24NETWORK_HEADER_H__
#define __OBIR_RF24NETWORK_HEADER_H__

#define uint8_t unsigned char 
#define uint16_t unsigned short 
#define uint32_t unsigned long 

struct ObirRF24NetworkHeader{
  uint16_t from_node; 		/**< Logical address where the message was generated */
  uint16_t to_node; 		/**< Logical address where the message is going */
  uint16_t len; 			/**< length of the message */

  ObirRF24NetworkHeader() {}
  //ObirRF24NetworkHeader(uint16_t _to, unsigned char _type = 0): to_node(_to), id(next_id++), type(_type) {}
  ObirRF24NetworkHeader(uint16_t _to, unsigned char _type = 0): to_node(_to) {}
};

#endif // __OBIR_RF24NETWORK_HEADER_H__