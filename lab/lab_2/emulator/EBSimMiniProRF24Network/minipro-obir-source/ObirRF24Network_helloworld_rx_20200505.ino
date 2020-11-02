//
//Modyfied 2020.05.05 by APruszkowski example of helloworld_rx for ObirRF24Network
//
/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Update 2014 - TMRh20
 */

/**
 * Simplest possible example of using RF24Network,
 *
 * RECEIVER NODE
 * Listens for messages from the transmitter and prints them out.
 */


#include <ObirRF24Network.h>
#include <ObirRF24.h>
#include <SPI.h>

ObirRF24 radio(7,8);                // nRF24L01(+) radio attached using Getting Started board 

ObirRF24Network network(radio);      // Network uses that radio

//const uint16_t this_node = 00;    // Address of our node in Octal format ( 04,031, etc)
//const uint16_t other_node = 01;   // Address of the other node in Octal format
const uint16_t this_node = 0xCCDD;  // Address of our node in Octal format
const uint16_t other_node = 0xAABB; // Address of our node in Octal format

struct payload_t {                  // Structure of our payload
  unsigned long ms;
  unsigned long counter;
};

void setup(void){
    Serial.begin(115200);
    Serial.print(F("ObirRF24Network RX init... "));

    SPI.begin();
    radio.begin();
    network.begin(/*channel*/ 90, /*node address*/ this_node);
}

void loop(void){
    network.update();                       // Check the network regularly

    while(network.available()){         // Is there anything ready for us?  
        ObirRF24NetworkHeader header;       // If so, grab it and print it out
        payload_t payload;
        uint8_t n=network.read(header, &payload, sizeof(payload));
        if(n>0){
            Serial.print("Received packet #0x");
            Serial.print(payload.counter, HEX);
            Serial.print(" at ");
            Serial.println(payload.ms);
        }else{
            Serial.print("Recieving error: ");Serial.println(network.error(), HEX);
        }
    }    
}
