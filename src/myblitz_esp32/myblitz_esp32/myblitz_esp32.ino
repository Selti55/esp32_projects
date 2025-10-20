
#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>
#include <RemoteXY.h>
#include "DHT.h"

#define REMOTEXY_BLUETOOTH_NAME "MyBlitz"
#define REMOTEXY_ACCESS_PASSWORD "12345678"

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = { 255,2,0,13,0,10,0,19,0,0,0,66,76,69,95,77,121,66,108,105,116,122,0 };
struct {
  float temp;
  float hum;
  uint8_t connect_flag;
} RemoteXY;
#pragma pack(pop)

void setup() {
  Serial.begin(115200);
  RemoteXY_Init();
  dht.begin();
  Serial.println("✅ MyBlitz gestartet");
}

void loop() {
  RemoteXY_Handler();
  RemoteXY.temp = dht.readTemperature();
  RemoteXY.hum = dht.readHumidity();

  Serial.print("🌡️ Temp: ");
  Serial.print(RemoteXY.temp);
  Serial.print("°C | 💧 Feuchte: ");
  Serial.print(RemoteXY.hum);
  Serial.println("%");

  delay(2000);
}
