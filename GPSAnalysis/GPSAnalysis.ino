#include <HardwareSerial.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <TM1637Display.h>

#define DEBUGSerial Serial
#define CLK 26
#define DIO 27

const char* ssid = "Redmi K50";
const char* password = "04870587ts87";
const char* host = "3kkelvin-api.italkutalk.com";
const uint16_t port = 443;
const char* test_root_ca =  //https需要的SSL，需要手動從網頁抓
  "-----BEGIN CERTIFICATE-----\n"
  "MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD\n"
  "VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEUMBIG\n"
  "A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw\n"
  "WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz\n"
  "IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi\n"
  "AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi\n"
  "QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR\n"
  "HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW\n"
  "BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D\n"
  "9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8\n"
  "p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD\n"
  "-----END CERTIFICATE-----\n";
WiFiClientSecure client;
HardwareSerial GPSSerial(1);

TM1637Display display(CLK, DIO);

String latitude = "";
String longitude = "";
String speed = "";
String gpsTime = "";
String gpsDate = "";
int satellites = 0;
unsigned long lastSendTime = 0;
const unsigned long interval = 1000;  // 1秒

void setup() {
  GPSSerial.begin(115200, SERIAL_8N1, 33, 25);
  DEBUGSerial.begin(115200);
  DEBUGSerial.println("Wating...");
  display.setBrightness(0x0f);  // 顯示亮度
  //連線WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    DEBUGSerial.println("Connecting to WiFi...");
  }
  DEBUGSerial.println("Connected to WiFi");
  //連線TCP
  client.setCACert(test_root_ca);  //設定HTTPS 用CA
  while (!client.connect(host, port)) {
    Serial.println("Connection failed! Retrying...");
    delay(1000);
  }
  Serial.println("Connected to server!");
  client.println("GET / HTTP/1.0");
  client.println("Host: 3kkelvin-api.italkutalk.com");
  client.println("Connection: close");
  client.println();
}

void loop() {
  while (GPSSerial.available()) {
    String line = GPSSerial.readStringUntil('\n');  // 一次讀一行
    if (line.startsWith("$GNGGA")) {                // 多衛星組合的時間、位置等
      parseGGA(line);
    } else if (line.startsWith("$GNRMC")) {  // 多衛星組合的導航資訊(速度)
      parseRMC(line);
    }
  }

  unsigned long currentMillis = millis();  //控制至少超過一秒再發
  if (currentMillis - lastSendTime >= interval) {
    lastSendTime = currentMillis;

    String jsonData = packToJson();
    postToAPIServer(jsonData);
    if (client.connected()) {
      String data = "Latitude=" + latitude + "&Longitude=" + longitude + "&Speed=" + speed + "&Date=" + gpsDate + "&Time=" + gpsTime + "&Satellites=" + String(satellites);
      client.println("POST /gps HTTP/1.1");
      client.println("Host: " + String(host));
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.println("Content-Length: " + String(data.length()));
      client.println();
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

void parseGGA(String line) {  //讀取其中的緯度 經度 衛星數
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

void parseRMC(String line) {  //讀取其中的UTC時間 速度
  int commaIndex = line.indexOf(',');
  gpsTime = line.substring(commaIndex + 1, line.indexOf(',', commaIndex + 1));
  commaIndex = line.indexOf(',', commaIndex + 1);
  for (int i = 0; i < 5; i++) {
    commaIndex = line.indexOf(',', commaIndex + 1);
  }
  speed = String(line.substring(commaIndex + 1, line.indexOf(',', commaIndex + 1)).toFloat() * 1.852);  // 節轉時速
  for (int i = 0; i < 2; i++) {
    commaIndex = line.indexOf(',', commaIndex + 1);
  }
  gpsDate = line.substring(commaIndex + 1, line.indexOf(',', commaIndex + 1));
}

void displaySpeed() {
  float speedValue = speed.toFloat();
  int intPart = (int)speedValue;
  int decimalPart = (int)((speedValue - intPart) * 10);  // 只取小數點後一位

  uint8_t data[] = { 0, 0, 0, 0 };

  if (intPart >= 100) {
    data[0] = display.encodeDigit((intPart / 100) % 10);
    data[1] = display.encodeDigit((intPart / 10) % 10);
    data[2] = display.encodeDigit(intPart % 10) | 0x80;  // 加上小數點
    data[3] = display.encodeDigit(decimalPart);
  } else if (intPart >= 10) {
    data[1] = display.encodeDigit((intPart / 10) % 10);
    data[2] = display.encodeDigit(intPart % 10) | 0x80;  // 加上小數點
    data[3] = display.encodeDigit(decimalPart);
  } else {
    data[2] = display.encodeDigit(intPart % 10) | 0x80;  // 加上小數點
    data[3] = display.encodeDigit(decimalPart);
  }

  display.setSegments(data);
}

String packToJson() {  //包成JSON好給APIserver
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["latitude"] = latitude;
  jsonDoc["longitude"] = longitude;
  jsonDoc["speed"] = speed;
  jsonDoc["date"] = gpsDate;
  jsonDoc["time"] = gpsTime;
  jsonDoc["satellites"] = satellites;

  String jsonString;
  serializeJson(jsonDoc, jsonString);
  return jsonString;
}

void postToAPIServer(String jsonData) {
  if (!client.connected()) {
    if (!client.connect(host, port)) {
      Serial.println("Connection to API server failed!");
      return;
    }
  }
  Serial.println("上傳");

  client.print("POST /gps/ HTTP/1.1\r\n");
  client.print("Host: ");
  client.println(host);
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonData.length());
  client.println();
  client.println(jsonData);

  while (client.available()) {
    client.read();
  }
}
