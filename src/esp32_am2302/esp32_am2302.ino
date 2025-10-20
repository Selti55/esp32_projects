//////////////////////////////////////////////
//       include library                    //
//////////////////////////////////////////////
#include <DHT.h>      // AM2302 oder DHT11 oder DHT22 (AM2302-->DHT22)
#include <WiFi.h>     // WiFi

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG    

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__WIFI

#include <WiFi.h>

// RemoteXY connection settings 
#define REMOTEXY_WIFI_SSID "A1-A77A4B"
#define REMOTEXY_WIFI_PASSWORD "BZQZK3Y4ZM"
#define REMOTEXY_SERVER_PORT 6377
#define REMOTEXY_ACCESS_PASSWORD "12345678"

#include <RemoteXY.h> // include after define

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 158 bytes
  { 255,0,0,16,0,151,0,19,0,0,0,69,83,80,51,50,95,68,72,84,
  50,50,0,31,1,106,200,1,1,8,0,129,5,17,72,12,64,17,84,104,
  101,109,112,101,114,97,116,117,114,58,0,129,5,83,87,12,64,17,76,117,
  102,116,102,101,117,99,104,116,105,103,107,101,105,116,58,0,67,7,29,53,
  17,78,38,26,3,67,6,97,52,17,78,2,26,3,129,60,30,16,15,64,
  1,194,176,67,0,129,58,98,11,15,64,17,37,0,73,7,52,85,10,5,
  128,0,2,26,0,0,160,193,0,0,32,66,0,0,160,193,73,6,121,85,
  9,5,128,0,2,26,0,0,0,0,0,0,200,66,0,0,0,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // output variables
  float temperature;
  float humidity;
  float linearbar_01; // from -20 to 40
  float linearbar_02; // from 0 to 100

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

// ============================================================
// DHT22 Sensor
// ============================================================
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ============================================================
// setup
// ============================================================
void setup() {
  RemoteXY_Init (); 
   
  // TODO you setup code
  dht.begin();
  // WiFi.begin(ssid, password);
  // delay(2000);
  Serial.begin(115200);
  delay(1000); // Wait for the serial monitor to open
  Serial.println("IP-Adresse:");
  Serial.println(WiFi.localIP());
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
    RemoteXY.linearbar_01=h;
    RemoteXY.temperature = t;
    RemoteXY.linearbar_02=t;
    // RemoteXY.realTime_01=getLocalTime()
    Serial.printf("Temp: %.1ft °C | Feuchte: %.1f %%\n", t, h);
  }
  Serial.println("IP-Adresse:");
  Serial.println(WiFi.localIP());
    
  RemoteXY_delay(5000);
}



