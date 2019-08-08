# PZEM004T ESP8266 Power Meter

Contains the firmware for the ESP8266 with the PZEM-004T based power meter.

This firmware provides the following functionality:

- Allows to connect to several access points and cycle between them in case of failure
- Sends data to a MQTT broker
- Uses software serial to connect and transfer data between the ESP8266 and the PZEM004T
- Provides an UDP based logging facility (much similar to syslog server, but not compatible) to view logging data without the need of a serial connection to the board
- Over the Air update support 
- Web page that shows the current state:

![](/images/web_page.png?raw=true)

- Integration with Node-Red where among other things we can see the current power factor, (more of interest of industrial applications than home applications)

![](/images/node_red?raw=true)
