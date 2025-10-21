# 🔧 Arduino IDE Einrichtung
https://espressif.github.io/arduino-esp32/package_esp32_index.json
## 1. Arduino IDE Installation

### Download
- [Arduino IDE 2.x](https://www.arduino.cc/en/software) herunterladen
- Installieren und starten

### ESP32 Board Support
1. **Datei → Voreinstellungen**
   - Zusätzliche Board-Verwaltungs-URLs:
https://espressif.github.io/arduino-esp32/package_esp32_index.json




2. **Werkzeuge → Board → Boardverwalter**
- "esp32" suchen
- "ESP32 by Espressif Systems" installieren

## 2. Benötigte Bibliotheken

### Über Bibliotheksverwalter installieren:
- **Adafruit BME280 Library**
- **Adafruit Unified Sensor** 
- **Adafruit GFX Library**
- **Adafruit ILI9341**
- **TFT_eSPI** (für Displays)

### Manuelle Installation:
1. **Sketch → Bibliothek einbinden → .ZIP-Bibliothek hinzufügen**
2. Herunterladene ZIP-Dateien auswählen

## 3. Board Einstellungen

### Für beide Stationen:
- **Board:** ESP32 Dev Module
- **Upload Speed:** 921600
- **Flash Frequency:** 80MHz
- **Partition Scheme:** Default 4MB with spiffs...

### Spezifisch:
- **CPU Frequency:** 240MHz (WiFi)
- **Core Debug Level:** Info

## 4. Sketch Upload

1. **Station auswählen:**
- `arduino_sketches/outdoor_station/outdoor_station.ino`
- `arduino_sketches/indoor_station/indoor_station.ino`

2. **ESP32 verbinden** via USB

3. **Werkzeuge → Port** auswählen

4. **Hochladen** klicken