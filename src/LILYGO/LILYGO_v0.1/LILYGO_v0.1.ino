/*
   TTGO T5 V2.3 + GDEM0213B74 (SSD1680) + DHT22 + RemoteXY WiFi AP
   © 2025 – App-kompatible Version (RemoteXY 4.1.4)
   Getestet mit GxEPD2 1.6.5 und DHT Sensor Library 1.4.6

      Info:
   https://lilygo.cc/products/t5-v2-3-1?srsltid=AfmBOops9zqapfBA03_gsPr8TM6WmtTbr4zUauHAhwKXpLUF-_hDIHp1
*/

//////////////////////////////////////////////
// RemoteXY include
//////////////////////////////////////////////
#define REMOTEXY_MODE__ESP32CORE_WIFI_POINT
#include <WiFi.h>


//////////////////////////////////////////////
// RemoteXY connection settings
//////////////////////////////////////////////
#define REMOTEXY_WIFI_SSID "TTGO_DHT22"
#define REMOTEXY_WIFI_PASSWORD "12345678"
#define REMOTEXY_SERVER_PORT 6377

//////////////////////////////////////////////
// RemoteXY GUI configuration
//////////////////////////////////////////////

#include <RemoteXY.h>

#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 48 bytes
  { 255,0,0,8,0,41,0,19,0,0,0,84,84,71,79,95,68,72,84,50,
  50,0,31,1,106,200,1,1,2,0,67,13,28,78,18,78,2,26,2,67,
  14,58,78,19,78,2,26,2 };
#pragma pack(pop)

//////////////////////////////////////////////
// Struktur der RemoteXY Variablen
//////////////////////////////////////////////
struct {
  float temperature;
  float humidity;
  uint8_t connect_flag;
} RemoteXY;


//////////////////////////////////////////////
// Bibliotheken für Sensor und Display
//////////////////////////////////////////////
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold12pt7b.h>

//////////////////////////////////////////////
// DHT22 Setup
//////////////////////////////////////////////
#define DHTPIN 19
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//////////////////////////////////////////////
// E-Paper Display Setup (TTGO T5 V2.3)
//////////////////////////////////////////////
GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> display(GxEPD2_213_B74(/*CS=*/5, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));

//////////////////////////////////////////////
// Setup
//////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  delay(100);

  // DHT starten
  dht.begin();

  // RemoteXY starten (Access Point)
  RemoteXY_Init();

  // Display initialisieren
  display.init(115200);
  display.setRotation(1);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold12pt7b);

  // Startbildschirm
  display.firstPage();
  do {
    display.setCursor(10, 30);
    display.print("TTGO T5 DHT22");
    display.setCursor(10, 60);
    display.print("Starte WiFi...");
  } while (display.nextPage());

  Serial.println("Setup abgeschlossen!");
}

//////////////////////////////////////////////
// Hauptloop
//////////////////////////////////////////////
void loop() {
  RemoteXY_Handler();  // Kommunikation mit der App

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    RemoteXY.humidity = h;
    RemoteXY.temperature = t;

    // Serielle Ausgabe
    Serial.printf("Temp: %.1f°C, Feuchte: %.1f%%\n", t, h);

    // Anzeige auf E-Paper
    display.firstPage();
    do {
      display.setCursor(10, 30);
      display.print("Grad: ");
      display.print(t, 1);
      display.print(" C");

      display.setCursor(10, 60);
      display.print("Feuchte: ");
      display.print(h, 1);
      display.print(" %");
    } while (display.nextPage());
  } else {
    Serial.println("DHT22 Fehler: keine gültigen Werte");
  }

  delay(30000);  // alle 5 Sekunden aktualisieren
}

