//#include "utility/w5100.h"
#include "ObirEthernet.h"
#include "ObirDhcp.h"

#include "Obir.h"

// XXX: don't make assumptions about the value of MAX_SOCK_NUM.
uint8_t ObirEthernetClass::_state[MAX_SOCK_NUM] = { 
  0, 0, 0, 0 };
uint16_t ObirEthernetClass::_server_port[MAX_SOCK_NUM] = { 
  0, 0, 0, 0 };

int ObirEthernetClass::begin(uint8_t *mac_address, unsigned long timeout, unsigned long responseTimeout)
{
//  static ObirDhcpClass s_dhcp;
//  _dhcp = &s_dhcp;
//
//
//  // Initialise the basic info
//  W5100.init();
//  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
//  W5100.setMACAddress(mac_address);
//  W5100.setIPAddress(ObirIPAddress(0,0,0,0).raw_address());
//  SPI.endTransaction();

//  // Now try to get our config info from a DHCP server
//  int ret = _dhcp->beginWithDHCP(mac_address, timeout, responseTimeout);
//  if(ret == 1)
//  {
//    // We've successfully found a DHCP server and got our configuration info, so set things
//    // accordingly
//    SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
//    W5100.setIPAddress(_dhcp->getLocalIp().raw_address());
//    W5100.setGatewayIp(_dhcp->getGatewayIp().raw_address());
//    W5100.setSubnetMask(_dhcp->getSubnetMask().raw_address());
//    SPI.endTransaction();
//    _dnsServerAddress = _dhcp->getDnsServerIp();
//  }
//
//  return ret;

//FIXME!!!
  _dnsServerAddress=ObirIPAddress(8,8,8,8);
  return 0;
}

void ObirEthernetClass::begin(uint8_t *mac_address, ObirIPAddress local_ip)
{
  // Assume the DNS server will be the machine on the same network as the local IP
  // but with last octet being '1'
  //ObirIPAddress dns_server = local_ip;
  //dns_server[3] = 1;
  //begin(mac_address, local_ip, dns_server);
}

void ObirEthernetClass::begin(uint8_t *mac_address, ObirIPAddress local_ip, ObirIPAddress dns_server)
{
  // Assume the gateway will be the machine on the same network as the local IP
  // but with last octet being '1'
  //ObirIPAddress gateway = local_ip;
  //gateway[3] = 1;
  //begin(mac_address, local_ip, dns_server, gateway);
}

void ObirEthernetClass::begin(uint8_t *mac_address, ObirIPAddress local_ip, ObirIPAddress dns_server, ObirIPAddress gateway)
{
  //ObirIPAddress subnet(255, 255, 255, 0);
  //begin(mac_address, local_ip, dns_server, gateway, subnet);
}

void ObirEthernetClass::begin(uint8_t *mac, ObirIPAddress local_ip, ObirIPAddress dns_server, ObirIPAddress gateway, ObirIPAddress subnet)
{
  //W5100.init();
  //SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  //W5100.setMACAddress(mac);
  //W5100.setIPAddress(local_ip.raw_address());
  //W5100.setGatewayIp(gateway.raw_address());
  //W5100.setSubnetMask(subnet.raw_address());
  //SPI.endTransaction();
  //_dnsServerAddress = dns_server;
}

int ObirEthernetClass::maintain(){
  int rc = DHCP_CHECK_NONE;
  //if(_dhcp != NULL){
  //  //we have a pointer to dhcp, use it
  //  rc = _dhcp->checkLease();
  //  switch ( rc ){
  //    case DHCP_CHECK_NONE:
  //      //nothing done
  //      break;
  //    case DHCP_CHECK_RENEW_OK:
  //    case DHCP_CHECK_REBIND_OK:
  //      //we might have got a new IP.
  //      SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  //      W5100.setIPAddress(_dhcp->getLocalIp().raw_address());
  //      W5100.setGatewayIp(_dhcp->getGatewayIp().raw_address());
  //      W5100.setSubnetMask(_dhcp->getSubnetMask().raw_address());
  //      SPI.endTransaction();
  //      _dnsServerAddress = _dhcp->getDnsServerIp();
  //      break;
  //    default:
  //      //this is actually a error, it will retry though
  //      break;
  //  }
  //}
  return rc;
}

ObirIPAddress ObirEthernetClass::localIP()
{
  ObirIPAddress ret;
  //SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  //W5100.getIPAddress(ret.raw_address());
  //SPI.endTransaction();
  
  ret=ObirIPAddress(read_ether_space_addr(OFFSET_LOCAL_IP), 
                            read_ether_space_addr(OFFSET_LOCAL_IP+1), 
                            read_ether_space_addr(OFFSET_LOCAL_IP+2), 
                            read_ether_space_addr(OFFSET_LOCAL_IP+3));

  
  return ret;
}

ObirIPAddress ObirEthernetClass::subnetMask()
{
  ObirIPAddress ret;
  //SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  //W5100.getSubnetMask(ret.raw_address());
  //SPI.endTransaction();
  return ret;
}

ObirIPAddress ObirEthernetClass::gatewayIP()
{
  ObirIPAddress ret;
  //SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  //W5100.getGatewayIp(ret.raw_address());
  //SPI.endTransaction();
  return ret;
}

ObirIPAddress ObirEthernetClass::dnsServerIP()
{
  return _dnsServerAddress;
}

ObirEthernetClass ObirEthernet;
