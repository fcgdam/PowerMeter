#ifndef __LOG_CLIENT_H__
#define __LOG_CLIENT_H__

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <IPAddress.h>

class LogClient
{
private:
  bool serialEnabled = true;
  bool udpEnabled    = false;
  
  WiFiUDP UDPConnection;

  IPAddress udpServer;
  uint16_t  udpPort;
  uint16_t  localPort = 2390;

  String    tagname;

  void sendUdpMsg( String);

public:
  void setSerial( bool );
  void setUdp ( bool );
  void setServer( IPAddress, uint16_t);
  void setTagName( String );

  // INFO Messages
  void I( const char *);
  void I( String );

  // WARN Messages
  void W( const char *);
  void W( String );

  // ERROR Messages
  void E( const char *);
  void E( String );

};

extern LogClient Log;

#endif