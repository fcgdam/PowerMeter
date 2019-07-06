#ifndef _SECRETS_H
#define _SECRETS_H

#define NUMAPS 4
static char const *APs[NUMAPS][2] = {
  {"AAAAAA","asdfljkasdfqweui34u"},
  {"BBBBB","açsdlkfj29338fnree2"},
  {"CCCCCCC","dfewererr"},
  {"ZZZZZZZ","herer3ra"}
};

// For connecting to the MQTT Broker
#define MQTT_Server    "192.168.1.17"
#define MQTT_Port      1883
#define MQTT_ClientID  "ESP8266_PowerMeter"
#define MQTT_UserID    "ESP8266_PowerMeter"
#define MQTT_Password  "password1"

// For the UDP Log Server
#define UDPLOG_Server  "192.168.1.68"     // My local PC, but should be a server"
#define UDPLOG_Port    5014

#endif
