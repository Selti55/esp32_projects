/*
   TTGO T5 V2.3 + GDEM0213B74 (SSD1680) + DHT22 + Deep Sleep + Wake Button
   © 2025 – stromsparende Variante mit manuellem Aufwachen

   Funktionen:
   - misst Temperatur & Luftfeuchtigkeit
   - zeigt Werte groß auf dem 2.13" E-Paper an
   - geht 10 Minuten in Deep Sleep
   - kann zusätzlich durch Button (GPIO13) geweckt werden
*/
/*
🧩 Grundprinzip

Der ESP32 wacht aus Deep Sleep auf, wenn der konfigurierte GPIO39 (VN)
→ auf LOW gezogen wird (also gegen GND).

Das bedeutet:

Der Pin GPIO13 (VN) liegt standardmäßig HIGH (intern)
Ein Taster gegen GND löst den Wake-up aus

🟢 Kurz gesagt:
Ein Bein des Tasters an GPIO13 (VN)
Das andere Bein an GND
*/
/*
   TTGO T5 V2.3 (2.13" SSD1680) + DHT22 + Deep Sleep + Wake Button
   © 2025 – stromsparende Variante mit manuellem Aufwachen

   Funktionen:
   - misst Temperatur & Luftfeuchtigkeit
   - zeigt Werte groß auf dem 2.13" E-Paper an
   - geht 10 Minuten in Deep Sleep
   - kann zusätzlich durch Button (GPIO13 → GND) geweckt werden
*/

#include <WiFi.h>           
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <GxEPD2_BW.h>
#include "esp_sleep.h"

// ==================== Schriftarten ====================
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

// ==================== DHT22 Einstellungen ====================
#define DHTPIN 19
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ==================== Display (TTGO T5 V2.3 mit SSD1680) ====================
GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> display(GxEPD2_213_B74(/*CS=*/5, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));

// ==================== Deep Sleep ====================
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 600   // 10 Minuten

// ==================== Wake-Up Button ====================
#define WAKE_BUTTON_PIN GPIO_NUM_13   // Taster nach GND

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\nAufwachen aus Deep Sleep...");

  dht.begin();
  for (int i = 0; i < 5; i++) {
    delay(1000);
    float t = dht.readTemperature();
    if (!isnan(t)) break;
  }

  // Display vorbereiten
  display.init(115200);
  display.setRotation(1);
  display.setTextColor(GxEPD_BLACK);
  display.fillScreen(GxEPD_WHITE);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE); // wichtig: löscht alte Inhalte

    if (!isnan(h) && !isnan(t)) {
      Serial.printf("Temperatur: %.1f °C | Feuchte: %.1f %%\n", t, h);

      // Überschrift
      display.setFont(&FreeMono9pt7b);
      display.setCursor(5, 10);
      display.print("Messe alle 5 min");
      display.setCursor(5, 23);
      display.print("Sofort -> Taster");

      // Temperatur groß
      display.setFont(&FreeMonoBold24pt7b);
      display.setCursor(50, 60);
      display.print(t, 1);
      display.print(" °C");

      // Luftfeuchtigkeit groß
      display.setCursor(50, 95);
      display.print(h, 1);
      display.print(" %");

      // Statuszeile
      display.setFont(&FreeMono9pt7b);
      display.setCursor(130, 115);
      display.print("(c)Selti");
    } 
    else {
      Serial.println("Fehler beim Lesen des DHT22!");
      display.setFont(&FreeMonoBold12pt7b);
      display.setCursor(10, 100);
      display.print("DHT22 Fehler!");
    }
  } while (display.nextPage());

  // ==================== Wake-Up Konfiguration ====================
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);   // Zeitgesteuert
  esp_sleep_enable_ext0_wakeup(WAKE_BUTTON_PIN, 0);                // Button Wake-up (LOW = aktiv)

  Serial.printf("Gehe in Deep Sleep (%d Sekunden)...\n", TIME_TO_SLEEP);
  Serial.println("Drücke den Button (GPIO13→GND) zum Aufwachen.");

  delay(2000);
  esp_deep_sleep_start();
}

void loop() {
  // Wird im Deep Sleep nie erreicht
}
