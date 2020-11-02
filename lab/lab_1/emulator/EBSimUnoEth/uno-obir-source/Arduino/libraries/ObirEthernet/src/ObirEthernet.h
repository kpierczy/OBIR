#ifndef Obir_ethernet_h
#define Obir_ethernet_h

#include <inttypes.h>
//#include "w5100.h"
#include "ObirIPAddress.h"
#include "ObirEthernetClient.h"
#include "ObirEthernetServer.h"
#include "ObirDhcp.h"

#define MAX_SOCK_NUM 4

class ObirEthernetClass {
private:
  ObirIPAddress _dnsServerAddress;
  ObirDhcpClass* _dhcp;
public:
  static uint8_t _state[MAX_SOCK_NUM];
  static uint16_t _server_port[MAX_SOCK_NUM];
  // Initialise the Ethernet shield to use the provided MAC address and gain the rest of the
  // configuration through DHCP.
  // Returns 0 if the DHCP configuration failed, and 1 if it succeeded
  int begin(uint8_t *mac_address, unsigned long timeout = 60000, unsigned long responseTimeout = 4000);
  void begin(uint8_t *mac_address, ObirIPAddress local_ip);
  void begin(uint8_t *mac_address, ObirIPAddress local_ip, ObirIPAddress dns_server);
  void begin(uint8_t *mac_address, ObirIPAddress local_ip, ObirIPAddress dns_server, ObirIPAddress gateway);
  void begin(uint8_t *mac_address, ObirIPAddress local_ip, ObirIPAddress dns_server, ObirIPAddress gateway, ObirIPAddress subnet);
  int maintain();

  ObirIPAddress localIP();
  ObirIPAddress subnetMask();
  ObirIPAddress gatewayIP();
  ObirIPAddress dnsServerIP();

  friend class ObirEthernetClient;
  friend class ObirEthernetServer;
};

extern ObirEthernetClass ObirEthernet;

#endif
