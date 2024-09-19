#include <WiFi.h>

// WiFi 設定
const char* ssid = "mmslab_smallRoom";
const char* password = "mmslab406";

// TCP 伺服器設定
const char* serverIP = "140.124.73.173";
const uint16_t serverPort = 12345;

WiFiClient client;

void setup() {
  // 初始化序列埠
  Serial.begin(115200);

  // 連接到 WiFi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
  // 連接到 TCP 伺服器
  Serial.print("Connecting to server ");
  Serial.print(serverIP);
  Serial.print(":");
  Serial.println(serverPort);



  if (client.connect(serverIP, serverPort)) {
    Serial.println("Connected to server");
  } else {
    Serial.println("Connection to server failed");
  }
}

void loop() {
  // 檢查是否有可讀取的資料
  if (client.available()) {
    String line = client.readStringUntil('\n');
    Serial.println("Received from server: " + line);
  }

  // 檢查是否斷線
  if (!client.connected()) {
    Serial.println("Disconnected from server");
    while (true);  // 停止運行
  }
}
