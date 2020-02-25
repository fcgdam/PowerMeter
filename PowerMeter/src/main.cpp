#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <MQTT.h>               // For MQTT support
#include <ArduinoOTA.h>         // For OTA firmware update support
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <PZEM004T.h>
#include <SimpleTimer.h>
#include <WebServer.hpp>
#include <LogClient.hpp>
#include <TimeProvider.hpp>
#include <AppData.hpp>
#include "secrets.h"

#define  FW_Version "1.0.1"

// PZEM004T Configuration.
// Change the pins if using something other than the Wemos D1 mini and D5/D6 for UART communication.
#define         PZEM_TIMEOUT  3500
PZEM004T        pzem( D6, D5);  // RX,TX
IPAddress       ip( 192,168,1,1 );

unsigned long   pzemDataOK  = 0;
unsigned long   pzemDataNOK = 0;

int             mState = 0;                       // Current state for the state machine controlling the connection to the PZEM
                                                  // 0 - Disconnected
                                                  // 1 - Connecting
                                                  // 2 - Retrieve data
                                                  // 3 - Retrieving data

unsigned long   SLEEP_TIME    = 60 * 1000;        // Sleep time between reads of PZEM004T values 
int             MONITOR_LED   = 2;                // Monitoring led to send some visual indication to the user.
//unsigned long   tick;                             // Used for blinking the MONITOR_LED: ON -> OTA , Blink FAST: Connecting, Blink SLOW: Working
unsigned long   ledBlink = 200;                   // Led blink interval: Fast - Connecting to PZEM, slow - Connected
int             monitor_led_state = LOW;  

WiFiClient      WIFIClient;
IPAddress       thisDevice;
String          Wifi_ssid;
char            hostname[32];

SimpleTimer     timer;

MQTTClient      MQTT_client(512);
char            MQTT_AttributesTopic[256];
char            MQTT_TelemetryTopic[256];
char            SensorAttributes[512];
char            SensorTelemetry[512];

unsigned long   previousMillis = 0;
unsigned long   pingMqtt = 5 * 60 * 1000;  // Ping the MQTT broker every 5 minutes by sending the IOT Atributes message.
unsigned long   previousPing = 0;

/*
 * Hostname:
 * 
 * Sets the device hostname for OTA and MDNS.
 * 
 * */
void setHostname() {
 
  // Set Hostname for OTA and network mDNS (add only 2 last bytes of last MAC Address)
  sprintf_P( hostname, PSTR("ESP-PWRMETER-%04X"), ESP.getChipId() & 0xFFFF);
}

/*
 * MQTT Support
 * 
 * Static atributes like, IP, SSID, and so on are set to the MQTT atributes topic.
 * Telemetry data, data that changes through time, are sent to the MQTT telemetry topic.
 * 
 */

//* Supporting functions:
void calcAttributesTopic() {
    String s = "iot/device/" + String(MQTT_ClientID) + "/attributes";
    s.toCharArray(MQTT_AttributesTopic,256,0);
}

void calcTelemetryTopic() {
    String s = "iot/device/" + String(MQTT_ClientID) + "/telemetry";
    s.toCharArray(MQTT_TelemetryTopic,256,0);
}

/*
 * IOT Support:
 * 
 * Functions that using MQTT send data to the IOT server by publishing data on specific topics.
 * 
 */
void IOT_setAttributes() {
    String s = "[{\"type\":\"ESP8266\"}," \
                 "{\"ipaddr\":\"" + thisDevice.toString() + "\"}," \
                 "{\"ssid\":\""+ WiFi.SSID() + "\"}," \
                 "{\"rssi\":\""+ String(WiFi.RSSI()) + "\"}," \
                 "{\"web\":\"http://" + thisDevice.toString() + "\"}," \
                 "{\"dataok\":" + String(pzemDataOK) + "}," \
                 "{\"datanok\":" + String(pzemDataNOK) + "}" \
                 "]";
    
    s.toCharArray( SensorAttributes, 512,0);
    Log.I("PowerMeter Attributes:");
    Log.I(SensorAttributes);
    MQTT_client.publish( MQTT_AttributesTopic, SensorAttributes);
}

void IOT_setTelemetry(String SensorTelemetry) {
    //s.toCharArray(SensorAttributes, 512,0);
    MQTT_client.publish( MQTT_TelemetryTopic, SensorTelemetry);
}

// MQTT Calback function for receiving subscribed messages.
void MQTT_callback(String &topic, String &payload) {
    /* Just a standard callback. */
    Log.I("Message arrived in topic: ");
    Log.I(topic);

    Log.I("Message:");
    Log.I( payload );
}

//* Connects to the MQTT Broker
void MQTT_Connect() {
    Log.I("Connecting to MQTT Broker...");
    MQTT_client.begin( MQTT_Server, MQTT_Port , WIFIClient );
    MQTT_client.onMessage( MQTT_callback );
    MQTT_client.setOptions( 120, true, 120 );

    while (! MQTT_client.connect( MQTT_ClientID, MQTT_UserID, MQTT_Password ) ) {
        Log.E("MQTT Connection failed.");
        delay(1000);
    }

    calcAttributesTopic();
    calcTelemetryTopic();

    Log.I("Connected to MQTT!");
}

/*
 * OTA support
 * 
 */
void OTA_Setup() {

    Log.I("Setting up OTA...");
    ArduinoOTA.setHostname( hostname );
    ArduinoOTA.begin();

    // OTA callbacks
    ArduinoOTA.onStart([]() {
      Log.I(F("\r\nOTA Starting"));
      digitalWrite( MONITOR_LED, HIGH);
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      uint8_t percent = progress/( total/100 );
      
      if (percent % 10 == 0) {
        Log.I( String(percent));
        digitalWrite( MONITOR_LED, !digitalRead( MONITOR_LED));    
      }
    });

    ArduinoOTA.onEnd([]() {

      Log.I(F("OTA Done\nRebooting..."));
      digitalWrite( MONITOR_LED, LOW);
    });

    ArduinoOTA.onError([](ota_error_t error) {
      Log.E("OTA Error: " + String(error));

      if (error == OTA_AUTH_ERROR) {
        Log.E("OTA Auth Failed");
      } else
      if (error == OTA_BEGIN_ERROR) {
        Log.E("OTA Begin Failed");
      } else
      if (error == OTA_CONNECT_ERROR) {
        Log.E("OTA Connect Failed");
      } else
      if (error == OTA_RECEIVE_ERROR) {
        Log.E("OTA Receive Failed");
      } else
      if (error == OTA_END_ERROR) {
        Log.E("OTA End Failed");
      }

      ESP.restart();
    });

    Log.I("OTA setup done!");
}

void display_WIFIInfo() {
    Log.I("Connected to WIFI: " + WiFi.SSID() );

    thisDevice = WiFi.localIP();
    Log.I(" IP: " + thisDevice.toString() );
}

/*
 * WIFI_Setup: Setup the WIFI connection.
 * 
 * It cycles over the configured access points until a sucessufull connection is done
 * 
 */
void WIFI_Setup() {
    bool   connected = false;
    char   *ssid;
    char   *pwd;
    int    cntAP = 0;
    int    tries = 0;
    String out;

    Log.I("Connecting to WIFI...");
    
    // Station mode.
    WiFi.disconnect();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);

    while ( !connected ) {
        ssid = (char *)APs[cntAP][0];
        pwd = (char *)APs[cntAP][1];
        Log.I("Connecting to: ");
        Log.I(ssid);

        WiFi.begin(ssid, pwd );

        if (WiFi.waitForConnectResult() != WL_CONNECTED) {
                Log.E("Connection Failed! Trying next AP...");
                Log.E("Number of tries: " + String(tries));
                
                cntAP++;
                tries++;
                // Circle the array one entry after another
                if (cntAP == NUMAPS )
                cntAP = 0;
                
                // Set Monitor led on:  Light up the led to show that something is working
                digitalWrite( MONITOR_LED, LOW); 
                delay(1000);
                digitalWrite( MONITOR_LED, HIGH); 

                yield();
        } else
                connected = true;
    }

/*    WiFi.macAddress(MAC_address);
    for (unsigned int i = 0; i < sizeof(MAC_address); ++i){
      sprintf(MAC_char,"%s%02x:",MAC_char,MAC_address[i]);
    }
*/  
    display_WIFIInfo();
}

/*
 * check_Connectivity:  Checks the connectivity. 
 * 
 * Checks the connectivity namely if we are connected to WIFI and to the MQTT broker.
 * If not, we try to reconnect.
 * 
 */

void check_Connectivity() {

        /* Check WIFI connection first. */        
        if ( WiFi.status() != WL_CONNECTED ) {
            WIFI_Setup();
            MQTT_Connect();
        } else {
            /* Check MQTT connectivity: */
            if ( !MQTT_client.connected() ) {
                MQTT_Connect();
                // Send the IOT device attributes at MQTT connection
                IOT_setAttributes();
            }
        }
}

/*
 * back_tasks:  Executes the background tasks
 * 
 * Calls the functions necessary to keep everything running smoothly while waiting or looping
 * Such tasks include mantaining the MQTT connection, checking OTA status and updating the timers.
 * 
 */

void back_tasks() {
    MQTT_client.loop();           // Handle MQTT
    timer.run();                  // Handle SimpleTimer
}

/*
 * PWRMeter_Connect:
 * 
 * Tries to connect to the PowerMeter
 * 
 */

void PWRMeter_Connect() {
    //uint8_t tries = 0;
    bool    pzemOK = false;
    
    Log.I("Connecting to PZEM004T...");
    appData.setPZEMState(PZEM_CONNECTING);
    pzem.setReadTimeout( PZEM_TIMEOUT);

    //while ( ((pzemOK=pzem.setAddress(ip)) == false) && ( tries < 10 ) ) {
    if ( (pzemOK=pzem.setAddress(ip)) == false)  {
        Log.E("Failed to connect to PZEM004T...");
        appData.setPZEMState(PZEM_CONNECTFAIL);
        //tries++;
        mState = 0;       // Return to the NOT Connected State.
    }

    if (pzemOK) {
        Log.I("Connection to PZEM004T OK!");
        appData.setPZEMState(PZEM_CONNECTED);
        mState = 2;        // Move forward to the Connected State.
    } else {
      appData.setPZEMState(PZEM_DISCONNECTED);
      mState = 0;          // Return to the NOT Connected State.
    }
}

/*
 * PWRMeter_getData:
 * 
 * Gets data from the Power meter and sends it to the backend through MQTT
 * 
 */

void PWRMeter_getData() {
    float v = 0;
    uint8_t tries = 0;

    // Set Monitor led on:
    digitalWrite( MONITOR_LED, LOW); 

    Log.I("Getting PowerMeter data...");

    // Get the PZEM004T Power Meter data
    do {
      v = pzem.voltage(ip);
      tries++;
      
      // Execute the back ground tasks otherwise we may loose conectivity to the MQTT broker.
      back_tasks();
      delay(250);
      
    } while ( (v == -1) && ( tries < 10) );

    if ( tries == 10 ) Log.E("Failed to get PowerMeter data after 10 tries!");
    if ( v == -1 ) Log.E("No valid data obtained from the PowerMeter: V=-1"); 
    else Log.I("PowerMeter Data OK!");

    float i = pzem.current(ip);
    float p = pzem.power(ip);
    float e = pzem.energy(ip);

    // Turn led off:
    digitalWrite( MONITOR_LED, HIGH); 

    // Build the MQTT message:
    String s = "{\"V\":" + String(v) + \
               ",\"I\":" + String(i) + \
               ",\"P\":" + String(p) + \
               ",\"E\":" + String(e) + "}";

    Log.I("-> Power Meter data: ");
    Log.I( s );

    // Send the data through MQTT to the backend
    if ( v >= 0 ) {
        IOT_setTelemetry(s);

        // Set AppData for display
        appData.setVoltage( v );
        appData.setCurrent( i );
        appData.setPower( p ); 
        appData.setEnergy( e );
        appData.setSamplesOK();

        pzemDataOK++;
    } else {
        Log.W("Data not sent due to invalid read.");
        appData.setSamplesNOK();
        pzemDataNOK++;
    }

    mState = 2;   // Move back to the Read data state to trigger another (future) read.
}

// Moves the state machine to the next state.
void PWRMeter_ReadState() {
    mState = 2;
}

// Just blink the onboard led according to the defined period
void Blink_MonitorLed() {
    digitalWrite( MONITOR_LED , monitor_led_state );
    if ( monitor_led_state == LOW ) 
        monitor_led_state = HIGH;
    else 
        monitor_led_state = LOW;

    timer.setTimeout( ledBlink , Blink_MonitorLed ); // With this trick we can change the blink rate of the led
}

// Used to send the system atributes to the MQTT topic regarding the device attributes.
void IOT_SendAttributes() {
    IOT_setAttributes();
}

// Prints time.
void printTime() {
    timeProvider.logTime();
}

/*
 * MAIN CODE
 * 
 */

void setup() {
    IPAddress udpServerAddress;
    udpServerAddress.fromString(UDPLOG_Server);

    appData.setFWVersion(FW_Version);
    appData.setLogServerIPInfo(udpServerAddress.toString());

    Serial.begin(115200);
    delay (200);                           // Wait for the serial port to settle.
    setHostname();
    Log.setSerial( true );                 // Log to Serial
    Log.setServer( udpServerAddress, UDPLOG_Port );
    Log.setTagName("PWM01");                // Define a tag for log lines output

    // Indicator onbord LED
    pinMode( MONITOR_LED, OUTPUT);
    digitalWrite( MONITOR_LED, LOW);

    // We set WIFI first...
    WIFI_Setup();

    // Setup Logging system.
    Log.I("Enabling UDP Log Server...");
    Log.setUdp( true );                  // Log to UDP server when connected to WIFI.

    // Print a boot mark
    Log.W("------------------------------------------------> Power Meter REBOOT");

    // Setup OTA
    OTA_Setup();

    // Set time provider to know current date and time
    timeProvider.setup();
    timeProvider.logTime();

    //Connect to the MQTT Broker:
    MQTT_Connect();

    // Send the IOT device attributes at MQTT connection
    IOT_setAttributes();

    // Setup WebServer so that we can have a web page while connecting to the PZEM004T
    Log.I("Setting up the embedded web server...");
    webServer.setup();
    Log.I("Web server available at port 80.");

    delay(100);
    display_WIFIInfo();                   // To display wifi info on the UDP socket.

    // Setup the monitor blinking led
    timer.setTimeout( ledBlink , Blink_MonitorLed ); 

    // Periodically send to the MQTT server the IOT device state
    timer.setInterval( pingMqtt , IOT_SendAttributes );

    // Periodically log the time
    timer.setInterval( 3 * 60 * 1000 , printTime );

    // Setup MDNS
    MDNS.begin( hostname );
    MDNS.addService("http", "tcp", 80);
}

void loop() {
    // Check if we are still connected.
    check_Connectivity();

    // Power meter state machine
    switch (mState)
    {
        case 0:   // We are'nt connected. Trigger a connection every 3s until we connect.
                timer.setTimeout( 3000 , PWRMeter_Connect );
                ledBlink = 200;  // Blink the LED Fast
                mState = 1;
        break;
        case 1:   // Connecting. The PWRMeter_Connect function will move to next state
                  // So we do nothing here.
        break;
        case 2:   // We are connected. Trigger a Power meter read.
                PWRMeter_getData();
                timer.setTimeout(SLEEP_TIME, PWRMeter_ReadState);
                ledBlink = 500;  // Blink the LED Slow
                mState = 3;
        break;
        case 3:   // We are waiting for reading the data. The PWRMeter_ReadState will move back to the previous state.
        break;
    
        default:
        break;
    }
    
    // Execute the background tasks.
    back_tasks();

    ArduinoOTA.handle();          // Handle OTA.
}
