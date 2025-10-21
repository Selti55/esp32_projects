/**
 * Wetterstation Innenstation
 * ESP32 mit TFT Display
 * Empfängt Daten via ESP-NOW und zeigt Wetter + BTC an
 */

#include <WiFi.h>
#include <esp_now.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

// Display Größe
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

// WiFi Credentials
const char* ssid = "DEIN_WLAN_SSID";
const char* password = "DEIN_WLAN_PASSWORT";

// BTC API
const String bitcoinAPI = "https://blockchain.info/ticker";

// Datenstruktur für ESP-NOW
typedef struct struct_message {
  float temperature;
  float humidity;
  float pressure;
  int battery;
} struct_message;

struct_message weatherData;
bool newWeatherData = false;

// BTC Daten
float btcPrice = 0.0;
unsigned long lastBTCUpdate = 0;
const unsigned long BTC_UPDATE_INTERVAL = 60000; // 1 Minute

void setup() {
  Serial.begin(115200);
  
  // Display initialisieren
  setupDisplay();
  
  // WiFi verbinden
  connectWiFi();
  
  // ESP-NOW initialisieren
  setupESP_NOW();
  
  // Erste BTC Daten holen
  updateBTCPrice();
}

void loop() {
  // Neue Wetterdaten anzeigen
  if (newWeatherData) {
    displayWeatherData();
    newWeatherData = false;
  }
  
  // BTC Preis regelmäßig aktualisieren
  if (millis() - lastBTCUpdate > BTC_UPDATE_INTERVAL) {
    updateBTCPrice();
    displayBTCData();
  }
  
  delay(1000);
}

void setupDisplay() {
  tft.init();
  tft.setRotation(3); // Landscape
  tft.fillScreen(TFT_BLACK);
  
  // Willkommensbildschirm
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(50, 140);
  tft.println("Wetterstation Startet...");
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.setTextSize(1);
  tft.println("Verbinde mit WiFi...");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    tft.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    tft.println("\nVerbunden! IP: " + WiFi.localIP().toString());
  } else {
    tft.println("\nWiFi Verbindung fehlgeschlagen!");
  }
  
  delay(2000);
}

void setupESP_NOW() {
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init fehlgeschlagen");
    return;
  }
  
  esp_now_register_recv_cb(OnDataRecv);
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&weatherData, incomingData, sizeof(weatherData));
  newWeatherData = true;
  
  Serial.print("Empfangene Daten: ");
  Serial.print(weatherData.temperature); Serial.print("°C, ");
  Serial.print(weatherData.humidity); Serial.print("%, ");
  Serial.print(weatherData.pressure); Serial.print("hPa, ");
  Serial.print(weatherData.battery); Serial.println("%");
}

void updateBTCPrice() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(bitcoinAPI);
    
    int httpCode = http.GET();
    
    if (httpCode == 200) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      
      btcPrice = doc["EUR"]["last"];
      lastBTCUpdate = millis();
    }
    
    http.end();
  }
}

void displayWeatherData() {
  // Wetterdaten Bereich
  tft.fillRect(0, 0, SCREEN_WIDTH, 200, TFT_BLACK);
  
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 20);
  tft.print("Aussen: ");
  tft.print(weatherData.temperature, 1);
  tft.println(" C");
  
  tft.setCursor(20, 60);
  tft.print("Feuchte: ");
  tft.print(weatherData.humidity, 0);
  tft.println(" %");
  
  tft.setCursor(20, 100);
  tft.print("Druck: ");
  tft.print(weatherData.pressure, 0);
  tft.println(" hPa");
  
  tft.setCursor(20, 140);
  tft.print("Batterie: ");
  tft.print(weatherData.battery);
  tft.println(" %");
}

void displayBTCData() {
  // BTC Daten Bereich
  tft.fillRect(0, 200, SCREEN_WIDTH, 120, TFT_BLACK);
  
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 220);
  tft.print("BTC/EUR: ");
  tft.print(btcPrice, 0);
  tft.println(" EUR");
  
  tft.setCursor(20, 250);
  tft.print("Letztes Update: ");
  tft.print(millis() / 1000);
  tft.println(" s");
}