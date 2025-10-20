// ------------------------
// RemoteXY + BLE + DeepSleep + Wake Button
// ------------------------
/*
⚙️ Hardware-Verdrahtung
Komponente	Anschluss
LED	an GPIO 2 (onboard oder extern)
Taster	GPIO 33 → GND
Stromversorgung	3.3 V (Achtung bei Batterieprojekten: keine 5 V!)

💡 Hinweis:
Der Wake-Pin muss RTC-fähig sein. Geeignet sind z. B. GPIO 33, 32, 25, 26, 27.
Nicht jeder Pin unterstützt Deep-Sleep-Wake!
*/

/*
🧠 Ziel:
ESP32 startet, stellt BLE-Verbindung zu RemoteXY her
Du steuerst eine LED über die App
Nach 15 Sekunden geht der ESP32 in Deep-Sleep
Durch Taster an GPIO 32 → GND wird der ESP wieder aufgeweckt
*/


// Hardware-Pins
const int LED_PIN = 5;        // Onboard-LED
const int WAKE_PIN = 32;      // Wake-up-Taster (nach GND)

// BLE-Verbindung definieren (VOR dem include!)
#define REMOTEXY_MODE__ESP32CORE_BLE
#define REMOTEXY_BLUETOOTH_NAME "ESP32_BLE_RemoteXY"
#define REMOTEXY_ACCESS_PASSWORD "12345678"

#include <BLEDevice.h>
#include "esp_sleep.h"
#include <RemoteXY.h>

// RemoteXY GUI-Layout (ein Schalter + eine LED)
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = {
  255,1,0,1,0,42,0,19,0,0,0,66,76,69,0,31,1,106,200,1,
  1,2,0,2,25,20,44,22,0,2,26,31,31,79,78,0,79,70,70,0,
  70,35,66,29,29,16,26,37,0
};

struct {
  uint8_t switch_01;     // Eingang: Schalter in der App
  uint8_t led_01;        // Ausgang: LED-Anzeige in der App
  uint8_t connect_flag;  // =1, wenn verbunden
} RemoteXY;
#pragma pack(pop)

// ------------------------
// SETUP
// ------------------------
void setup() {
  Serial.begin(115200);

  // Prüfen, ob Aufwachen aus DeepSleep
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("Aufgewacht durch GPIO 32!");
  }

  // Pin vorbereiten
  pinMode(LED_PIN, OUTPUT);
  pinMode(WAKE_PIN, INPUT_PULLUP); // Taster gegen GND

  // RemoteXY initialisieren
  RemoteXY_Init();
  Serial.println("ESP32 mit RemoteXY BLE gestartet");
}

// ------------------------
// LOOP
// ------------------------
void loop() {
  RemoteXY_Handler();

  // App-Schalter steuert LED
  if (RemoteXY.switch_01) {
    digitalWrite(LED_PIN, HIGH);
    RemoteXY.led_01 = 1;
  } else {
    digitalWrite(LED_PIN, LOW);
    RemoteXY.led_01 = 0;
  }

  static unsigned long start = millis();
  if (millis() - start > 15000) {
    Serial.println("BLE-Sitzung beendet, gehe in Deep-Sleep...");

    // Wakeup-Pin konfigurieren
    esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKE_PIN, 0);

    // Deep-Sleep starten
    esp_deep_sleep_start();
  }
}
