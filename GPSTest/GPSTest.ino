#include <HardwareSerial.h>
#define DEBUGSerial Serial

HardwareSerial GPSSerial(1);

void setup()	
{
  GPSSerial.begin(115200, SERIAL_8N1, 33, 25);	
  DEBUGSerial.begin(115200);  
  DEBUGSerial.println("Wating...");
}

void loop()		
{
  while (GPSSerial.available()) {   
     DEBUGSerial.write(GPSSerial.read());
  }
}

