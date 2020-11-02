//
//Modyfied 2020.05.05 by APruszkowski example of helloworld_tx for ObirRF24Network
//
/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Update 2014 - TMRh20
 */

#include <ObirRF24Network.h>
#include <ObirRF24.h>
#include <SPI.h>

#include "ObirFeatures.h"

ObirRF24 radio(7,8);                 // nRF24L01(+) radio attached using Getting Started board 

ObirRF24Network network(radio);      // Network uses that radio

//const uint16_t this_node = 01;     // Address of our node in Octal format
//const uint16_t other_node = 00;    // Address of the other node in Octal format
const uint16_t this_node = 0xAABB;   // Address of our node in Octal format
const uint16_t other_node = 0xCCDD;  // Address of the other node in Octal format

const unsigned long interval = 2000; //ms  // How often to send 'hello world to the other unit

unsigned long last_sent;             // When did we last send?
unsigned long packets_sent;          // How many have we sent already

struct payload_t {                   // Structure of our payload
  unsigned long ms;
  unsigned long counter;
};

void setup(void){
    Serial.begin(115200);
    Serial.print(F("ObirRF24Network TX init... "));
 
    SPI.begin();
    radio.begin();
    network.begin(/*channel*/ 90, /*node address*/ this_node);
}

void loop() {  
    network.update();                          // Check the network regularly
 
    unsigned long now = ObirMilis();           // If it's time to send a message, send it!
    if ( now - last_sent >= interval){
        int i;
        last_sent = now;

        Serial.print("Sending...");
        unsigned long d=millis();
        payload_t payload = { d, packets_sent };
        ObirRF24NetworkHeader header(/*to node*/ other_node);
        bool ok=network.write(header, &payload, sizeof(payload));
        if(ok){
            Serial.print(F("ok. (0x"));Serial.print(packets_sent, HEX);Serial.print(F(", "));Serial.print(d);Serial.println(F(")"));
            packets_sent++;
        }else{
            Serial.println(F("failed."));
        }
    }
}
