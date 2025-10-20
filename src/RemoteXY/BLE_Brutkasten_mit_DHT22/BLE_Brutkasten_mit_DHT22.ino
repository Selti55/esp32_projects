// =============================================================
// 🐣 ESP32 Brutkasten mit RemoteXY BLE-Steuerung und 2-Punkt-Regler
// =============================================================
//
// 🔧 Funktionen:
// - BLE-Verbindung mit RemoteXY-App
// - Einstellbarer Sollwert über Schieberegler (15–25°C)
// - Ein-/Ausschalten über Hauptschalter
// - 2-Punkt-Regelung basierend auf DHT22-Temperatur
// - LED und Relais zeigen Heizungszustand
// - Serielle Diagnose mit Live-Werten
//
// ⚙️ Hardware:
// - ESP32 Dev Board
// - Relaismodul an GPIO 5
// - LED (Heizungsanzeige) an GPIO 2
// - DHT22 an GPIO 4 
//   (Vcc=DHT_Pin1; Signal=DHT_Pin2; GND=DHT_Pin2; 10K=DHT_Pin-DHT_Pin2)
//
// =============================================================

// ------------------------
// RemoteXY Config (BLE)
// ------------------------
#define REMOTEXY_MODE__ESP32CORE_BLE
#define REMOTEXY_BLUETOOTH_NAME "RemoteXY"
#define REMOTEXY_ACCESS_PASSWORD "12345678"

// ------------------------
// Bibliotheken
// ------------------------
#include <BLEDevice.h>       // BLE-Unterstützung
#include <RemoteXY.h>        // RemoteXY-Schnittstelle
#include <DHT.h>             // DHT22 Sensor

// ------------------------
// RemoteXY Layout
// ------------------------
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = {
  255,2,0,13,0,207,0,19,0,0,0,66,76,69,95,69,83,80,51,50,
  66,114,117,116,107,97,115,116,101,110,0,31,1,106,200,1,1,16,0,
  130,6,17,96,53,11,19,129,12,4,82,12,64,8,66,114,117,116,107,97,
  115,116,101,110,32,66,76,69,0,129,39,19,22,12,64,6,115,111,108,108,
  58,0,4,9,32,89,19,128,2,26,67,25,57,40,10,78,2,26,2,130,
  9,76,91,39,11,21,129,46,78,16,12,64,6,105,115,116,58,0,67,26,
  91,40,10,78,2,26,2,130,12,119,85,76,19,17,70,64,128,18,18,16,
  8,12,0,129,25,132,34,12,64,6,104,101,97,116,101,114,0,2,21,157,
  68,22,0,2,26,31,31,79,78,0,79,70,70,0,129,70,57,13,12,64,
  8,194,176,67,0,129,69,90,13,12,64,8,194,176,67,0,67,26,103,40,
  10,78,2,26,2,129,72,102,9,12,64,24,37,0
};

// RemoteXY-Datenstruktur
struct {
  int8_t soll_slider;     // Slider für Solltemperatur
  uint8_t main_switch;    // Hauptschalter (Heizung an/aus)
  float soll_value;       // Anzeige: aktuelle Solltemperatur
  float ist_value;        // Anzeige: aktuelle Isttemperatur
  uint8_t heater_led;     // LED Heizung aktiv
  float feuchte_value;    // optional
  uint8_t connect_flag;   // BLE-Verbindung
} RemoteXY;
#pragma pack(pop)

// ------------------------
// Hardware-Pins
// ------------------------
const int RELAY_PIN = 5;    // Relais Heizung
const int HEATER_LED = 2;   // LED Heizung
const int DHT_PIN = 4;      // DHT22 Signal-Pin

// ------------------------
// 2-Punkt-Regler Variablen
// ------------------------
double Setpoint = 20.0;           // Solltemperatur
const double HYSTERESIS = 0.3;    // Hysterese

// ------------------------
// DHT22 Sensor initialisieren
// ------------------------
DHT dht(DHT_PIN, DHT22);

// ------------------------
// Serielle Diagnose
// ------------------------
unsigned long lastSerial = 0;
const unsigned long serialInterval = 1000; // alle 1 Sekunden Ausgabe

// =============================================================
// SETUP
// =============================================================
void setup() {
  Serial.begin(115200);
  RemoteXY_Init();           // BLE-Verbindung
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(HEATER_LED, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(HEATER_LED, LOW);

  dht.begin();

  Serial.println("✅ System gestartet: RemoteXY + 2-Punkt-Regler (DHT22)");
}

// =============================================================
// LOOP
// =============================================================
void loop() {
  // 1️⃣ RemoteXY-Daten lesen
  RemoteXY_Handler();

  // 2️⃣ Solltemperatur aus Slider berechnen (15°C – 25°C)
  RemoteXY.soll_value = 15.0 + (RemoteXY.soll_slider / 100.0) * 10.0;
  Setpoint = RemoteXY.soll_value;

  // 3️⃣ Aktuelle Temperatur vom DHT22 lesen
  float DHT_Ist_Wert = dht.readTemperature();   // °C
  float DHT_Humidity = dht.readHumidity();      // % rF

 if (isnan(DHT_Ist_Wert) || isnan(DHT_Humidity)) {
    Serial.println("⚠️ DHT22-Fehler: keine Temperatur gelesen");
    delay(2000);
    return;
  }

  // 4️⃣ Hauptschalter prüfen
  if (RemoteXY.main_switch) {

    // 5️⃣ 2-Punkt-Regelung mit Hysterese
    if (DHT_Ist_Wert < Setpoint - HYSTERESIS) {
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(HEATER_LED, HIGH);
      RemoteXY.heater_led = 1;
    } else if (DHT_Ist_Wert > Setpoint + HYSTERESIS) {
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(HEATER_LED, LOW);
      RemoteXY.heater_led = 0;
    }

  } else {
    // Heizung aus, wenn Hauptschalter aus
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(HEATER_LED, LOW);
    RemoteXY.heater_led = 0;
  }

  // 6️⃣ Istwert an App senden
  RemoteXY.ist_value = DHT_Ist_Wert;
  // Feuchtigkeit an RemoteXY übertragen
  RemoteXY.feuchte_value = DHT_Humidity;

  // 7️⃣ Serielle Diagnose
  if (millis() - lastSerial > serialInterval) {
    lastSerial = millis();
    Serial.print("MainSwitch: "); Serial.print(RemoteXY.main_switch);
    Serial.print(" | Soll: "); Serial.print(Setpoint, 2);
    Serial.print("°C | Ist: "); Serial.print(DHT_Ist_Wert, 2);
    Serial.print("°C | Heizung: "); Serial.println(RemoteXY.heater_led);
  }

  delay(1000);
}
