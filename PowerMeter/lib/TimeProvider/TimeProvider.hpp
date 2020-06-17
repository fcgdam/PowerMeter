#ifndef __TIME_PROVIDER_H__
#define __TIME_PROVIDER_H__

#include <Arduino.h>
#include <TimeLib.h>

class TimeProvider
{
private:
    bool initialized      = false ;
    String  NTPServer     = "pt.pool.ntp.org";
    unsigned long NTPSync = 3600;   
    int           NTPTZ   = 0 ;                // TZ for Lisbon/PT

public:
    time_t getLocalTime();
    void setNTPServer( String, unsigned long );
    void setTimeZone( int );
    void logTime();
    void setup ();
    
    String getFullTime();
    String getDate();
    String getTime();

    String getHours();
    String getMinutes();
    String getSeconds();
    //void handle();
};

extern TimeProvider timeProvider;

#endif