#include <Arduino.h>
#include <inttypes.h>
#include "Obir.h"

//method to access to particular fields in etherCard I/O Space
void set_ether_space_addr(uint16_t a){
    _SFR_IO8(0x3C)=(uint8_t)((a & 0xff00)>>8);
    _SFR_IO8(0x3B)=(uint8_t)(a & 0x00ff);
}
void write_ether_space_addr(uint16_t a, uint8_t v){
    _SFR_IO8(0x3C)=(uint8_t)((a & 0xff00)>>8);
    _SFR_IO8(0x3B)=(uint8_t)(a & 0x00ff);
    _SFR_IO8(0x3A)=v;
}
uint8_t read_ether_space_addr(uint16_t a){
    _SFR_IO8(0x3C)=(uint8_t)((a & 0xff00)>>8);
    _SFR_IO8(0x3B)=(uint8_t)(a & 0x00ff);
    return _SFR_IO8(0x3A);

}
