#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>
#include <Ticker.h>
#include "DHT.h"

#define DHTPIN D4
#define BUTTONPIN D3
#define DHTTYPE DHT22
#define MEASUREMENT_INTERVAL 5000

#define SERVER_ADDRESS "192.168.1.49"
#define SERVER_PORT 8266

DHT dht(DHTPIN, DHTTYPE);
float humidity = 0;
float temperature = 0;
float heatindex = 0;
bool doMeasurementFlag = false;
Ticker dhtTicker;

int buttonState = 0;
Ticker debounceTicker;
bool debounce = false;

const char* ssid = "***";
const char* password = "***";

int sendData(String url, String payload) {
  HTTPClient http;
  http.begin(SERVER_ADDRESS, SERVER_PORT, url);
  //http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Content-Type", "application/json");
  //http.POST("title=foo&body=bar&userId=1");
  http.POST(payload);
  //http.writeToStream(&Serial);
  http.end();
}

void measureDHT() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  else {
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %  ");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" *C ");

    String payload = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
    sendData("/dht", payload);
  }

  doMeasurementFlag = false;
  
}

void doMeasure() {
  doMeasurementFlag = true;
}

void doDebounce() {
  debounce = false;
}

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);
  
  ArduinoOTA.setHostname("bembu.iot.1");

  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
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

  dht.begin();
  
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  dhtTicker.attach(3.0, doMeasure);
}

void loop() {
  ArduinoOTA.handle();

  if (doMeasurementFlag) measureDHT();

  buttonState = digitalRead(BUTTONPIN);
  if (buttonState == LOW && !debounce) {
    Serial.println("Button pres'd");
    String payload = "{\"button\":1}";
    sendData("/button", payload);
    debounce = true;
    debounceTicker.attach_ms(500, doDebounce);
  }

}

