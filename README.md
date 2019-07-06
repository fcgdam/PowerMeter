# PowerMeter
PZEM-004T and ESP8266 based power meter: https://primalcortex.wordpress.com/2019/07/06/measuring-home-energy-consumption-with-the-pzem004t-and-esp8266/

This repository contains the firmware for an ESP8266 and PZEM-004T based power meter.

The PZEM004T-Test repository contains a simple program for testing the ESP8266 to PZEM004 communications and data retrieve.

The PowerMeter contains the full firmware for the ESP8266 that connects to a MQTT broker to send the data that is read, 
but also provides a status web page for viewing the current data.
