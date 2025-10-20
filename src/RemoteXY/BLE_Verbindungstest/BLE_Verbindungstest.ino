#define REMOTEXY_MODE__ESP32_BLE


// RemoteXY Verbindungsparameter
#define REMOTEXY_BLUETOOTH_NAME "ESP32_RemoteXY"

#include <RemoteXY.h>

// RemoteXY interface Beschreibung
#pragma pack(push, 1)
struct {
  int8_t slider;   // Beispiel-Steuerelement
} RemoteXY;
#pragma pack(pop)

// Verbindung initialisieren
void setup() {
  RemoteXY_Init(); // Start BLE
  Serial.begin(115200);
  Serial.println("RemoteXY BLE gestartet...");
}

void loop() {
  RemoteXY_Handler();  // BLE-Handling
  Serial.println("Warte auf App...");
  delay(1000);
}
