/*
So funktioniert’s
Phase	        Beschreibung
Start	        ESP32 wacht aus Deep-Sleep auf
Messung	      DHT22 liefert Temperatur & Luftfeuchtigkeit
Anzeige	      Werte werden auf dem E-Paper aktualisiert
Schlafmodus	  Gerät schläft 5 Minuten und misst dann erneut
*/
/*
   TTGO T5 V2.3 + GDEM0213B74 (SSD1680) + DHT22 + Deep Sleep
   © 2025 – stromsparende Variante ohne RemoteXY
*/

#include <WiFi.h>           // notwendig für ESP32-Initialisierung (auch wenn nicht genutzt)
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <GxEPD2_BW.h>
#include "esp_sleep.h"

// Schriftarten
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBold18pt7b.h>
#include <Fonts/FreeSerifBold24pt7b.h>
/*
Mono →  gleichbreit, technisch, gut für Werte (z. B. „23.4 °C“)
Sans →  modern, klar, gute Lesbarkeit
Serif → elegant, aber auf kleinen Displays oft unruhig
Bold →  für Überschriften oder größere Texte
Die Zahl (z. B. 12pt, 18pt, 24pt) entspricht der ungefähren Schriftgröße.
*/

// ==================== DHT22 Einstellungen ====================
#define DHTPIN 19
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ==================== Display (TTGO T5 V2.3 mit SSD1680) ====================
GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> display(GxEPD2_213_B74(/*CS=*/5, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));

// ==================== Deep Sleep Einstellungen ====================
#define uS_TO_S_FACTOR 1000000ULL   // Mikrosekunden → Sekunden
#define TIME_TO_SLEEP 300           // Schlafdauer in Sekunden (5 Minuten)

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\nAufwachen aus Deep Sleep...");

  dht.begin();

  // E-Paper Display initialisieren
  display.init(115200);
  display.setRotation(1);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold18pt7b);

  // Messung
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Ausgabe
  if (!isnan(h) && !isnan(t)) {
    Serial.printf("Temperatur: %.1f °C | Feuchte: %.1f %%\n", t, h);

    // Anzeige aktualisieren
    display.firstPage();
    do {
      display.setCursor(10, 30);
      display.print("Temeratur:");
      display.setCursor(60, 60);
      display.print(t, 1);
      display.print(" C");

      display.setCursor(10, 85);
      display.print("Feuchte:");
      display.setCursor(56, 115);
      display.print(h, 1);
      display.print(" %");
    } while (display.nextPage());
  } 
  else {
    Serial.println("Fehler beim Lesen des DHT22!");
    display.firstPage();
    do {
      display.setCursor(10, 30);
      display.print("DHT22 Fehler!");
    } while (display.nextPage());
  }

  // Deep Sleep starten
  Serial.printf("Gehe in Deep Sleep (%d Sekunden)...\n", TIME_TO_SLEEP);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

// ==================== Loop ====================
// Wird im Deep Sleep nie erreicht
void loop() {}
