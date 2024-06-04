#define HELTEC_WIRELESS_STICK
#include <heltec_unofficial.h>
#include <ArduinoJson.h>
#include <OneWire.h> //https://github.com/PaulStoffregen/OneWire/issues/132#issuecomment-1934368810
#include <DallasTemperature.h>
#include <Preferences.h>
#include <HX711.h>

#define BOARD_ID               1 // Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define TIME_TO_SLEEP        600 // Time ESP32 will go to sleep (in seconds)
#define ONE_WIRE_BUS          20 // GPIO where the DS18B20 is connected to

#define BUTTON_TARE_PIN        1 // GPIO where is connected the button for tare scale number 1 on boot
#define SCALEWAIT            500 // Waiting time for scale power up
#define HX711_scale0_DOUT_PIN 48 // HX711 scale0 circuit wiring DOUT
#define HX711_scale0_SCK_PIN  47 // HX711 scale0 circuit wiring SCK
#define HX711_scale1_DOUT_PIN  5 // HX711 scale1 circuit wiring DOUT
#define HX711_scale1_SCK_PIN   6 // HX711 scale1 circuit wiring SCK
#define HX711_scale2_DOUT_PIN  7 // HX711 scale2 circuit wiring DOUT
#define HX711_scale2_SCK_PIN  26 // HX711 scale2 circuit wiring SCK
#define HX711_scale3_DOUT_PIN 46 // HX711 scale3 circuit wiring DOUT
#define HX711_scale3_SCK_PIN  45 // HX711 scale3 circuit wiring SCK
#define HX711_scale4_DOUT_PIN 48 // HX711 scale4 circuit wiring DOUT
#define HX711_scale4_SCK_PIN  47 // HX711 scale4 circuit wiring SCK
#define HX711_scale5_DOUT_PIN  5 // HX711 scale5 circuit wiring DOUT
#define HX711_scale5_SCK_PIN   6 // HX711 scale5 circuit wiring SCK
#define HX711_scale6_DOUT_PIN  7 // HX711 scale6 circuit wiring DOUT
#define HX711_scale6_SCK_PIN  26 // HX711 scale6 circuit wiring SCK
#define HX711_scale7_DOUT_PIN 46 // HX711 scale7 circuit wiring DOUT
#define HX711_scale7_SCK_PIN  45 // HX711 scale7 circuit wiring SCK

#define FREQUENCY            866.3 // for Europe. Frequency in MHz. Keep the decimal point to designate float.
#define BANDWIDTH            125.0 // Allowed values are 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0 and 500.0 kHz.
#define SPREADING_FACTOR      10 // Number from 5 to 12. Higher means slower but higher "processor gain", meaning (in nutshell) longer range and more robust against interference.
#define TRANSMIT_POWER         0 // Transmit power in dBm. 0 dBm = 1 mW. This value set between -9 dBm (0.125 mW) to 22 dBm (158 mW). 

OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature sensor
HX711 scale[8];
Preferences preferences;

const int knownWeight = 650; //grams
RTC_DATA_ATTR int bootCount = 0;

float readTemperature(int index) {
  float tempC = sensors.getTempCByIndex(index);
  if(tempC == DEVICE_DISCONNECTED_C) { // Check if reading was successful
    Serial.print("Error: Could not read temperature data for index ["); Serial.print(index); Serial.println("]");
  }
  DeviceAddress dAddress;
  if (sensors.getAddress(dAddress, index)) {
    Serial.print("Index: [");
    Serial.print(index);
    Serial.print("] => Address: [");
    printAddress(dAddress);
    Serial.println("]");
  }
  return tempC;
}

float readTemperature(DeviceAddress deviceAddress) {
  float tempC = sensors.getTempC(deviceAddress);
  if(tempC == DEVICE_DISCONNECTED_C) { // Check if reading was successful
    Serial.print("Error: Could not read temperature data for address ["); printAddress(deviceAddress); Serial.println("]");
  }
  return tempC;
}

void printAddress(DeviceAddress deviceAddress) { // Function to print a device address
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}

float getCalFactor(int num) {
  char buf[11];
  const char* lblCalFactor = "calFactor";
  strcpy(buf, lblCalFactor);
  strcat(buf, String(num).c_str());

  preferences.begin("hiveMon", true); // Namespace is opened in read-only (RO) mode
  float calFactor = preferences.getFloat(buf, 0.0);
  preferences.end();

  if (calFactor == 0) { calFactor = -1; } // For divide by zero
  //Serial.print(buf); Serial.print(" => "); Serial.println(calFactor);
  return calFactor;
}

float getScaleOffset(int num) {
  char buf[13];
  const char* lblScaleOffset = "scaleOffset";
  strcpy(buf, lblScaleOffset);
  strcat(buf, String(num).c_str());

  preferences.begin("hiveMon", true); // Namespace is opened in read-only (RO) mode
  float scaleOffset = preferences.getFloat(buf, 0.0);
  preferences.end();

  //Serial.print(buf); Serial.print(" => "); Serial.println(scaleOffset);
  return scaleOffset;
}

void setCalFactor(int num, float calFactor) {
  char buf[11];
  const char* lblCalFactor = "calFactor";
  strcpy(buf, lblCalFactor);
  strcat(buf, String(num).c_str());
  Serial.print(buf); Serial.print(" => "); Serial.println(calFactor);

  preferences.begin("hiveMon", false); // Namespace is opened in read-write (RW) mode
  preferences.putFloat(buf, calFactor);
  preferences.end();
}

void setScaleOffset(int num, float scaleOffset) {
  char buf[13];
  const char* lblScaleOffset = "scaleOffset";
  strcpy(buf, lblScaleOffset);
  strcat(buf, String(num).c_str());
  Serial.print(buf); Serial.print(" => "); Serial.println(scaleOffset);

  preferences.begin("hiveMon", false); // Namespace is opened in read-write (RW) mode
  preferences.putFloat(buf, scaleOffset);
  preferences.end();
}

float readWeight(int num) {
  scale[num].power_up();
  float weight;
  float calFactor = getCalFactor(num);
  float scaleOffset = getScaleOffset(num);
  heltec_delay(SCALEWAIT);
  if (scale[num].is_ready()) { // https://forum.arduino.cc/t/drifting-load-cell-in-a-stable-environment/1048252/123
    weight = (scale[num].get_units(10) - scaleOffset) / calFactor;
  } else {
    Serial.print("HX711 number ["); Serial.print(num); Serial.println("] not found");
  }
  scale[num].power_down();
  return weight;
}

void calibratingScale(int num) { //https://randomnerdtutorials.com/esp32-load-cell-hx711/
  Serial.println("Starting calibrate scale...");
  scale[num].power_up();
  heltec_delay(SCALEWAIT);
  if (scale[num].is_ready()) {
    Serial.print("Tare: remove any weights from the scale number ["); Serial.print(num); Serial.print("]... ");
    display.print("TARE ["); display.print(num); display.println("]");
    display.print("remove...");
    heltec_delay(5000);
    float scaleOffset = scale[num].read_average(20);
    scale[num].tare();
    Serial.println("done");
    display.println("done");
    Serial.print("Place a known weight of " + (String)knownWeight + " grams on the scale number ["); Serial.print(num); Serial.print("]... ");
    display.print("place " + (String)knownWeight + "g...");
    heltec_delay(5000);
    float reading = scale[num].get_units(10); //scale.get_value(10);
    Serial.println("done");
    display.println("done");
    float calFactor = reading / knownWeight;
    scale[num].power_down();
    setCalFactor(num, calFactor);
    setScaleOffset(num, scaleOffset);
  } else {
    Serial.print("HX711 number ["); Serial.print(num); Serial.println("] not found");
  }
}

void initLoRa() {
  radio.begin();
  radio.setFrequency(FREQUENCY);
  radio.setBandwidth(BANDWIDTH);
  radio.setSpreadingFactor(SPREADING_FACTOR);
  radio.setOutputPower(TRANSMIT_POWER);
}

void sendLoRa(String message) {
  Serial.printf("TX [%s] ", message.c_str());
  //heltec_led(50); // 50% brightness is plenty for this LED
  uint64_t tx_time = millis();
  radio.transmit(message.c_str());
  tx_time = millis() - tx_time;
  //heltec_led(0);
  if (_radiolib_status == RADIOLIB_ERR_NONE) {
    Serial.printf("=> OK (%i ms)\n", (int)tx_time);
  } else {
    Serial.printf("=> fail (%i)\n", _radiolib_status);
  }
}

void setup() {
  heltec_setup();
  heltec_delay(500); // For the deep_sleep bug
  Serial.begin(115200);
  display.println(" BEEHIVE");
  display.println(" BALANCE");
  heltec_ve(true); // Turn on Ve (external power) +3.3v
/*
  rtc_cpu_freq_config_t config; // Reduce CPU freq for problem with HX711
  rtc_clk_cpu_freq_get_config(&config);
  rtc_clk_cpu_freq_to_config(RTC_CPU_FREQ_80M, &config);
  rtc_clk_cpu_freq_set_config_fast(&config);
*/
  scale[0].begin(HX711_scale0_DOUT_PIN, HX711_scale0_SCK_PIN); // Initialize HX711
  scale[1].begin(HX711_scale1_DOUT_PIN, HX711_scale1_SCK_PIN); // Initialize HX711
  scale[2].begin(HX711_scale2_DOUT_PIN, HX711_scale2_SCK_PIN); // Initialize HX711
  scale[3].begin(HX711_scale3_DOUT_PIN, HX711_scale3_SCK_PIN); // Initialize HX711
  scale[4].begin(HX711_scale4_DOUT_PIN, HX711_scale4_SCK_PIN); // Initialize HX711
  scale[5].begin(HX711_scale5_DOUT_PIN, HX711_scale5_SCK_PIN); // Initialize HX711
  scale[6].begin(HX711_scale6_DOUT_PIN, HX711_scale6_SCK_PIN); // Initialize HX711
  scale[7].begin(HX711_scale7_DOUT_PIN, HX711_scale7_SCK_PIN); // Initialize HX711
  sensors.begin(); // Start the DS18B20 sensor
  sensors.requestTemperatures(); // Request to read DS18B20 sensor
  pinMode(BUTTON_TARE_PIN, INPUT_PULLUP);

  if(digitalRead(BUTTON_TARE_PIN) == LOW) {
    calibratingScale(0);
  }

  if (++bootCount > 1000) {
    ESP.restart();
  }

  int battery = heltec_battery_percent();
  heltec_temperature(); // The first read is wrong
  float temperature = heltec_temperature();

  StaticJsonDocument<400> jsondoc;
  jsondoc["station"] = BOARD_ID;

  JsonObject stats = jsondoc.createNestedObject("stats");
  stats["read"] = bootCount;
  stats["batt"] = battery;
  stats["temp"] = temperature;

  JsonArray reads = jsondoc.createNestedArray("reads");

  StaticJsonDocument<50> jsonDocHive0;
  jsonDocHive0["hive"] = 0;
  jsonDocHive0["temp"] = readTemperature(0);
  jsonDocHive0["weight"] = readWeight(0);
  reads.add(jsonDocHive0);

  StaticJsonDocument<50> jsonDocHive1;
  jsonDocHive1["hive"] = 1;
  jsonDocHive1["temp"] = readTemperature(1);
  jsonDocHive1["weight"] = readWeight(1);
  reads.add(jsonDocHive1);

  StaticJsonDocument<50> jsonDocHive2;
  jsonDocHive2["hive"] = 2;
  jsonDocHive2["temp"] = readTemperature(2);
  jsonDocHive2["weight"] = readWeight(2);
  reads.add(jsonDocHive2);

  StaticJsonDocument<50> jsonDocHive3;
  jsonDocHive3["hive"] = 3;
  jsonDocHive3["temp"] = readTemperature(3);
  jsonDocHive3["weight"] = readWeight(3);
  reads.add(jsonDocHive3);
  
  String jsonString1;
  serializeJson(jsondoc, jsonString1);

  jsondoc.clear();
  jsondoc["station"] = BOARD_ID;

  stats = jsondoc.createNestedObject("stats");
  stats["read"] = bootCount;
  stats["batt"] = battery;
  stats["temp"] = temperature;

  reads = jsondoc.createNestedArray("reads");

  StaticJsonDocument<50> jsonDocHive4;
  jsonDocHive4["hive"] = 4;
  jsonDocHive4["temp"] = readTemperature(4);
  jsonDocHive4["weight"] = readWeight(4);
  reads.add(jsonDocHive4);

  StaticJsonDocument<50> jsonDocHive5;
  jsonDocHive5["hive"] = 5;
  jsonDocHive5["temp"] = readTemperature(5);
  jsonDocHive5["weight"] = readWeight(5);
  reads.add(jsonDocHive5);

  StaticJsonDocument<50> jsonDocHive6;
  jsonDocHive6["hive"] = 6;
  jsonDocHive6["temp"] = readTemperature(6);
  jsonDocHive6["weight"] = readWeight(6);
  reads.add(jsonDocHive6);

  StaticJsonDocument<50> jsonDocHive7;
  jsonDocHive7["hive"] = 7;
  jsonDocHive7["temp"] = readTemperature(7);
  jsonDocHive7["weight"] = readWeight(7);
  reads.add(jsonDocHive7);

  String jsonString2;
  serializeJson(jsondoc, jsonString2);

  initLoRa();
  sendLoRa(jsonString1);
  sendLoRa(jsonString2);

  heltec_delay(100);
  heltec_ve(false); // Turn off Ve (external power) +3.3v
  heltec_deep_sleep(TIME_TO_SLEEP);
}
 
void loop() {
  heltec_loop();
}