#define HELTEC_WIRELESS_STICK // must be before "#include <heltec_unofficial.h>"
#include <heltec_unofficial.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#define FREQUENCY          866.3 // for Europe. Frequency in MHz. Keep the decimal point to designate float.
#define BANDWIDTH          125.0 // Allowed values are 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0 and 500.0 kHz.
#define SPREADING_FACTOR      10 // Number from 5 to 12. Higher means slower but higher "processor gain", meaning (in nutshell) longer range and more robust against interference.
#define TRANSMIT_POWER         0 // Transmit power in dBm. 0 dBm = 1 mW. This value set between -9 dBm (0.125 mW) to 22 dBm (158 mW). 

String rxdata;
float rssi = 0;
volatile bool rxFlag = false;
const char* ssid = "<INSERT_SSID>";
const char* password = "<INSERT_PASSWORD>";
const char* serverName = "http://<INSERT_SERVERNAME>.org/insert.php";

void prepareRequest() {
  String jsonString;
  StaticJsonDocument<400> doc;
  DeserializationError error = deserializeJson(doc, rxdata);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  display.print("STATION: "); display.println(String(doc["station"]));
  display.print("RSSI: "); display.println(rssi);
  doc["stats"]["rssi"] = rssi;
  serializeJson(doc, jsonString);
  Serial.print("PACKET: "); Serial.println(jsonString);
  makeRequest(jsonString);
}

void makeRequest(String bodyReq) {
  HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(bodyReq);
    if (httpResponseCode == HTTP_CODE_OK) {
      Serial.print("REQUEST: ");
      Serial.println(bodyReq);
      String payload = http.getString();
      Serial.print("RESPONSE: ");
      Serial.println(payload);
    } else {
      Serial.print("HTTP Response code: ");
      Serial.print(httpResponseCode);
      Serial.print(" => ");
      Serial.println(http.errorToString(httpResponseCode));
    }
    http.end();
}

void initWifi() {
  WiFi.mode(WIFI_STA); // Set the device as a Station
  WiFi.begin(ssid, password);

  Serial.print("Connecting to SSID: ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("connected");
  display.println("WIFI... OK");
  display.print("RSSI: "); display.println(WiFi.RSSI());
  Serial.print("ESP Board MAC Address: ");
  Serial.print(WiFi.macAddress());
  Serial.print(" | Station IP Address: ");
  Serial.print(WiFi.localIP());
  Serial.print(" | Wi-Fi Channel: ");
  Serial.print(WiFi.channel());
  Serial.print(" | RSSI: ");
  Serial.println(WiFi.RSSI());
}

void initLoRa() {
  radio.begin();
  radio.setDio1Action(rx); // Set the callback function for received packets
  radio.setFrequency(FREQUENCY);
  radio.setBandwidth(BANDWIDTH);
  radio.setSpreadingFactor(SPREADING_FACTOR);
  radio.setOutputPower(TRANSMIT_POWER);
  radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF); // Start receiving
}

void rx() { // Can't do Serial or display things here, takes too much time for the interrupt
  rxFlag = true;
}

void setup() {
  heltec_setup();
  initWifi();
  initLoRa();
}

void loop() {
  heltec_loop();

  if (rxFlag) {
    rxFlag = false;
    radio.readData(rxdata);
    if (_radiolib_status == RADIOLIB_ERR_NONE) {
      rssi = radio.getRSSI();
      prepareRequest();
    }
  }
}