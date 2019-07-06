#include <PZEM004T.h>
 
//PZEM004T pzem( 5, 4 );  // RX,TX
PZEM004T pzem( D5, D6 ) ;  // Wemos D1 pins to be used for PZEM004T UART communications.

// The PZEM004T needs an IP to be set.
// We're not using it since we read data from the serial port.
IPAddress ip(192,168,1,1);

bool isPzemReady = false;
 
void setup() {
  Serial.begin( 115200 );

  while ( !isPzemReady ) {
      Serial.println("Connecting to PZEM...");
      isPzemReady = pzem.setAddress(ip);
      delay(1000);
   }
   
   Serial.println("Connected to PZEM...");
}
 
void loop() {

    delay(5000);
    
    Serial.println(F("-----------------------------------"));
    
    float v = pzem.voltage(ip);
    if (v < 0.0) 
        v = 0.0;
  
    Serial.print("V: ");
    Serial.print( v );
    Serial.println("V");
 
    float i = pzem.current(ip);
    if ( i >= 0.0 ) { 
        Serial.print("I: ");
        Serial.print( i );
        Serial.println("A ");
    }
  
    float p = pzem.power(ip);
    if (p >= 0.0 ) {
        Serial.print("PWR: ");
        Serial.print( p );
        Serial.print("W ");
    }
  
    float e = pzem.energy(ip);
    if( e >= 0.0 ) {
        Serial.print("Energy: ");
        Serial.print( e );
        Serial.println("Wh; ");        
    }
}
