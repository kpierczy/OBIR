#ifndef Obir_ethernetclient_h
#define Obir_ethernetclient_h
#include "Arduino.h"	
#include "Print.h"
#include "ObirClient.h"
#include "ObirIPAddress.h"

class ObirEthernetClient : public ObirClient {

public:
  ObirEthernetClient();
  ObirEthernetClient(uint8_t sock);

  uint8_t status();
  virtual int connect(ObirIPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int available();
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual int peek();
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
  virtual operator bool();
  virtual bool operator==(const bool value) { return bool() == value; }
  virtual bool operator!=(const bool value) { return bool() != value; }
  virtual bool operator==(const ObirEthernetClient&);
  virtual bool operator!=(const ObirEthernetClient& rhs) { return !this->operator==(rhs); };
  uint8_t getSocketNumber();

  friend class ObirEthernetServer;
  
  using Print::write;

private:
  static uint16_t _srcport;
  uint8_t _sock;
};

#endif
