#include <RemoteXY.h>
#include <WiFi.h>
#include <esp_now.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// TFT Display
TFT_eSPI tft = TFT_eSPI();

// Display Größe
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

// RemoteXY BLE Konfiguration - OPTIMIERTES DESIGN
#define REMOTEXY_BLUETOOTH_NAME "WetterStation Pro"

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 372 bytes
  { 255,3,0,45,0,109,1,19,0,0,0,87,101,116,116,101,114,83,116,97,
  116,105,111,110,32,80,114,111,0,31,1,106,200,1,1,28,0,129,3,3,
  102,13,64,1,87,101,116,116,101,114,115,116,97,116,105,111,110,32,80,114,
  111,0,129,3,24,32,5,0,8,65,117,115,115,101,110,115,116,97,116,105,
  111,110,0,67,4,32,29,10,14,1,16,2,67,5,45,30,11,14,6,16,
  1,67,5,58,30,10,14,12,16,1,67,5,71,31,10,30,2,16,2,5,
  84,96,8,0,2,26,31,31,79,78,0,79,70,70,0,129,77,24,16,5,
  0,8,98,105,116,99,111,105,110,0,67,53,33,40,10,14,1,16,2,67,
  54,46,39,10,22,6,16,67,55,60,48,9,6,147,16,12,129,6,102,95,
  10,0,8,68,105,115,112,108,97,121,32,69,105,110,115,116,101,108,108,117,
  110,103,101,110,0,4,7,113,93,11,128,2,26,74,9,127,87,6,13,12,
  30,37,64,87,101,105,115,115,0,2,9,146,44,8,0,2,26,31,31,79,
  78,0,79,70,70,0,67,10,156,28,11,14,1,16,2,67,11,169,27,10,
  14,6,16,2,67,56,157,31,10,14,13,16,2,67,57,169,29,10,30,2,
  16,129,33,32,10,9,0,1,194,176,67,0,129,36,45,9,11,0,6,37,
  0,129,37,59,13,7,0,12,104,80,97,0,129,38,71,9,11,0,17,37,
  0,129,93,34,6,9,0,1,226,130,172,0,129,89,169,9,11,0,17,37,
  0,129,88,158,13,7,0,12,104,80,97,0,129,39,168,9,11,0,6,37,
  0,129,39,157,10,9,0,1,194,176,67,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t switch_aussen; // =1 if switch ON and =0 if OFF, from 0 to 1
  int8_t brightness; // from 0 to 100
  uint8_t switch_innen; // =1 if switch ON and =0 if OFF, from 0 to 1

    // output variables
  float temp_aussen;
  float humi_aussen;
  float press_aussen;
  int8_t bat_aussen; // -128 .. 127
  float btc_price;
  int16_t block_height; // -32768 .. +32767
  char status[12]; // string UTF8 end zero
  uint8_t farbe; // from 0 to 1
  float temp_innen;
  float value_01;
  float prress_innen;
  int8_t bat_innen; // -128 .. 127

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)

// BLE Service und Characteristics
BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pTempCharacteristic;
BLECharacteristic *pHumidityCharacteristic;
BLECharacteristic *pPressureCharacteristic;
BLECharacteristic *pBtcCharacteristic;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define TEMP_UUID           "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define HUMIDITY_UUID       "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define PRESSURE_UUID       "beb5483e-36e1-4688-b7f5-ea07361b26aa"
#define BTC_UUID            "beb5483e-36e1-4688-b7f5-ea07361b26ab"

// WiFi Credentials
const char* ssid = "DEIN_WLAN";
const char* password = "DEIN_PASSWORT";

// BTC API
const String bitcoinAPI = "https://blockchain.info/ticker";
const String blockHeightAPI = "https://blockchain.info/q/getblockcount";

// Wetterdaten Struktur (von Außenstation)
typedef struct struct_message {
  float temperature;
  float humidity;
  float pressure;
  int battery;
  uint32_t timestamp;
} struct_message;

struct_message weatherData;
bool newWeatherData = false;
bool deviceConnected = false;
unsigned long lastDataReceived = 0;

// BLE Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("iPhone verbunden via BLE");
      strcpy(RemoteXY.status, "Verbunden");
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("iPhone getrennt");
      strcpy(RemoteXY.status, "Getrennt");
      // Restart advertising
      pServer->getAdvertising()->start();
    }
};

void setup() {
  Serial.begin(115200);
  
  // Display initialisieren
  setupDisplay();
  
  // RemoteXY initialisieren
  RemoteXY_Init();
  
  // BLE initialisieren
  setupBLE();
  
  // WiFi verbinden
  connectWiFi();
  
  // ESP-NOW initialisieren
  setupESP_NOW();
  
  // Initiale Daten holen
  updateBTCData();
  updateIndoorData();
  
  // Default Werte setzen
  RemoteXY.switch_aussen = 1;
  RemoteXY.switch_innen = 1;
  strcpy(RemoteXY.farbe, "Weiss");
  strcpy(RemoteXY.status, "Bereit");
  
  Serial.println("Wetterstation Pro bereit - BLE aktiv");
}

void loop() {
  RemoteXY_Handler();
  
  // Neue Wetterdaten von Außenstation verarbeiten
  if (newWeatherData) {
    updateAussenData();
    updateBLEData();
    displayWeatherData();
    newWeatherData = false;
    lastDataReceived = millis();
  }
  
  // Daten regelmäßig aktualisieren
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 30000) { // Alle 30 Sekunden
    updateBTCData();
    updateIndoorData();
    updateBLEData();
    updateDisplayData();
    lastUpdate = millis();
  }
  
  // Verbindungsstatus überwachen
  checkConnectionStatus();
  
  // Display Einstellungen anwenden
  applyDisplaySettings();
  
  delay(100);
}

void setupBLE() {
  // BLE Device initialisieren
  BLEDevice::init(REMOTEXY_BLUETOOTH_NAME);
  
  // BLE Server erstellen
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  // BLE Service erstellen
  pService = pServer->createService(SERVICE_UUID);
  
  // Characteristics erstellen
  pTempCharacteristic = pService->createCharacteristic(
    TEMP_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  
  pHumidityCharacteristic = pService->createCharacteristic(
    HUMIDITY_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  
  pPressureCharacteristic = pService->createCharacteristic(
    PRESSURE_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  
  pBtcCharacteristic = pService->createCharacteristic(
    BTC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  
  // Service starten
  pService->start();
  
  // Advertising starten
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE Service gestartet - Warte auf iPhone Verbindung...");
}

void updateAussenData() {
  if (RemoteXY.switch_aussen) {
    RemoteXY.temp_aussen = weatherData.temperature;
    RemoteXY.humi_aussen = weatherData.humidity;
    RemoteXY.press_aussen = weatherData.pressure;
    RemoteXY.bat_aussen = weatherData.battery;
  }
}

void updateIndoorData() {
  if (RemoteXY.switch_innen) {
    // Simulierte Innendaten - könnten später mit zusätzlichem Sensor erweitert werden
    RemoteXY.temp_innen = 22.5 + (rand() % 100 - 50) * 0.1;
    RemoteXY.humi_innen = 45 + (rand() % 100 - 50) * 0.5;
    RemoteXY.press_innen = 1013.0;
    RemoteXY.bat_innen = 100; // Innenstation ist netzbetrieben
  }
}

void updateBTCData() {
  if (WiFi.status() == WL_CONNECTED) {
    // BTC Preis
    HTTPClient http;
    http.begin(bitcoinAPI);
    int httpCode = http.GET();
    
    if (httpCode == 200) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      RemoteXY.btc_price = doc["EUR"]["last"];
    }
    http.end();
    
    // Block Höhe
    http.begin(blockHeightAPI);
    httpCode = http.GET();
    
    if (httpCode == 200) {
      String payload = http.getString();
      RemoteXY.block_height = payload.toInt();
    }
    http.end();
  }
}

void updateBLEData() {
  if (deviceConnected) {
    // Temperatur senden
    String tempStr = String(RemoteXY.temp_aussen, 1);
    pTempCharacteristic->setValue(tempStr.c_str());
    pTempCharacteristic->notify();
    
    // Luftfeuchtigkeit senden
    String humidityStr = String(RemoteXY.humi_aussen, 0);
    pHumidityCharacteristic->setValue(humidityStr.c_str());
    pHumidityCharacteristic->notify();
    
    // Luftdruck senden
    String pressureStr = String(RemoteXY.press_aussen, 0);
    pPressureCharacteristic->setValue(pressureStr.c_str());
    pPressureCharacteristic->notify();
    
    // BTC Preis senden
    String btcStr = String(RemoteXY.btc_price, 0);
    pBtcCharacteristic->setValue(btcStr.c_str());
    pBtcCharacteristic->notify();
  }
}

void checkConnectionStatus() {
  // Überprüfe ob Daten von Außenstation aktuell sind
  if (millis() - lastDataReceived > 120000) { // 2 Minuten ohne Daten
    strcpy(RemoteXY.status, "Keine Daten");
  } else if (deviceConnected) {
    strcpy(RemoteXY.status, "Verbunden");
  } else {
    strcpy(RemoteXY.status, "Bereit");
  }
}

void applyDisplaySettings() {
  // Helligkeit basierend auf Switch
  int brightness = (RemoteXY.switch_innen) ? 80 : 20;
  analogWrite(TFT_LED_PIN, map(brightness, 0, 100, 0, 255));
}

// ESP-NOW Callback für Datenempfang
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&weatherData, incomingData, sizeof(weatherData));
  newWeatherData = true;
  
  Serial.print("Empfangene Wetterdaten: ");
  Serial.print(weatherData.temperature); Serial.print("°C, ");
  Serial.print(weatherData.humidity); Serial.print("%, ");
  Serial.print(weatherData.pressure); Serial.print("hPa, ");
  Serial.print(weatherData.battery); Serial.println("%");
}

void setupESP_NOW() {
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init fehlgeschlagen");
    return;
  }
  
  esp_now_register_recv_cb(OnDataRecv);
}

void setupDisplay() {
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(50, 140);
  tft.println("Wetterstation Pro");
  tft.setCursor(80, 170);
  tft.setTextSize(1);
  tft.println("BLE aktiv - Warte auf Verbindung...");
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
    tft.println("\nVerbunden! BLE aktiv");
  } else {
    tft.println("\nWiFi Verbindung fehlgeschlagen!");
  }
  
  delay(2000);
}

void displayWeatherData() {
  if (!RemoteXY.switch_innen) return;
  
  tft.fillScreen(TFT_BLACK);
  
  // Header
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(120, 10);
  tft.println("WETTERSTATION PRO");
  
  // Außendaten
  tft.setTextSize(1);
  tft.setCursor(20, 50);
  tft.println("AUSSENSTATION:");
  
  tft.setCursor(40, 70);
  tft.print("Temperatur: ");
  tft.print(RemoteXY.temp_aussen, 1);
  tft.println(" C");
  
  tft.setCursor(40, 90);
  tft.print("Luftfeuchtigkeit: ");
  tft.print(RemoteXY.humi_aussen, 0);
  tft.println(" %");
  
  tft.setCursor(40, 110);
  tft.print("Luftdruck: ");
  tft.print(RemoteXY.press_aussen, 0);
  tft.println(" hPa");
  
  tft.setCursor(40, 130);
  tft.print("Batterie: ");
  tft.print(RemoteXY.bat_aussen);
  tft.println(" %");
  
  // BTC Daten
  tft.setCursor(20, 170);
  tft.println("BITCOIN:");
  
  tft.setCursor(40, 190);
  tft.print("Preis: ");
  tft.print(RemoteXY.btc_price, 0);
  tft.println(" EUR");
  
  tft.setCursor(40, 210);
  tft.print("Block: ");
  tft.println(RemoteXY.block_height);
  
  // Status
  tft.setCursor(20, 250);
  tft.print("Status: ");
  tft.println(RemoteXY.status);
  
  tft.setCursor(20, 270);
  tft.print("BLE: ");
  tft.println(deviceConnected ? "Verbunden" : "Getrennt");
}

void updateDisplayData() {
  if (RemoteXY.switch_innen) {
    displayWeatherData();
  }
}