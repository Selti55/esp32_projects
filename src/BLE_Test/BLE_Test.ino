// =============================================================
// 🧭 ESP32 RemoteXY - NUR KOMPASS TEST
// =============================================================

#define REMOTEXY_MODE__ESP32CORE_BLE
#define REMOTEXY_BLUETOOTH_NAME "RemoteXY_V2"
#define REMOTEXY_ACCESS_PASSWORD "87654321"

#include <BLEDevice.h>
#include <RemoteXY.h>

#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = { 
  255,19,0,16,0,95,0,19,0,0,0,66,76,69,95,84,101,115,116,0,
  31,1,106,200,1,1,9,0,8,8,7,40,40,0,24,11,11,66,33,33,
  6,36,4,11,107,10,64,0,2,26,4,37,106,10,65,0,2,26,4,63,
  105,10,66,0,2,26,67,57,20,40,10,78,2,26,2,67,56,64,40,10,
  78,2,26,2,67,56,75,40,10,78,2,26,2,67,56,87,40,10,78,2,
  26,2 
};

struct {
  float compass_01;
  uint8_t connect_flag;
} RemoteXY;
#pragma pack(pop)

unsigned long lastUpdate = 0;
float angle = 0.0;

void setup() {
  Serial.begin(115200);
  RemoteXY_Init();
  Serial.println("Nur Kompass Test gestartet");
}

void loop() {
  RemoteXY_Handler();
  
  if (millis() - lastUpdate > 500) {
    lastUpdate = millis();
    
    angle += 30.0;
    if (angle >= 360.0) angle = 0.0;
    
    RemoteXY.compass_01 = angle;
    
    Serial.print("Kompass: ");
    Serial.println(angle);
  }
  delay(100);
}