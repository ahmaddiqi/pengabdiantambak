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

//define sensor
#define pH 34
#define turbidity 35
#define waterlevel 32
#define temp 27

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
  
  
}

void loop() {
  // Sketches the application here.

  // Invokes mDNS::update and AutoConnect::handleClient() for the menu processing.
  mDNSUpdate(MDNS);
  portal.handleClient();
  ArduinoOTA.handle();
//  Serial.println((String)"DATA,DATE,TIME,"+analogRead(35)+",AUTOSCROLL_20");
  delay(500);
  Serial.println("Proses Baca Data Seluruh Sensor");
  Serial.println("-------------------------------");
  Serial.println("proses baca data sensor pH mulai");
  delay(500);
  Serial.println("proses baca data sensor pH selesai");
  Serial.println("proses baca data sensor kekeruhan mulai");
  delay(500);
  Serial.println("proses baca data sensor kekeruhan selesai");
  Serial.println("proses baca data sensor suhu mulai");
  delay(500);
  Serial.println("proses baca data sensor suhu selesai");
  Serial.println("proses baca data sensor ketinggian mulai");
  delay(500);
  Serial.println("proses baca data sensor ketinggian selesai");
  Serial.println("-----------------------------");
  Serial.println("");
  Serial.println("jarak pandang   = 56,2cm");
  Serial.println("pH              = 7,01");
  Serial.println("ketinggian      = 3,4");
  Serial.println("ketinggian      = 23,8 degree Celcius");
}

void printLocalTime()
{

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
