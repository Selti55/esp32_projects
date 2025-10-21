
### 7. **arduino_sketches/outdoor_station/outdoor_station.ino**
```cpp
/**
 * Wetterstation Außenstation
 * ESP32 mit BME280 und Deep Sleep
 * Sendet Daten via ESP-NOW alle 15 Minuten
 */

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

// BME280 Sensor
Adafruit_BME280 bme;
#define SEALEVELPRESSURE_HPA (1013.25)

// ESP-NOW Peer Info
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Datenstruktur für ESP-NOW
typedef struct struct_message {
  float temperature;
  float humidity;
  float pressure;
  int battery;
} struct_message;

struct_message myData;

// GPIO Pins
#define I2C_SDA 21
#define I2C_SCL 22
#define LED_PIN 2

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  
  // I2C Initialisieren
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // BME280 initialisieren
  if (!bme.begin(0x76)) {
    Serial.println("BME280 nicht gefunden!");
    goToSleep();
  }
  
  // WiFi für ESP-NOW
  setupESP_NOW();
  
  // Messwerte lesen
  readSensors();
  
  // Daten senden
  sendData();
  
  // Deep Sleep
  goToSleep();
}

void loop() {
  // Wird nicht erreicht wegen Deep Sleep
}

void setupESP_NOW() {
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init fehlgeschlagen");
    return;
  }
  
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("ESP-NOW Peer hinzufügen fehlgeschlagen");
    return;
  }
}

void readSensors() {
  myData.temperature = bme.readTemperature();
  myData.humidity = bme.readHumidity();
  myData.pressure = bme.readPressure() / 100.0F;
  myData.battery = readBatteryLevel();
  
  Serial.print("Temperatur: "); Serial.println(myData.temperature);
  Serial.print("Luftfeuchtigkeit: "); Serial.println(myData.humidity);
  Serial.print("Luftdruck: "); Serial.println(myData.pressure);
  Serial.print("Batterie: "); Serial.println(myData.battery);
}

int readBatteryLevel() {
  // Batteriespannung lesen (über Spannungsteiler)
  int analogValue = analogRead(34);
  float voltage = analogValue * (3.3 / 4095.0) * 2; // Spannungsteiler 1:2
  return (int)(voltage * 100); // in Prozent umrechnen
}

void sendData() {
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  
  if (result == ESP_OK) {
    Serial.println("Daten gesendet");
  } else {
    Serial.println("Senden fehlgeschlagen");
  }
  
  delay(100); // Warten auf Sendevorgang
}

void goToSleep() {
  digitalWrite(LED_PIN, LOW);
  Serial.println("Gehe in Deep Sleep für 15 Minuten...");
  Serial.flush();
  
  esp_sleep_enable_timer_wakeup(15 * 60 * 1000000); // 15 Minuten
  esp_deep_sleep_start();
}