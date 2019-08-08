# PZEM004T ESP8266 Power Meter

Contains the firmware for the ESP8266 with the PZEM-004T based power meter as described in https://primalcortex.wordpress.com/2019/07/06/measuring-home-energy-consumption-with-the-pzem004t-and-esp8266/ and https://primalcortex.wordpress.com/2019/07/25/pzem-004t-esp8266-software/ 

This firmware provides the following functionality:

- Allows to connect to several access points and cycle between them in case of failure
- Sends data to a MQTT broker
- Uses software serial to connect and transfer data between the ESP8266 and the PZEM004T
- Provides an UDP based logging facility (much similar to syslog server, but not compatible) to view logging data without the need of a serial connection to the board
- Over the Air update support 
- Web page that shows the current state (Available at the standard port 80):

![](/images/web_page.png?raw=true)

- Integration with Node-Red where among other things we can see the current power factor, (more of interest of industrial applications than home applications)

![](/images/node_red.png?raw=true)

# Configuration

An initial configuration should be done on the *secrets.h* file, where the available access points are defined, namely the number: *NUMAPS*.
The Wifi configuration array then should be filled with the correct number, SSID and passwords for the available access points. 
The recomendation is that the first entry should be the main access point, either the one nearer or stronger.

The MQTT broker host and credential should also be set on this file.

Finaly the end point for the UDP log server should also be set.

On the log server, a Linux machine with the socat package installed, the *logserver.sh* script should be run to view the current log messages from the firmware.
Off course data is only available if the ESP8266 is connected to WIFI and is running.

# Onboard LED
The Wemos D1 onboard led blinks as follows:

- Very slow -> Connecting to WIFI
- Very fast -> Connecting to PZEM004T  -> We can see the connection state either at the web page or the log output
- Medium speed -> Working
- Lit -> Reading data from the PZEM004T.

Enjoy!
