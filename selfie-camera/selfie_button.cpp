/*
  A script for Wemos D1 Mini.
  
  By pressing a button, the code sends an HTTP
  request to the given address.

  server_ip:port/photo
  
  Install Arduino core for ESP8266 from
  https://github.com/esp8266/Arduino

  @author Peter Kronström
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>

#define WIFI_SSID "ssid"                // WiFi  SSID
#define WIFI_PASSWORD "pass"            // WiFi password
#define SERVER_ADDRESS "192.168.1.xxx"  // use 'ifconfig' in Raspberry Pi server
#define SERVER_PORT 5000                // server.py port

#define DEVICE_ID "button"
#define PAYLOAD "{\"success\":\"True\"}"

#define BUTTONPIN D3

int buttonState = 0;
Ticker debounceTicker;
bool debounce = false;

int sendData(String url, String payload) {
  HTTPClient http;
  http.begin(SERVER_ADDRESS, SERVER_PORT, url);
  http.addHeader("Content-Type", "application/json");
  http.POST(payload);
  http.end();
}

void doDebounce() {
  debounce = false;
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.print("Initialized.");
  Serial.print(". IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  buttonState = digitalRead(BUTTONPIN);

  if (buttonState == LOW && !debounce) {
    sendData("/photo", PAYLOAD);
    debounce = true;
    debounceTicker.attach_ms(500, doDebounce);
  }

}