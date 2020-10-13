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
#include <FirebaseJson.h>
#include <NewPing.h>

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
AutoConnectConfig Config;

//deklarasi FirebaseESP32
FirebaseData firebaseData;
FirebaseJson jsonSensor;
String configPath = "/device/1/config";
String updatePath = "/device/realtime";
String pushPath = "/device/data";
void printResult(FirebaseData &data);
//define sensor
#define pHpin 34
#define turbiditypin 35
#define temppin 27
#define flushWaterpin 18
#define cleanWaterpin 19
#define tambakWaterpin 21

//linear regression
float a_pH = -30.7487;
float b_pH = 171.4479;
float weight_pH = 0.1;

float a_turbidity = 1362.84112746429;
float b_turbidity = 13.0205945928571;
float weight_turbidity = 0.5;

float a_temp = -1.70;
float b_temp = 1;


String clockConfig[10];
int longClock;

OneWire oneWire(temppin);
DallasTemperature suhu(&oneWire);

#define TRIGGER_PIN  4  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     2  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 20 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

bool debug;
void setup() {
  pinMode(flushWaterpin,OUTPUT);
  pinMode(cleanWaterpin,OUTPUT);
  pinMode(tambakWaterpin,OUTPUT);
  digitalWrite(flushWaterpin,HIGH);
  digitalWrite(cleanWaterpin,LOW);
  digitalWrite(tambakWaterpin,HIGH);
  delay(1000);
  Serial.begin(115200);
  Serial.println("\nBooting Sketch...");
  int timeOut = 0 ;//detik
  WiFi.begin();
  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print(".");
    timeOut -= 1;
    if(timeOut < 1){
      break;
    }
  }
  // Prepare the ESP8266HTTPUpdateServer
  // The /update handler will be registered during this function.
  httpUpdater.setup(&httpServer, USERNAME, PASSWORD);
  Config.autoReconnect = true;
  Config.apid = "Pengabdian tambak - UM";
  Config.psk = "tambak2020";
  Config.title = "Pengabdian tambak - UM";
  Config.retainPortal = true;
  portal.config(Config);
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
  Firebase.setMaxRetry(firebaseData, 10);
  Firebase.reconnectWiFi(true);
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
}

void loop() {
  // Invokes mDNS::update and AutoConnect::handleClient() for the menu processing.
  mDNSUpdate(MDNS);
  portal.handleClient();
  ArduinoOTA.handle();

  // proses ambil data konfigurasi dari web 
    if (Firebase.getJSON(firebaseData,configPath)){
      Serial.println("PASSED");
      FirebaseJson &json = firebaseData.jsonObject();
      FirebaseJsonData jsondata;
      json.get(jsondata,"/debug");
      if (jsondata.success){
      debug = (jsondata.boolValue ? true : false);
      Serial.println(debug); 
      }
      json.get(jsondata,"/updateTime");
      if (jsondata.success){
        FirebaseJsonArray jsonArray;
        jsondata.getArray(jsonArray);
        for (int i = 0; i < jsonArray.size(); i++){
          jsonArray.get(jsondata, i);
          clockConfig[i] = jsondata.stringValue;
          longClock = i+1;
          Serial.println(clockConfig[i]);
        }
        Serial.print("konfigurasi jam ada ");
        Serial.println(longClock);
      }
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
    for (int x = 0; x < longClock; x++){
      while (!getLocalTime(&timeinfo)){
      }
      int hour, minute;
      hour = clockConfig[x].substring(0,2).toInt();
      minute = clockConfig[x].substring(3,5).toInt();
      Serial.print("check jam ");
      Serial.print(hour);
      Serial.print(":");
      Serial.println(minute);
      Serial.print("sekarang jam ");
      Serial.print(timeinfo.tm_hour);
      Serial.print(":");
      Serial.println(timeinfo.tm_min);
      if (hour == timeinfo.tm_hour && minute == timeinfo.tm_min){
        if (debug != true){
          flushWater(true); // buang air sekarang
          Serial.println("kran pembuangan dibuka");
          while (checklevel() <18){
          }
          flushWater(false); //tutup kran
          Serial.println("kran pembuangan ditutup");
          Serial.println("aktuator tambak dibuka");
          sampleTambak(true); //buka kran tambak
          while(checklevel() > 10){
          }
          sampleTambak(false); //tutup kran tambak
          Serial.println("aktuator tambak ditutup");
        }        
        Serial.println("Proses Baca Data Seluruh Sensor");
        Serial.println("-------------------------------");

        Serial.println("proses baca data sensor pH mulai");
        float pHrate = 0;
        for (int a = 0; a<150;a++){
          int pHraw = analogRead(pHpin);
          pHrate = weight_pH * pHraw + (1 - weight_pH) * pHrate;
          Serial.println(pHraw);
          delay(20);
        }
        float pH = pHcalc(pHrate,a_pH,b_pH); //linear regression for pH measurement
        
        Serial.print("nilai pHnya adalah ");
        Serial.println(pH);
        jsonSensor.add("pH",String(pH)); 
        Serial.println("proses baca data sensor pH selesai");

        Serial.println("proses baca data sensor kekeruhan mulai");
        float turbidityRate = 0;
          for (int a = 0; a<100;a++){
          int turbidityRaw = analogRead(turbiditypin);
          turbidityRate = weight_turbidity * turbidityRaw + (1 - weight_turbidity) * turbidityRate;
          Serial.println(turbidityRaw);
          delay(20);
        }
        Serial.println("nilai rata-rata ADC kekeruhan");
        Serial.print(turbidityRate);
        float turbidity = turbiditycalc(turbidityRate,a_turbidity,b_turbidity);   //linear regression for turbidity measurement
        Serial.print("nilai kekeruhannya adalah ");
        Serial.println(turbidity);
        jsonSensor.add("turbidity",String(turbidity));
        Serial.println("proses baca data sensor kekeruhan selesai");

        Serial.println("proses baca data sensor suhu mulai");
        delay(2000);
        suhu.requestTemperatures();
        float tempraw = suhu.getTempCByIndex(0);
        Serial.print(tempraw);
        Serial.println(" derajat celcius");
        //linear regression for temperature measurement
        float temperature = tempcalc(tempraw,a_temp,b_temp);
        jsonSensor.add("temperature",String(temperature));
        Serial.println("proses baca data sensor suhu selesai");

        String years = String(timeinfo.tm_year + 1900);
        String datetime = String(years + "-" + (checktime(timeinfo.tm_mon + 1)) + "-" + (checktime(timeinfo.tm_mday)) + "T" + (checktime(timeinfo.tm_hour)) + ":" + (checktime(timeinfo.tm_min)) + ":" + (checktime(timeinfo.tm_sec))+ "+07:00");
        jsonSensor.add("date",datetime);
        if (Firebase.updateNode(firebaseData,updatePath,jsonSensor)){
          Serial.println("UPDATE NODE PASSED");
        }
        else{
          Serial.println("failed update");
        }

        if (Firebase.pushJSON(firebaseData,pushPath,jsonSensor)){
          Serial.println("PUSH NODE PASSED");
        }
        else{
          Serial.println("failed to push");
        }
        if(debug != true){
          flushWater(true); // buang air sekarang
          Serial.println("kran pembuangan dibuka");
          while (checklevel() <18){
          }
          flushWater(false); //tutup kran
          Serial.println("kran pembuangan ditutup");
          Serial.println("aktuator air bersih dibuka");
          cleanWater(true); //buka kran air bersih
          while(checklevel() > 10){
          }
          cleanWater(false); //tutup kran air bersih
          Serial.println("aktuator air bersih ditutup");
        }
        while(hour == timeinfo.tm_hour && minute == timeinfo.tm_min){
          getLocalTime(&timeinfo);
        }
      }
    }
    delay(1000);
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


void flushWater(bool Switch){
  if (Switch == true){
    digitalWrite(flushWaterpin,LOW);
  }
  else{
    digitalWrite(flushWaterpin,HIGH);
  }
}

void sampleTambak(bool Switch){
  if (Switch == true){
    digitalWrite(tambakWaterpin,LOW);
  }
  else{
    digitalWrite(tambakWaterpin,HIGH);
  }
}

void cleanWater(bool Switch){
  if (Switch == true){
    digitalWrite(cleanWaterpin,HIGH);
  }
  else{
    digitalWrite(cleanWaterpin,LOW);
  }
}

float checklevel(){
  int waterlevelraw = 0;
  for(int i = 0; i < 10; i++){
    int jarak = sonar.ping_cm();
    if( jarak > 0 && jarak < 30){
      waterlevelraw = (jarak + waterlevelraw) / 2;
      delay(50);
    }
    else{
      i--;
    }
  }
  Serial.print("estimasi air ");
  Serial.println(waterlevelraw);
  return waterlevelraw;
}

String checktime (int check){
  String aftercheck;
  if(check < 10){
    aftercheck = String(String(0) + check);
    return aftercheck;
  }
  else { //if (check >= 10)
    aftercheck = String(check);
    return aftercheck;
  }
}
