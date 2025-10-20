// =============================================================
// 🐣 ESP32 Brutkasten mit RemoteXY BLE-Steuerung und 2-Punkt-Regler
// =============================================================
//
// 🔧 Funktionen:
// - BLE-Verbindung mit RemoteXY-App
// - Einstellbarer Sollwert über Schieberegler (15–25°C)
// - Ein-/Ausschalten über Hauptschalter
// - Einfache 2-Punkt-Regelung (Heizung EIN/AUS)
// - LED und Relais zeigen Heizungszustand
// - Serielle Diagnose mit Live-Werten
//
// ⚙️ Hardware:
// - ESP32 Dev Board
// - Relaismodul an GPIO 5
// - LED (Heizungsanzeige) an GPIO 2
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
#include <BLEDevice.h>       // Für Bluetooth Low Energy
#include <RemoteXY.h>        // RemoteXY-Schnittstelle

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
  int8_t soll_slider;     // Eingabe: Slider für Solltemperatur
  uint8_t main_switch;    // Eingabe: Hauptschalter (Heizung an/aus)
  float soll_value;       // Anzeige: aktuelle Solltemperatur
  float ist_value;        // Anzeige: aktuelle Isttemperatur
  uint8_t heater_led;     // Anzeige: Heizung aktiv (LED)
  float feuchte_value;    // (Reserviert für spätere Feuchtemessung)
  uint8_t connect_flag;   // BLE-Verbindungsstatus
} RemoteXY;
#pragma pack(pop)

// ------------------------
// Hardware-Pins
// ------------------------
const int RELAY_PIN = 5;   // GPIO 5 → Relais für Heizung
const int HEATER_LED = 2;  // GPIO 2 → LED (zeigt Heizstatus)

// ------------------------
// 2-Punkt-Regler Variablen
// ------------------------
double Input = 20.0;        // Aktuelle Temperatur (Istwert)
double Setpoint = 20.0;     // Solltemperatur (Zielwert)
const double HYSTERESIS = 0.5; // Hysterese für Ein/Aus-Regelung

// ------------------------
// Serielle Diagnose
// ------------------------
unsigned long lastSerial = 0;
const unsigned long serialInterval = 2000; // alle 2 Sekunden Ausgabe

// =============================================================
// SETUP
// =============================================================
void setup() {
  Serial.begin(115200);
  RemoteXY_Init(); // BLE-Verbindung zur RemoteXY-App aufbauen

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(HEATER_LED, OUTPUT);

  // Heizung initial aus
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(HEATER_LED, LOW);

  Serial.println("✅ System gestartet: RemoteXY + 2-Punkt-Regler aktiv");
}

// =============================================================
// LOOP
// =============================================================
void loop() {
  // 1️⃣ Daten von RemoteXY-App lesen
  RemoteXY_Handler();

  // 2️⃣ Solltemperatur aus Slider berechnen (15°C – 25°C)
  RemoteXY.soll_value = 15.0 + (RemoteXY.soll_slider / 100.0) * 10.0;
  Setpoint = RemoteXY.soll_value;

  // 3️⃣ Hauptschalter prüfen (App-Schalter)
  if (RemoteXY.main_switch) {

    // 4️⃣ 2-Punkt-Regelung mit Hysterese
    if (Input < Setpoint - HYSTERESIS) {
      // Temperatur zu niedrig → Heizung einschalten
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(HEATER_LED, HIGH);
      RemoteXY.heater_led = 1;
    } else if (Input > Setpoint + HYSTERESIS) {
      // Temperatur zu hoch → Heizung ausschalten
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(HEATER_LED, LOW);
      RemoteXY.heater_led = 0;
    }

    // 5️⃣ Temperatur-Simulation
    if (RemoteXY.heater_led) {
      // Heizung läuft → Erwärmung
      Input += 0.2;  // Temperaturzunahme pro Sekunde
    } else {
      // Heizung aus → Abkühlung
      if (Input > 20.0) Input -= 0.05; // sanfte Abkühlung
    }

  } else {
    // Hauptschalter AUS → Heizung aus, Abkühlung simulieren
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(HEATER_LED, LOW);
    RemoteXY.heater_led = 0;

    if (Input > 20.0) Input -= 0.05; // Umgebungseffekt
  }

  // 6️⃣ Aktuelle Isttemperatur an App senden
  RemoteXY.ist_value = Input;

  // 7️⃣ Serielle Ausgabe zur Diagnose
  if (millis() - lastSerial > serialInterval) {
    lastSerial = millis();
    Serial.print("MainSwitch: "); Serial.print(RemoteXY.main_switch);
    Serial.print(" | Soll: "); Serial.print(Setpoint, 2);
    Serial.print("°C | Ist: "); Serial.print(Input, 2);
    Serial.print("°C | Heizung: "); Serial.println(RemoteXY.heater_led);
  }

  // 8️⃣ Loop-Verzögerung (Regler arbeitet alle 1s)
  delay(1000);
}

