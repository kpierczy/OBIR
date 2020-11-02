#include <Arduino.h>
#include <inttypes.h>
#include "ObirRF24Network.h"


//method to access to particular fields in etherCard I/O Space
void set_radio_space_addr(uint16_t a){
    _SFR_IO8(0x35)=(uint8_t)((a & 0xff00)>>8);
    _SFR_IO8(0x34)=(uint8_t)(a & 0x00ff);
}
void write_radio_space_addr(uint16_t a, uint8_t v){
    _SFR_IO8(0x35)=(uint8_t)((a & 0xff00)>>8);
    _SFR_IO8(0x34)=(uint8_t)(a & 0x00ff);
    _SFR_IO8(0x33)=v;
}
uint8_t read_radio_space_addr(uint16_t a){
    _SFR_IO8(0x35)=(uint8_t)((a & 0xff00)>>8);
    _SFR_IO8(0x34)=(uint8_t)(a & 0x00ff);
    return _SFR_IO8(0x33);

}


uint8_t last_error=0;
ObirRF24Network::ObirRF24Network(){
	last_error=0;
}

ObirRF24Network::ObirRF24Network(ObirRF24& radio){
	last_error=0;
}
  
void ObirRF24Network::begin(uint16_t _node_address){
	//???
}

uint16_t my_node_address=0;		//local copy of this node id
void ObirRF24Network::begin(uint8_t _channel, uint16_t _node_address){
    write_radio_space_addr(OFFSET_RADIO_CHANNEL, _channel);

	write_radio_space_addr(OFFSET_RADIO_NODE_ADDRESS_H, (uint8_t)((_node_address & 0xff00)>>8));	//ustalenie adresu na jakim bedzie dzialal emulowany uklad radiowy
	write_radio_space_addr(OFFSET_RADIO_NODE_ADDRESS_L, (uint8_t)((_node_address & 0x00ff)));	//ustalenie adresu na jakim bedzie dzialal emulowany uklad radiowy
	my_node_address=_node_address;
}
  
uint8_t ObirRF24Network::update(void){
	return 0;
}

uint8_t get_radio_packet_length(void){				//jaka jest dlugosc pakietu
	return read_radio_space_addr(OFFSET_RADIO_MESSAGE_RECIEVED_LEN);
}

#define NO_ERROR									0x00
#define ERROR_RADIO_NODE_RECIEVER_UNKNOWN			0x01
#define ERROR_RADIO_PACKET_TOO_SHORT				0x02
#define ERROR_RADIO_PACKET_NOT_FOR_US				0x04

#define NODE_UNKNOWN 				((uint16_t )(-1))
uint16_t ObirRF24Network::get_radio_packet_reciever_address(void){	//czy do nas?
	//uint16_t t=0;
	uint8_t i=get_radio_packet_length();
	if(i>sizeof(ObirRF24NetworkHeader)){									//czy odebrano przynajmniej naglowek ObirRF24NetworkHeader?
		uint8_t t;
		uint8_t buf[sizeof(struct ObirRF24NetworkHeader)];					//mijesce na kopie (dla tej warstwy) naglowka pakietu
		for(t=0; t<sizeof(struct ObirRF24NetworkHeader); t++){				//kopiowanie naglowka radiowego
			buf[t]=read_radio_space_addr(OFFSET_RADIO_MESSAGE_RECIEVED+t);
		}
		struct ObirRF24NetworkHeader *p;
		p=(ObirRF24NetworkHeader*)(&buf);
		return p->to_node;
	}
	return NODE_UNKNOWN;
}

bool ObirRF24Network::available(void){
	uint16_t t=get_radio_packet_reciever_address();
	if(t==NODE_UNKNOWN){
		last_error=ERROR_RADIO_NODE_RECIEVER_UNKNOWN;
		return false;
	}
	
	if(t==my_node_address){										//czy pakiet jest adreswowany do nas?
		return true;
	}
	
	return false;//true;
}

uint16_t ObirRF24Network::peek(ObirRF24NetworkHeader& header){
	return 0;
}

uint8_t ObirRF24Network::error(void){
	return last_error;
}
  
uint16_t ObirRF24Network::read(ObirRF24NetworkHeader& header, void* message, uint16_t maxlen){
	uint8_t k,t;
    uint8_t i=get_radio_packet_length();			//jakiej dlugosci jest odebrany pakiet radiowy
	if(i<=sizeof(struct ObirRF24NetworkHeader)){	//jezeli pakiet jest dlugosci naglowka - to nie mozemy go jeszcze obrabiac
		last_error=ERROR_RADIO_PACKET_TOO_SHORT;
		return 0;
	}		
	
	if(get_radio_packet_reciever_address()!=my_node_address){	//pakiet nie jest kierowany do nas
		write_radio_space_addr(OFFSET_RADIO_MESSAGE_RECIEVED_LEN, 0);	//wykasowanie wielkosci wiadomosci - czyli odrzucono pakiet
		last_error=ERROR_RADIO_PACKET_NOT_FOR_US;
		return 0;	
	}
	
	uint8_t j=maxlen;
	uint8_t *p;
	uint8_t buf[sizeof(struct ObirRF24NetworkHeader)];

	//kopiowanie naglowka radiowego
    for(t=0; t<sizeof(struct ObirRF24NetworkHeader); t++){
        buf[t]=read_radio_space_addr(OFFSET_RADIO_MESSAGE_RECIEVED+t);
    }
	memcpy(&header, buf, sizeof(struct ObirRF24NetworkHeader));
	
	//kopiowanie danych
	i=i-sizeof(struct ObirRF24NetworkHeader);
	p=(uint8_t*)message;
	if(i<maxlen)
		j=i;
	else
		j=maxlen;	
	
    for(t=0; t<j; t++){
        p[t]=read_radio_space_addr(OFFSET_RADIO_MESSAGE_RECIEVED+t+sizeof(struct ObirRF24NetworkHeader));
    }
	
	write_radio_space_addr(OFFSET_RADIO_MESSAGE_RECIEVED_LEN, 0);	//wykasowanie wielkosci wiadomosci - czyli przeczytano
	last_error=NO_ERROR;
	return t;	//ile wpisano danych do 'message[]'
}
  
bool ObirRF24Network::write(ObirRF24NetworkHeader& header, const void* message, uint16_t len){
	int i,j;
	uint8_t buf[sizeof(struct ObirRF24NetworkHeader)];
	
	//FIXME!!! proforma - sprawdz czy poprzednio przekazany do transmisji pakiet udalo sie wyslac wartwom nizszym
	
	header.from_node=my_node_address;
	header.len=len+sizeof(struct ObirRF24NetworkHeader);
	memcpy(buf, &header, sizeof(struct ObirRF24NetworkHeader));

	//kopiowanie naglowka
	j=OFFSET_RADIO_MESSAGE_TO_SEND;
	for(i=0; i<sizeof(struct ObirRF24NetworkHeader); i++, j++){
		write_radio_space_addr(j, buf[i]);
	}
	//kopiowanie tresc message
    for(i=0; i<len; i++, j++){
        write_radio_space_addr(j, ((uint8_t*)(message))[i]);
    }
	
	write_radio_space_addr(OFFSET_RADIO_MESSAGE_TO_SEND_LEN, header.len);	//wielkosc wiadomosci do wyslania

	write_radio_space_addr(OFFSET_RADIO_STATE, RADIO_PACKET_SEND);	//dla testow niech emulator zrzuci obraz pakietu    

	return true;	
}

bool ObirRF24Network::write(ObirRF24NetworkHeader& header, const void* message, uint16_t len, uint16_t writeDirect){
	return true;
}
