#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "HTTPUpdateServer.h"
#define HOSTIDENTIFY  "esp32"
#define mDNSUpdate(c)  do {} while(0)
using WebServerClass = WebServer;
using HTTPUpdateServerClass = HTTPUpdateServer;
#include <WiFiClient.h>
#include <AutoConnect.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "time.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "FirebaseESP32.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature suhu(&oneWire);

struct tm timeinfo;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 25200; //gmt +7
const int   daylightOffset_sec = 0;

// Fix hostname for mDNS. It is a requirement for the lightweight update feature.
static const char* host = HOSTIDENTIFY "-webupdate";
#define HTTP_PORT 80

// ESP8266WebServer instance will be shared both AutoConnect and UpdateServer.
WebServerClass  httpServer(HTTP_PORT);

#define USERNAME "user"   //*< Replace the actual username you want */
#define PASSWORD "pass"   //*< Replace the actual password you want */
// Declare AutoConnectAux to bind the HTTPWebUpdateServer via /update url
// and call it from the menu.
// The custom web page is an empty page that does not contain AutoConnectElements.
// Its content will be emitted by ESP8266HTTPUpdateServer.
HTTPUpdateServerClass httpUpdater;
AutoConnectAux  update("/update", "Update");

// Declare AutoConnect and the custom web pages for an application sketch.
AutoConnect     portal(httpServer);

//deklarasi FirebaseESP32
FirebaseData firebaseData;
String configPath = "";
//define sensor
#define pHpin 34
#define turbiditypin 35
#define waterlevelpin 32
#define temppin 27

//linear regression
float a_pH = -30.7487;
float b_pH = 171.4479;

float a_turbidity = 3052.671667;
float b_turbidity = 13.2855;

float a_temp = -1.70;
float b_temp = 1;

float a_waterlevel = 1299.643;
float b_waterlevel = 155.7143;

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("\nBooting Sketch...");

  // Prepare the ESP8266HTTPUpdateServer
  // The /update handler will be registered during this function.
  httpUpdater.setup(&httpServer, USERNAME, PASSWORD);

  // Load a custom web page for a sketch and a dummy page for the updater.
  portal.join({ update });

  if (portal.begin()) {
    if (MDNS.begin(host)) {
        MDNS.addService("http", "tcp", HTTP_PORT);
        Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
    }
    else
      Serial.println("Error setting up MDNS responder");
  }


  // OTA
  ArduinoOTA.setHostname("proyektambak");
  ArduinoOTA.setPassword("admin");
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      Serial.println();
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  
  suhu.begin();

  Firebase.begin("pengabdian-tambak.firebaseio.com", "4OzLJPrPPyyPpEPjVNoDWcoEFca57KJAGnCSnywy");
  Firebase.setMaxRetry(firebaseData, 5);

}

void loop() {
  // Sketches the application here.

  // Invokes mDNS::update and AutoConnect::handleClient() for the menu processing.
  mDNSUpdate(MDNS);
  portal.handleClient();
  ArduinoOTA.handle();
  Serial.println("Proses Baca Data Seluruh Sensor");
  Serial.println("-------------------------------");

  Serial.println("proses baca data sensor pH mulai");
  int pHraw = analogRead(pHpin);
  //linear regression for pH measurement
  pHcalc(pHraw,a_pH,b_pH);
  Serial.println("proses baca data sensor pH selesai");

  Serial.println("proses baca data sensor kekeruhan mulai");
  int turbidityraw = analogRead(turbiditypin);
  //linear regression for turbidity measurement
  turbiditycalc(turbidityraw,a_turbidity,b_turbidity);
  Serial.println("proses baca data sensor kekeruhan selesai");

  Serial.println("proses baca data sensor suhu mulai");
  suhu.requestTemperatures();
  float tempraw = suhu.getTempCByIndex(0);
  //linear regression for temperature measurement
  tempcalc(tempraw,a_temp,b_temp);
  Serial.println("proses baca data sensor suhu selesai");

  Serial.println("proses baca data sensor ketinggian mulai");
  //linear regression for water level measurement
  int waterlevelraw = analogRead(waterlevelpin);
  waterlevelcalc(waterlevelraw,a_waterlevel,b_waterlevel);
  Serial.println("proses baca data sensor ketinggian selesai");

  Serial.println("-----------------------------");
  Serial.println("");
  Serial.println("jarak pandang   =");
  Serial.println("pH              =");
  Serial.println("ketinggian      =");
  Serial.println("ketinggian      =");
}

void printLocalTime()
{

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

float pHcalc(float raw, float a, float b){
  float result;
  result = (raw + a) / b;
  return result;
}

float turbiditycalc(float raw, float a, float b){
  float result;
  result = (raw - a) / b;
  return result;
}

float tempcalc(float raw, float a, float b){
  float result;
  result = (raw - a) / b;
  return result;
}

float waterlevelcalc(float raw, float a, float b){
  float result;
  result = (raw - a) / b;
  return result;
}
