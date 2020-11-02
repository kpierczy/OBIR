/*
 *  Udp.cpp: Library to send/receive UDP packets with the Arduino ethernet shield.
 *  This version only offers minimal wrapping of socket.c/socket.h
 *  Drop Udp.h/.cpp into the Ethernet library directory at hardware/libraries/Ethernet/ 
 *
 * MIT License:
 * Copyright (c) 2008 Bjoern Hartmann
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * bjoern@cs.stanford.edu 12/30/2008
 */

//#include "utility/w5100.h"
//#include "utility/socket.h"
#include "ObirEthernet.h"
#include "ObirUdp.h"
#include "ObirDns.h"

#include "Obir.h"

/* Constructor */
ObirEthernetUDP::ObirEthernetUDP() : _sock(MAX_SOCK_NUM) {}

/* Start EthernetUDP socket, listening at local port PORT */
uint8_t ObirEthernetUDP::begin(uint16_t port) {
//  if (_sock != MAX_SOCK_NUM)
//    return 0;
//
//  for (int i = 0; i < MAX_SOCK_NUM; i++) {
//    uint8_t s = socketStatus(i);
//    if (s == SnSR::CLOSED || s == SnSR::FIN_WAIT) {
//      _sock = i;
//      break;
//    }
//  }
//
//  if (_sock == MAX_SOCK_NUM)
//    return 0;
//
//  _port = port;
//  _remaining = 0;
//  socket(_sock, SnMR::UDP, _port, 0);
    write_ether_space_addr(OFFSET_LOCAL_PORT,   (uint8_t)((port & 0xff00)>>8));
    write_ether_space_addr(OFFSET_LOCAL_PORT+1, (uint8_t)(port & 0x00ff));

    write_ether_space_addr(OFFSET_BUFFER_STATE, PACKET_UDP_LISTEN);	//wskaz aby system zaczol nasluchiwac pakiety

    return 1;   //FIXME!!! nie zawsze sie uda - sprawdzic czy aby port lokalny nie jest zajety

  return 1;
}

/* return number of bytes available in the current packet,
   will return zero if parsePacket hasn't been called yet */
int ObirEthernetUDP::available() {
 // return _remaining;
	return parsePacket();
}

/* Release any resources being used by this EthernetUDP instance */
void ObirEthernetUDP::stop()
{
//  if (_sock == MAX_SOCK_NUM)
//    return;
//
//  close(_sock);
//
//  ObirEthernetClass::_server_port[_sock] = 0;
//  _sock = MAX_SOCK_NUM;
}

int ObirEthernetUDP::beginPacket(const char *host, uint16_t port){
//  // Look up the host first
  int ret = 0;
  ObirDNSClient dns;
  ObirIPAddress remote_addr;

  Serial.println(F("ObirEthernetUDP::beginPacket(const char *host, uint16_t port); is not supported! Sorry!"));
  asm("break\n");	
  return 0;

/*  Serial.print(F("ObirEthernet.dnsServerIP()="));Serial.println(ObirEthernet.dnsServerIP());
  dns.begin(ObirEthernet.dnsServerIP());
  Serial.print(F("ObirEthernetUDP::beginPacket("));Serial.print(host);Serial.print(F(","));Serial.print(port);Serial.println(F(")"));
  ret = dns.getHostByName(host, remote_addr);
  if (ret == 1) {
    Serial.print(F("dns.getHostByName()="));Serial.print(host);Serial.print(F(","));Serial.println(remote_addr);
    return beginPacket(remote_addr, port);
  } else {
    Serial.print(F("dns.getHostByName()=FAIL: "));Serial.println(ret);
    return ret;
  }
*/
}

int ObirEthernetUDP::beginPacket(ObirIPAddress ip, uint16_t port)
{
  //_offset = 0;
  //return startUDP(_sock, rawIPAddress(ip), port);
    uint8_t tt[5];
    tt[0]=ip[0];
    tt[1]=ip[1];
    tt[2]=ip[2];
    tt[3]=ip[3];

    /*Serial.print(F("ObirEthernetUDP::beginPacket("));
    Serial.print(ip);
    Serial.print(F(","));
    Serial.print(port);
    Serial.println(F(")"));*/

    write_ether_space_addr(OFFSET_BUFFER_STATE, PACKET_UDP_START_BUILD);//tworzymy nowy pakiet - system nie kopiouje do bufora nowo otrzymanych paketow UDP 
    
    write_ether_space_addr(OFFSET_REMOTE_IP, tt[0]);
    write_ether_space_addr(OFFSET_REMOTE_IP+1, tt[1]);
    write_ether_space_addr(OFFSET_REMOTE_IP+2, tt[2]);
    write_ether_space_addr(OFFSET_REMOTE_IP+3, tt[3]);

    write_ether_space_addr(OFFSET_REMOTE_PORT,   (uint8_t)((port & 0xff00)>>8));
    write_ether_space_addr(OFFSET_REMOTE_PORT+1, (uint8_t)(port & 0x00ff));
    
    return 1;//FIXME!! czy zawsze sie uda???

  return 0;
}

int ObirEthernetUDP::endPacket()
{
  //return sendUDP(_sock);
    write_ether_space_addr(OFFSET_BUFFER_STATE, PACKET_UDP_DUMP);	//dla testow niech emulator zrzuci obraz pakietu    
    write_ether_space_addr(OFFSET_BUFFER_STATE, PACKET_UDP_SEND);	//wyslanie pakietu 


  /*Serial.println(F("ObirEthernetUDP::endPacket()"));*/
  return 0;
}

size_t ObirEthernetUDP::write(uint8_t byte)
{
  return write(&byte, 1);
}

size_t ObirEthernetUDP::write(const uint8_t *buffer, size_t size)
{
  //uint16_t bytes_written = bufferData(_sock, _offset, buffer, size);
  //_offset += bytes_written;
  //return bytes_written;
    uint16_t i,j;
    j=(uint16_t)(read_ether_space_addr(OFFSET_TO_SEND_DATAGRAM_OFFSET))<<8 |
      (uint16_t)(read_ether_space_addr(OFFSET_TO_SEND_DATAGRAM_OFFSET+1));
  
    //FIXME!!! czy aby jest tyle miejsca w buforze 
    //         if(OFFSET_TO_SEND_DATAGRAM_SPACE+j+len<MAX_UDP_DATAGRAM)???
    for(i=0; i<size; i++){
        write_ether_space_addr(OFFSET_TO_SEND_DATAGRAM_SPACE+i+j, buffer[i]);
    }
    //aktualizacja nowego offsetu
    j=j+size;
    write_ether_space_addr(OFFSET_TO_SEND_DATAGRAM_OFFSET, (j & 0xff00)>>8);
    write_ether_space_addr(OFFSET_TO_SEND_DATAGRAM_OFFSET+1, (j & 0x00ff));

  return 0;
}

int ObirEthernetUDP::parsePacket()
{
//  // discard any remaining bytes in the last packet
//  while (_remaining) {
//    // could this fail (loop endlessly) if _remaining > 0 and recv in read fails?
//    // should only occur if recv fails after telling us the data is there, lets
//    // hope the w5100 always behaves :)
//    read();
//  }
//
//  if (recvAvailable(_sock) > 0)
//  {
//    //HACK - hand-parse the UDP packet using TCP recv method
//    uint8_t tmpBuf[8];
//    int ret =0; 
//    //read 8 header bytes and get IP and port from it
//    ret = recv(_sock,tmpBuf,8);
//    if (ret > 0)
//    {
//      _remoteIP = tmpBuf;
//      _remotePort = tmpBuf[4];
//      _remotePort = (_remotePort << 8) + tmpBuf[5];
//      _remaining = tmpBuf[6];
//      _remaining = (_remaining << 8) + tmpBuf[7];
//
//      // When we get here, any remaining bytes are the data
//      ret = _remaining;
//    }
//    return ret;
//  }
//  // There aren't any packets available
//  return 0;
    int i,j;
    //jakiej dlugosci jest pakiet 
    i=(uint16_t)(read_ether_space_addr(OFFSET_RECIEVED_DATAGRAM_LENGTH))<<8 |
      (uint16_t)(read_ether_space_addr(OFFSET_RECIEVED_DATAGRAM_LENGTH+1));
    //dokad doczytano
    j=(uint16_t)(read_ether_space_addr(OFFSET_RECIEVED_DATAGRAM_OFFSET))<<8 |
      (uint16_t)(read_ether_space_addr(OFFSET_RECIEVED_DATAGRAM_OFFSET+1));

    _remoteIP=ObirIPAddress(read_ether_space_addr(OFFSET_REMOTE_IP), 
                            read_ether_space_addr(OFFSET_REMOTE_IP+1), 
                            read_ether_space_addr(OFFSET_REMOTE_IP+2), 
                            read_ether_space_addr(OFFSET_REMOTE_IP+3));
    _remotePort=((read_ether_space_addr(OFFSET_REMOTE_PORT))<<8) | 
                (read_ether_space_addr(OFFSET_REMOTE_PORT+1));


//Serial.print("parsePacket: i-j=");Serial.println(i-j, HEX);delay(1000);
    return i-j;
}

int ObirEthernetUDP::read()
{
//  uint8_t byte;
//
//  if ((_remaining > 0) && (recv(_sock, &byte, 1) > 0))
//  {
//    // We read things without any problems
//    _remaining--;
//    return byte;
//  }
//
//  // If we get here, there's no data available
//  return -1;
    unsigned char b[1];
    if(read(b, 1)<=0)
        return -1;
    return b[0];
}

int ObirEthernetUDP::read(unsigned char* buffer, size_t len)
{
//
//  if (_remaining > 0)
//  {
//
//    int got;
//
//    if (_remaining <= len)
//    {
//      // data should fit in the buffer
//      got = recv(_sock, buffer, _remaining);
//    }
//    else
//    {
//      // too much data for the buffer, 
//      // grab as much as will fit
//      got = recv(_sock, buffer, len);
//    }
//
//    if (got > 0)
//    {
//      _remaining -= got;
//      return got;
//    }
//
//  }
//
//  // If we get here, there's no data available or recv failed
//  return -1;

    uint16_t i,j,blen;
    int nread=0;
    //dokad doczytalismy?
    j=(uint16_t)(read_ether_space_addr(OFFSET_RECIEVED_DATAGRAM_OFFSET))<<8 |
      (uint16_t)(read_ether_space_addr(OFFSET_RECIEVED_DATAGRAM_OFFSET+1));
    //a ile jest w buforze
    blen=(uint16_t)(read_ether_space_addr(OFFSET_RECIEVED_DATAGRAM_LENGTH))<<8 |
         (uint16_t)(read_ether_space_addr(OFFSET_RECIEVED_DATAGRAM_LENGTH+1));

    nread=len;
    if(blen-j<len){     //czy odczyt moze wykroczyc poza dane zapisane w buforze
        nread=blen-j;
    }

    for(i=0; i<nread; i++){
        buffer[i]=read_ether_space_addr(OFFSET_RECIEVED_DATAGRAM_SPACE+i+j);
    }

//Serial.print("j=");Serial.println(j, HEX);Serial.print("blen=");Serial.println(blen, HEX );Serial.print("nread=");Serial.println(nread, HEX );delay(1000);

    //aktualizacja nowego offsetu
    j=j+nread;
    write_ether_space_addr(OFFSET_RECIEVED_DATAGRAM_OFFSET, (j & 0xff00)>>8);
    write_ether_space_addr(OFFSET_RECIEVED_DATAGRAM_OFFSET+1, (j & 0x00ff));
    return nread;
}

int ObirEthernetUDP::peek()
{
  uint8_t b;
//
//  // Unlike recv, peek doesn't check to see if there's any data available, so we must.
//  // If the user hasn't called parsePacket yet then return nothing otherwise they
//  // may get the UDP header
//  if (!_remaining)
//    return -1;
//  ::peek(_sock, &b);

  return b;
}

void ObirEthernetUDP::flush()
{
  // TODO: we should wait for TX buffer to be emptied
    write_ether_space_addr(OFFSET_RECIEVED_DATAGRAM_LENGTH, 0);
    write_ether_space_addr(OFFSET_RECIEVED_DATAGRAM_LENGTH+1, 0);
}

/* Start EthernetUDP socket, listening at local port PORT */
uint8_t ObirEthernetUDP::beginMulticast(ObirIPAddress ip, uint16_t port)
{
//  if (_sock != MAX_SOCK_NUM)
//    return 0;
//
//  for (int i = 0; i < MAX_SOCK_NUM; i++) {
//    uint8_t s = W5100.readSnSR(i);
//    if (s == SnSR::CLOSED || s == SnSR::FIN_WAIT) {
//      _sock = i;
//      break;
//    }
//  }
//
//  if (_sock == MAX_SOCK_NUM)
//    return 0;
//
//  // Calculate MAC address from Multicast IP Address
//  byte mac[] = {  0x01, 0x00, 0x5E, 0x00, 0x00, 0x00 };
//
//  mac[3] = ip[1] & 0x7F;
//  mac[4] = ip[2];
//  mac[5] = ip[3];
//
//  W5100.writeSnDIPR(_sock, rawIPAddress(ip));   //239.255.0.1
//  W5100.writeSnDPORT(_sock, port);
//  W5100.writeSnDHAR(_sock,mac);
//
//  _remaining = 0;
//  socket(_sock, SnMR::UDP, port, SnMR::MULTI);
  return 1;
}


