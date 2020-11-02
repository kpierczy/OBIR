#ifndef _Obir_h
#define _Obir_h

#define MAX_UDP_DATAGRAM                2000

#define OFFSET_BUFFER_STATE             (0)
#define OFFSET_MAC                      (OFFSET_BUFFER_STATE+1)
#define OFFSET_LOCAL_IP                 (OFFSET_MAC+6)
#define OFFSET_LOCAL_PORT               (OFFSET_LOCAL_IP+4)
#define OFFSET_REMOTE_IP                (OFFSET_LOCAL_PORT+2)
#define OFFSET_REMOTE_PORT              (OFFSET_REMOTE_IP+4)
#define OFFSET_TO_SEND_DATAGRAM_OFFSET  (OFFSET_REMOTE_PORT+2)
#define OFFSET_TO_SEND_DATAGRAM_SPACE   (OFFSET_TO_SEND_DATAGRAM_OFFSET+2)
#define OFFSET_RECIEVED_DATAGRAM_LENGTH ((OFFSET_TO_SEND_DATAGRAM_SPACE)+(MAX_UDP_DATAGRAM))
#define OFFSET_RECIEVED_DATAGRAM_OFFSET ((OFFSET_RECIEVED_DATAGRAM_LENGTH)+2)
#define OFFSET_RECIEVED_DATAGRAM_SPACE  (OFFSET_RECIEVED_DATAGRAM_OFFSET+2)
//...ostatnie pole 
#define OFFSET_REST                     ((OFFSET_RECIEVED_DATAGRAM_SPACE)+(MAX_UDP_DATAGRAM))

#define PACKET_UDP_IDLE         (0)
#define PACKET_UDP_START_BUILD  (1)
#define PACKET_UDP_SEND         (2)
#define PACKET_UDP_LISTEN       (3)
#define PACKET_UDP_DUMP         (255)

void set_ether_space_addr(uint16_t a);
void write_ether_space_addr(uint16_t a, uint8_t v);
uint8_t read_ether_space_addr(uint16_t a);

#endif //#ifndef _Obir_h
