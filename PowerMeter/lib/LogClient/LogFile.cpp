#include <Arduino.h>
#include <LogClient.hpp>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

// Variable used by all modules to output logging.
LogClient Log;

void LogClient::sendUdpMsg( String msg)
{
    // Only send data if we are connected...
    if ( WiFi.status() == WL_CONNECTED ) {
        unsigned int msg_length = msg.length();
        byte* p = (byte*)malloc(msg_length);
        memcpy(p, (char*) msg.c_str(), msg_length);

        UDPConnection.beginPacket( udpServer, udpPort);
        UDPConnection.write(p, msg_length);
        UDPConnection.endPacket();
        free(p);
    }
}

void LogClient::setTagName(String name ) {
    tagname = name;
}

void LogClient::setSerial(bool enabled ) {
    serialEnabled = enabled;
};

void LogClient::setUdp( bool enabled ) {
    uint8_t tries = 0;

    udpEnabled = enabled;
    while ( (WiFi.status() != WL_CONNECTED) && tries < 10 ) {
        delay( 100 );
        tries++;
        Serial.println("Not connected!!!");
    }
}

void LogClient::setServer( IPAddress ipa , uint16_t port ) {
    udpServer = ipa;
    udpPort = port;
}

void LogClient::I(const char *s ) {
    String msg = String(s);
    msg = "[" + tagname + "][INFO] " + msg + "\n";
    if ( serialEnabled ) Serial.print( msg );
    if ( udpEnabled) sendUdpMsg( msg );
}

void LogClient::I( String s) {
    s = "[" + tagname + "][INFO] " + s + "\n";
    if ( serialEnabled ) Serial.print( s );
    if ( udpEnabled ) sendUdpMsg( s );
} 

void LogClient::W(const char *s ) {
    String msg = String(s);
    msg = "[" + tagname + "][WARN] " + msg + "\n";
    if ( serialEnabled ) Serial.print( msg );
    if ( udpEnabled) sendUdpMsg( msg );
}

void LogClient::W( String s) {
    s = "[" + tagname + "][WARN] " + s + "\n";
    if ( serialEnabled ) Serial.print( s );
    if ( udpEnabled ) sendUdpMsg( s );
} 

void LogClient::E(const char *s ) {
    String msg = String(s);
    msg = "[" + tagname + "][ERROR] " + msg + "\n";
    if ( serialEnabled ) Serial.print( msg );
    if ( udpEnabled) sendUdpMsg( msg );
}

void LogClient::E( String s) {
    s = "[" + tagname + "][ERROR] " + s + "\n";
    if ( serialEnabled ) Serial.print( s );
    if ( udpEnabled ) sendUdpMsg( s );
} 
