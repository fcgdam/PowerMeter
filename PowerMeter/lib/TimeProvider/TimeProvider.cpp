#include <Arduino.h>
#include <TimeLib.h>
#include <TimeProvider.hpp>
#include <Timezone.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <LogClient.hpp>

const int   NTP_PACKET_SIZE = 48;					// Fixed size of NTP record
uint8_t     packetBuffer[NTP_PACKET_SIZE];
WiFiUDP     Udp;
IPAddress   ntpServer;							    // IP address of NTP_TIMESERVER from DNS Query
String      NTP_TIMESERVER;
int         NTP_TIMEZONES;

TimeProvider timeProvider;

// Define Time Zones here
TimeChangeRule PST = {"PST", Last, Sun, Mar, 1, 60};        //Portuguese Summer Time -  "UTC + 1" or GMT + 1
TimeChangeRule GMT = {"GMT", Last, Sun, Oct, 2, 0};         //Portuguese winter Time -  "UTC + 0" or GMT
Timezone tz(PST, GMT);
TimeChangeRule *tcr;  


// ----------------------------------------------------------------------------
// Send the request packet to the NTP server.
//
// ----------------------------------------------------------------------------
// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:                 
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}


time_t getNtpTime()
{
    while (Udp.parsePacket() > 0) ; // discard any previously received packets

    WiFi.hostByName(NTP_TIMESERVER.c_str() , ntpServer);

    Log.I("NTP Request: " + NTP_TIMESERVER + ": " + ntpServer.toString());
    sendNTPpacket(ntpServer);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE) {
            Log.I("Received NTP Response");
            Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
            unsigned long secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            return secsSince1900 - 2208988800UL + NTP_TIMEZONES * SECS_PER_HOUR;
        }
    }
    Log.E("No NTP Response :-(");
    return 0; // return 0 if unable to get the time
}

int  getHour() {
    time_t gt,lt;
    struct tm *slt;

    gt = now();
    lt = tz.toLocal( gt );

    slt = localtime( &lt );
    return slt->tm_hour;
}

void TimeProvider::setNTPServer(String server , unsigned long syncinterval ) {
    NTPServer = server;
    NTPSync   = syncinterval;
}

void TimeProvider::setTimeZone( int tz ) {
    NTPTZ = tz;
}

void TimeProvider::setup() {
    if ( !initialized ) {

        // Initialize a local UDP port otherwise the NTP request won't work.
        Udp.begin(57334);

        NTP_TIMESERVER = NTPServer;
        NTP_TIMEZONES = NTPTZ;
        setSyncProvider( getNtpTime );
        setSyncInterval( NTPSync );
        initialized = true;
    }
}

String TimeProvider::getFullTime() {
    const char * Days [] ={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
    String datetime = String(Days[weekday()-1]) + ", " + day() + "/" + month() + "/" + year() + " " + getHour() + ":";

    int minutos = minute();
    if ( minutos < 10 )
        datetime = datetime + "0" + minutos + ":";
    else
        datetime = datetime + minutos + ":";
    
    int segundos = second();
    if ( segundos < 10 )
        datetime = datetime + "0" + segundos;
    else 
        datetime = datetime + segundos;

    return datetime;
}

void TimeProvider::logTime() {
     String datetime = getFullTime();
    Log.I( datetime );
}


String TimeProvider::getTime() {
    String datetime;

    if ( getHour() < 10 ) 
        datetime = "0" + String( getHour() ) + ":";
    else
        datetime = String( getHour() ) + ":";

    int minutos = minute();
    if ( minutos < 10 )
        datetime = datetime + "0" + minutos + ":";
    else
        datetime = datetime + minutos + ":";
    
    int segundos = second();
    if ( segundos < 10 )
        datetime = datetime + "0" + segundos;
    else 
        datetime = datetime + segundos;

    return datetime;
}

String TimeProvider::getDate() {
    String datetime;

    if ( day() < 10 ) 
        datetime = "0" + String( day() ) + "/";
    else
        datetime = String( day() ) + "/";

    if ( month() < 10 ) 
        datetime = datetime + "0" + month() + "/";
    else
        datetime = datetime + String( month() ) + "/";

    datetime = datetime + String(year());

    return datetime;
}

String TimeProvider::getHours() {
    String datetime;

    if ( getHour() < 10 ) 
        datetime = "0" + String( getHour() );
    else
        datetime = String( getHour() );

    return datetime;
}

String TimeProvider::getMinutes() {
    String datetime;

    if ( minute() < 10 ) 
        datetime = "0" + String( minute() );
    else
        datetime = String( minute() );

    return datetime;
}

String TimeProvider::getSeconds() {
    String datetime;

    if ( second() < 10 ) 
        datetime = "0" + String( second() );
    else
        datetime = String( second() );

    return datetime;
}


time_t TimeProvider::getLocalTime() {
    time_t local_time = tz.toLocal(now(), &tcr);

    return local_time;
}