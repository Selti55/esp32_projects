//////////////////////////////////////////////
//       include library                    //
//////////////////////////////////////////////
#include <DHT.h>      // AM2302 oder DHT11 oder DHT22 (AM2302-->DHT22)
#include <WiFi.h>     // WiFi
#include <RemoteXY.h> // remotexy.com

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG    

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__WIFI_POINT

// RemoteXY connection settings 
#define REMOTEXY_WIFI_SSID "ESP32-DHT22"
#define REMOTEXY_WIFI_PASSWORD "12345678"
#define REMOTEXY_SERVER_PORT 6377
#define REMOTEXY_ACCESS_PASSWORD "12345678"

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 94 bytes
  { 255,0,0,8,0,87,0,19,0,0,0,69,83,80,51,50,95,68,72,84,
  50,50,0,31,1,106,200,1,1,4,0,129,5,17,72,12,64,17,84,104,
  101,109,112,101,114,97,116,117,114,58,0,129,6,68,87,12,64,17,76,117,
  102,116,102,101,117,99,104,116,105,103,107,101,105,116,58,0,67,9,33,84,
  22,78,38,26,3,67,8,85,84,20,78,2,26,3 };
  
// this structure defines all the variables and events of your control interface 
struct {
    // output variables
  float temperature;
  float humidity;
    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0
} RemoteXY;   

#pragma pack(pop)

// ============================================================
// DHT22 Sensor
// ============================================================
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup() 
{
  RemoteXY_Init (); 
  dht.begin();
}

// ============================================================
// Loop
// ============================================================
void loop() {
  // RemoteXY-Kommunikation verarbeiten
  RemoteXY_Handler ();
  // Sensordaten lesen
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    RemoteXY.humidity = h;
    RemoteXY.temperature = t;
  }

  RemoteXY_delay(5000);
}



