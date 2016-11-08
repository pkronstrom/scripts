#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <SoftwareSerial.h>

#include <Ticker.h>

const char* ssid = "my-ssid";
const char* password = "my-pw";

#define CO2_TX D1
#define CO2_RX D2

SoftwareSerial SerialCO2(CO2_RX, CO2_TX); // RX, TX
bool co2ready = true;
Ticker co2ticker;

void tick() {
  co2ready = true;
}

const uint8_t cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
int getCO2() {
  
  uint8_t response[9];
  for (int i=0; i<9; i++) {
    SerialCO2.write(cmd[i]);
  }

  if (SerialCO2.available()) {
    for(int i=0; i < 9; i++) {
      response[i] = SerialCO2.read();
    }
  }
  
  int responseHigh = (int) response[2];
  int responseLow = (int) response[3];
  int ppm = (responseHigh << 8) + responseLow;

  co2ready = false;
  
  return ppm;
}

void setup() {
  
  Serial.begin(115200);
  SerialCO2.begin(9600);
  
  Serial.println("Connecting Wifi.");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  ArduinoOTA.setHostname("gofore-iot-1");
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.print("Initialized. IP: ");
  Serial.println(WiFi.localIP());

  co2ticker.attach(2, tick);
}

void loop() {
  ArduinoOTA.handle();
  
  if (co2ready) {
    Serial.println(getCO2());
  }
  
}