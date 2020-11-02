#ifndef Obir_ethernetserver_h
#define Obir_ethernetserver_h

#include "Server.h"

class ObirEthernetClient;

class ObirEthernetServer : 
public Server {
private:
  uint16_t _port;
  void accept();
public:
  ObirEthernetServer(uint16_t);
  ObirEthernetClient available();
  virtual void begin();
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  using Print::write;
};

#endif
