#include <RemoteXY.h>

// RemoteXY connection settings
#define REMOTEXY_MODE__ESP32CORE_BLE
#define REMOTEXY_BLUETOOTH_NAME "ESP32_BLE_RemoteXY"

// RemoteXY GUI (wie oben)
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = {
  255, 2, 0, 0, 0, 23, 0, 8, 13, 0,
  1, 0, 5, 5, 22, 11, 2, 31, 88, 0
};

struct {
  uint8_t switch1;
} RemoteXY;
#pragma pack(pop)

const int LED_PIN = 2;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  RemoteXY_Init();
  Serial.println("ESP32 mit RemoteXY BLE gestartet");
}

void loop() {
  RemoteXY_Handler();

  if (RemoteXY.switch1) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // Nach 15 Sekunden BLE-Session Deep-Sleep
  static unsigned long start = millis();
  if (millis() - start > 15000) {
    Serial.println("BLE-Sitzung beendet, gehe in Deep-Sleep...");
    btStop(); // Bluetooth ausschalten
    esp_sleep_enable_timer_wakeup(10ULL * 1000000ULL);
    esp_deep_sleep_start();
  }
}
