#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebServer.hpp>
#include "AppData.hpp"
#include "html_templates.h"
#include "trianglify.min.js-gz.h"

WebServer webServer;

static AsyncWebServer server(80);

/*
 * Web Server pages:
 * 
 */

/* Handle page not founds. */
void Web_PageNotFound( AsyncWebServerRequest *request )
{
  request->send(404);
}

/* Root Page: / */

void Web_RootPage( AsyncWebServerRequest *request )
{
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->printf( TEMPLATE_HEADER,  appData.getFWVersion().c_str(), \
                                      appData.getDevIP().c_str(), \
                                      appData.getGWIP().c_str(), \
                                      appData.getSSID().c_str(), \
                                      appData.getRSSI().c_str(), \
                                      appData.getLogServerIPInfo().c_str(), \
                                      appData.getHeap().c_str(), \
                                      appData.getVoltage().c_str(), \
                                      appData.getCurrent().c_str(), \
                                      appData.getPower().c_str(), \
                                      appData.getEnergy().c_str(), \
                                      appData.getSamplesOK().c_str(), \
                                      appData.getSamplesNOK().c_str(), \
                                      appData.getPZEMState().c_str() );

  response->print( TEMPLATE_FOOTER);

  request->send(response);
}


/*
 * WebServer Class functions.
 */

WebServer::WebServer()
{ 
  initialized = false;
}

void WebServer::setup()
{
    server.on( "/", HTTP_GET, Web_RootPage );

    server.onNotFound( Web_PageNotFound );

    server.on("/trianglify.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript",
            trianglify_min_js_gz, sizeof(trianglify_min_js_gz));
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });

  server.begin();

  initialized = true;

}

void WebServer::handle()
{
  if( ! initialized )
  {
    setup();
  }
}

