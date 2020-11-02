#include <Arduino.h>
#include <inttypes.h>

//method to access to particular fields in new_feature I/O Space
uint32_t ObirMilis(){
    uint32_t t;
    t=(uint32_t)(_SFR_IO8(0x36))<<24;			//Fetching a time you must start from SFR register at address 0x36!!!
    t=t | (uint32_t)(_SFR_IO8(0x37))<<16;
    t=t | (uint32_t)(_SFR_IO8(0x38))<<8;
    t=t | (uint32_t)(_SFR_IO8(0x39));
    return t;
}