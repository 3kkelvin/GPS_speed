#include <HardwareSerial.h>
#include <WiFi.h>
#define DEBUGSerial Serial
const char* ssid = "mmslab_smallRoom";
const char* password = "mmslab406";
const char* host = "140.124.73.173";
const uint16_t port = 12345;
HardwareSerial GPSSerial(1);
WiFiClient client;

void setup() {
  GPSSerial.begin(115200, SERIAL_8N1, 33, 25);
  DEBUGSerial.begin(115200);
  DEBUGSerial.println("Wating...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    DEBUGSerial.println("Connecting to WiFi...");
  }
  DEBUGSerial.println("Connected to WiFi");
  

  if (!client.connect(host, port)) {
    DEBUGSerial.println("Connection to server failed");
  } else {
    DEBUGSerial.println("Connected to server");
  }
}

void loop() {
  while (GPSSerial.available()) {
    char c = GPSSerial.read();
    DEBUGSerial.write(c);
    if (client.connected()) {
      client.write(c);
    } else {
      DEBUGSerial.println("Disconnected from server");
      if (client.connect(host, port)) {
        DEBUGSerial.println("Reconnected to server");
      }
    }
  }
}