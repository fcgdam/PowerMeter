#ifndef __WEB_SERVER_H__
#define __WEB_SERVER_H__

class WebServer
{
private:
  bool initialized;

public:
  WebServer();

  void setup ();
  void handle();
};

extern WebServer webServer;

#endif