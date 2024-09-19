#include <HardwareSerial.h>
#include <WiFi.h>
#include <TM1637Display.h>

#define DEBUGSerial Serial
#define CLK 26
#define DIO 27

const char* ssid = "mmslab_smallRoom";
const char* password = "mmslab406";
const char* host = "140.124.73.173";
const uint16_t port = 12345;
HardwareSerial GPSSerial(1);
WiFiClient client;
TM1637Display display(CLK, DIO);

String latitude = "";
String longitude = "";
String speed = "";
String gpsTime = "";
String gpsDate = "";
int satellites = 0;
unsigned long lastSendTime = 0;
const unsigned long interval = 1000; // 1秒

void setup() {
  GPSSerial.begin(115200, SERIAL_8N1, 33, 25);
  DEBUGSerial.begin(115200);
  DEBUGSerial.println("Wating...");
  display.setBrightness(0x0f); // 顯示亮度
  //連線WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    DEBUGSerial.println("Connecting to WiFi...");
  }
  DEBUGSerial.println("Connected to WiFi");
  //連線TCP
  if (!client.connect(host, port)) {
    DEBUGSerial.println("Connection to server failed");
  } else {
    DEBUGSerial.println("Connected to server");
  }
}

void loop() {
  while (GPSSerial.available()) {
    String line = GPSSerial.readStringUntil('\n'); // 一次讀一行
    if (line.startsWith("$GNGGA")) { // 多衛星組合的時間、位置等
      parseGGA(line);
    } else if (line.startsWith("$GNRMC")) { // 多衛星組合的導航資訊(速度)
      parseRMC(line);
    }
  }

  unsigned long currentMillis = millis();//控制至少超過一秒再發
  if (currentMillis - lastSendTime >= interval) {
    lastSendTime = currentMillis;

    if (client.connected()) {
      String data = "Latitude: " + latitude + ", Longitude: " + longitude + ", Speed: " + speed + " km/h, Date: " + gpsDate + " Time: " + gpsTime + ", Satellites: " + String(satellites);
      client.println(data);
    } else {
      DEBUGSerial.println("Disconnected from server");
      if (client.connect(host, port)) {
        DEBUGSerial.println("Reconnected to server");
      }
    }
    displaySpeed();
  }
}

void parseGGA(String line) {//讀取其中的緯度 經度 衛星數
  int commaIndex = line.indexOf(',');
  commaIndex = line.indexOf(',', commaIndex + 1);
  latitude = line.substring(commaIndex + 1, line.indexOf(',', commaIndex + 1));
  for (int i = 0; i < 2; i++) {
    commaIndex = line.indexOf(',', commaIndex + 1);
  }
  longitude = line.substring(commaIndex + 1, line.indexOf(',', commaIndex + 1));
  commaIndex = line.indexOf(',', commaIndex + 1);
  for (int i = 0; i < 4; i++) {
    commaIndex = line.indexOf(',', commaIndex + 1);
  }
  satellites = line.substring(commaIndex + 1, line.indexOf(',', commaIndex + 1)).toInt();
}

void parseRMC(String line) {//讀取其中的UTC時間 速度
  int commaIndex = line.indexOf(',');
  gpsTime = line.substring(commaIndex + 1, line.indexOf(',', commaIndex + 1));
  commaIndex = line.indexOf(',', commaIndex + 1);
  for (int i = 0; i < 5; i++) {
    commaIndex = line.indexOf(',', commaIndex + 1);
  }
  speed = String(line.substring(commaIndex + 1, line.indexOf(',', commaIndex + 1)).toFloat() * 1.852); // 節轉時速
  for (int i = 0; i < 2; i++) {
    commaIndex = line.indexOf(',', commaIndex + 1);
  }
  gpsDate = line.substring(commaIndex + 1, line.indexOf(',', commaIndex + 1));
}

void displaySpeed() {
  float speedValue = speed.toFloat();
  int intPart = (int)speedValue;
  int decimalPart = (int)((speedValue - intPart) * 10); // 只取小數點後一位

  uint8_t data[] = {0, 0, 0, 0};

  if (intPart >= 100) {
    data[0] = display.encodeDigit((intPart / 100) % 10);
    data[1] = display.encodeDigit((intPart / 10) % 10);
    data[2] = display.encodeDigit(intPart % 10) | 0x80; // 加上小數點
    data[3] = display.encodeDigit(decimalPart);
  } else if (intPart >= 10) {
    data[1] = display.encodeDigit((intPart / 10) % 10);
    data[2] = display.encodeDigit(intPart % 10) | 0x80; // 加上小數點
    data[3] = display.encodeDigit(decimalPart);
  } else {
    data[2] = display.encodeDigit(intPart % 10) | 0x80; // 加上小數點
    data[3] = display.encodeDigit(decimalPart);
  }

  display.setSegments(data);
}