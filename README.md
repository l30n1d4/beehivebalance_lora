# beehivebalance_lora
![logic diagram](https://github.com/l30n1d4/beehivebalance_lora/blob/main/images/logic_diagram.png?raw=true)

### Library
- heltec_unofficial.h (https://github.com/ropg/heltec_esp32_lora_v3)
- ArduinoJson.h (https://arduinojson.org)
- OneWire.h (https://www.pjrc.com/teensy/td_libs_OneWire.html)
- DallasTemperature.h (https://github.com/milesburton/Arduino-Temperature-Control-Library)
- Preferences.h
- HX711.h (https://github.com/RobTillaart/HX711)
- WiFi.h
- HTTPClient.h

### Sender
- 1x Heltec Wireless Stick (https://heltec.org/project/wireless-stick-v3/)
- 8x HX711 (https://futuranet.it/prodotto/scheda-elettronica-per-cella-di-carico-hx711/)
- 8x DS18B20 (https://futuranet.it/prodotto/sonda-di-temperatura-waterproof-con-ds18b20/)

### Receiver
- 1x Heltec Wireless Stick (https://heltec.org/project/wireless-stick-v3/)

### Database
```sql
CREATE TABLE `beehive` (
  `station` tinyint unsigned NOT NULL,
  `hive` tinyint unsigned NOT NULL,
  `datetime` datetime NOT NULL,
  `temperature` decimal(5,2) DEFAULT NULL,
  `weight` decimal(5,2) DEFAULT NULL,
  PRIMARY KEY (`station`,`hive`,`datetime`)
)
```
```sql
CREATE TABLE `stats` (
  `station` tinyint unsigned NOT NULL,
  `datetime` datetime NOT NULL,
  `temperature` decimal(5,2) DEFAULT NULL,
  `battery` decimal(5,2) DEFAULT NULL,
  `reading` int unsigned DEFAULT NULL,
  `rssi` tinyint DEFAULT NULL,
  PRIMARY KEY (`datetime`,`station`)
)
```
### JSON
```json
{
    "reads": [
        {
            "hive": 0,
            "temp": 23.44,
            "weight": -7.36
        },
        ...
        {
            "hive": 7,
            "temp": -127.00,
            "weight": 0.00
        }
    ],
    "station": 1,
    "stats": {
        "batt": 96,
        "read": 1,
        "temp": 24.10
    }
}
```