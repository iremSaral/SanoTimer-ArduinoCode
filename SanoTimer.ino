//Define Libraries
#include <FirebaseESP8266.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
//Define necessary information//Define Libraries
#include <FirebaseESP8266.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
//Define necessary informations
//Wifi informations
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
//Firebase
#define API_KEY ""//Firebase Apikey
#define DATABASE_URL ""
#define USER_EMAIL ""
#define USER_PASSWORD ""
// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
WiFiClientSecure ssl_client;
//Define outputs  
const int ledPins[] = {D1, D5, D6, D7, D8}; // LED pinleri

const long utcOffsetInSeconds = 3 * 3600;
//Read Current Date
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//To Connecting Wifi
void ConnectWifi()
{
  WiFi.disconnect();//Varolan bağlantıyı sıfırlar
  Serial.print("Disconnect");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial_Printf("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Define the callback function to handle server status acknowledgement
void networkStatusRequestCallback()
{
  // Set the network status
  fbdo.setNetworkStatus(WiFi.status() == WL_CONNECTED);
}

//To connecting Firebase Database
void ConnectFirebase() {
  Serial_Printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  ssl_client.setInsecure();
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  fbdo.setExternalClient(&ssl_client);
  fbdo.setExternalClientCallbacks(ConnectWifi, networkStatusRequestCallback);
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
  Firebase.begin(&config, &auth);
}

//Run test mode
void TestMode() {
  String command = "normal";
  for (int i = 0; i < sizeof(ledPins) / sizeof(ledPins[0]); i++) {
    digitalWrite(ledPins[i], HIGH); // LED'i yak
    delay(500); // 0.5 saniye bekle
    digitalWrite(ledPins[i], LOW); // LED'i söndür
  }
  Serial_Printf("Completed mode...\n", Firebase.set(fbdo, F("/command"), command) ? "ok" : fbdo.errorReason().c_str());
}

//Run mannual mode
void MannualMode() {
  int duration = 0;
  int lednum = 0;
  int statu = 0;
  String command = "normal";
  if (Firebase.ready()) {
    if (Firebase.get(fbdo, F("/mannualmode/led"))) {
      if (fbdo.dataType() == "int") {
        lednum = fbdo.intData();
        Serial.print("LED value: ");
        Serial.println(lednum);
      }
    }
    if (Firebase.get(fbdo, F("/mannualmode/duration"))) {
      if (fbdo.dataType() == "int") {
        duration = fbdo.intData();
        Serial.print("duration ");
        Serial.println(duration);
      }
    }
    if (Firebase.get(fbdo, F("/mannualmode/status"))) {
      if (fbdo.dataType() == "int") {
        statu = fbdo.intData();
        Serial.print("statu ");
        Serial.println(statu);
      }
    }
    if (statu == 1) {
      digitalWrite(ledPins[lednum - 1], HIGH); // LED'i yak
      delay(duration); // 0.5 saniye bekle
      digitalWrite(ledPins[lednum - 1], LOW);

      Serial_Printf("Completed mode...\n", Firebase.set(fbdo, F("/command"), command) ? "ok" : fbdo.errorReason().c_str());
    }
    else {
      digitalWrite(ledPins[lednum - 1], LOW); // LED'i yak
      delay(duration); // 0.5 saniye bekle
      String data = "normal";
      Serial_Printf("Completed mode...\n", Firebase.set(fbdo, F("/command"), command) ? "ok" : fbdo.errorReason().c_str());
    }
  }
  else {
    Serial.print("Error reading duration value: ");
    Serial.println(fbdo.errorReason());
  }
}

//Struct to store program data
struct programData {
  String gun;
  String starthour;
  String stophour;
  int starth;
  int startm;
  int stoph;
  int stopm;
};

//Struct to store valve data
struct valve {
  String names;
  programData program[5];//Her vananın max beş programı olsun
};

//Allow to define max 5 valves
const int value = 5;
valve valves[5];

//Execution of saved programs
void writed() {

  String currentDay = daysOfTheWeek[timeClient.getDay()];
  int currentHour = timeClient.getHours();
  int currentMinutes = timeClient.getMinutes();
  int currentSeconds = timeClient.getSeconds();

  for (int j = 1; j <= 5; j++) {
    for (int i = 1; i <= 5; i++) {
      if (valves[j].program[i].gun.equals(currentDay) && valves[j].program[i].starth <= currentHour && currentMinutes >=  valves[j].program[i].startm && currentMinutes <=  valves[j].program[i].stopm && currentHour <= valves[j].program[i].stoph) {
        digitalWrite(ledPins[j - 1], HIGH);
        // delay(1000);
        //digitalWrite(ledPins[j - 1], LOW);
        Serial.println("valve");
        Serial.print(j);
        Serial.print("program");
        Serial.print(i);
        delay(1000);

      }

    }
  }

}

//Accessing program logs
void ProgramProcess() {
 String command = "normal";
  Serial.println("ProgramProcess");
  String currentDay = daysOfTheWeek[timeClient.getDay()];
  int currentHour = timeClient.getHours();
  int currentMinutes = timeClient.getMinutes();
  int currentSeconds = timeClient.getSeconds();
  timeClient.update();
  if (Firebase.ready()) {
    for (int j = 1; j <= 5; j++) {
      String valveName = "/programlogs/valve" + String(j);
      valves[j].names = "valve" + String(j);

      if (Firebase.get(fbdo, valveName)) {
        for (int i = 1; i <= 5; i++) {
          String programPath = "/programlogs/" + valves[j].names + "/program" + String(i);

          if (Firebase.get(fbdo, programPath + "/day")) {
            if (fbdo.dataType() == "string") {
              valves[j].program[i].gun = fbdo.stringData();
            }
          }

          if (Firebase.get(fbdo, programPath + "/start")) {
            {

              String starth = fbdo.stringData().substring(0, 2);
              String startm = fbdo.stringData().substring(3);
              valves[j].program[i].starth = starth.toInt();
              valves[j].program[i].startm = startm.toInt();

              Serial.println(valves[j].program[i].starth);
              Serial.println(valves[j].program[i].startm);

            }

            if (Firebase.get(fbdo, programPath + "/stop")) {

              String stoph = fbdo.stringData().substring(0, 2);
              String stopm = fbdo.stringData().substring(3);
              valves[j].program[i].stoph = stoph.toInt();
              valves[j].program[i].stopm = stopm.toInt();

              Serial.println(valves[j].program[i].stoph);
              Serial.println(valves[j].program[i].stopm);

            }
          }
        }
      }
    }
     Serial_Printf("Completed mode...\n", Firebase.set(fbdo, F("/command"), command) ? "ok" : fbdo.errorReason().c_str());
  }
}

void setup() {

  Serial.begin(115200);
  ConnectWifi();
  ConnectFirebase();
  timeClient.begin();
  //Set the output pins
  for (int i = 0; i < sizeof(ledPins) / sizeof(ledPins[0]); i++) {
    pinMode(ledPins[i], OUTPUT); 

  }
  //First time of access to program data
  ProgramProcess();
}

void loop() {

  writed();

  //A "command" is read every second
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    String data2; //command
    
    if (Firebase.get(fbdo, F("/command")))
    {
      if (fbdo.dataType() == "string")
      {
        data2 = fbdo.stringData();

        Serial_Printf("Data: %s\n", data2.c_str());
        if (data2 == "test") {
          Serial.println("testmode");
          TestMode();
        }
        else if (data2 == "mannual") {
          Serial.println("mannualmode");
          MannualMode();
        }
        else if (data2 == "normal") {
          writed();
        }
        else if (data2 == "update") {
          //Güncelleme, program ekleme durumlarında tekrar veriler okunur.
          ProgramProcess();
        }
      }
    }
    else
    {
      Serial_Printf("Error in reading data: %s\n", fbdo.errorReason().c_str());
    }
  }

}s
/* 1. Define the WiFi credentials */
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
/* 2. Define the API Key */
#define API_KEY ""//Firebase Apikey
/* 3. Define the RTDB URL */
#define DATABASE_URL ""
/* 4. Define the user Email and password that alreadey registerd or added in your project in Authentication*/
#define USER_EMAIL ""
#define USER_PASSWORD ""
// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
WiFiClientSecure ssl_client;

const int ledPins[] = {D1, D5, D6, D7, D8}; // LED pinleri
const long utcOffsetInSeconds = 3 * 3600;
//Read Current Date
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void ConnectWifi()
{
  WiFi.disconnect();//Varolan bağlantıyı sıfırlar
  Serial.print("Disconnect");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial_Printf("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Define the callback function to handle server status acknowledgement
void networkStatusRequestCallback()
{
  // Set the network status
  fbdo.setNetworkStatus(WiFi.status() == WL_CONNECTED);
}
//Tamamlandı
void ConnectFirebase() {
  Serial_Printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  ssl_client.setInsecure();
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  fbdo.setExternalClient(&ssl_client);
  fbdo.setExternalClientCallbacks(ConnectWifi, networkStatusRequestCallback);
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
  Firebase.begin(&config, &auth);
}
//Tamamlandı
void TestMode() {
  String command = "normal";
  for (int i = 0; i < sizeof(ledPins) / sizeof(ledPins[0]); i++) {
    digitalWrite(ledPins[i], HIGH); // LED'i yak
    delay(500); // 0.5 saniye bekle
    digitalWrite(ledPins[i], LOW); // LED'i söndür
  }
  //Serial.println("testtesttesttesttest");
  Serial_Printf("Completed mode...\n", Firebase.set(fbdo, F("/command"), command) ? "ok" : fbdo.errorReason().c_str());
}
//Tamamlandı
void MannualMode() {
  int duration = 0;
  int lednum = 0;
  int statu = 0;
  String command = "normal";
  if (Firebase.ready()) {
    if (Firebase.get(fbdo, F("/mannual/led"))) {
      if (fbdo.dataType() == "int") {
        lednum = fbdo.intData();
        Serial.print("LED value: ");
        Serial.println(lednum);
      }
    }
    if (Firebase.get(fbdo, F("/mannual/duration"))) {
      if (fbdo.dataType() == "int") {
        duration = fbdo.intData();
        Serial.print("duration ");
        Serial.println(duration);
      }
    }
    if (Firebase.get(fbdo, F("/mannual/status"))) {
      if (fbdo.dataType() == "int") {
        statu = fbdo.intData();
        Serial.print("statu ");
        Serial.println(statu);
      }
    }
    if (statu == 1) {
      digitalWrite(ledPins[lednum - 1], HIGH); // LED'i yak
      delay(duration); // 0.5 saniye bekle
      digitalWrite(ledPins[lednum - 1], LOW);

      Serial_Printf("Completed mode...\n", Firebase.set(fbdo, F("/command"), command) ? "ok" : fbdo.errorReason().c_str());
    }
    else {
      digitalWrite(ledPins[lednum - 1], LOW); // LED'i yak
      delay(duration); // 0.5 saniye bekle
      String data = "normal";
      Serial_Printf("Completed mode...\n", Firebase.set(fbdo, F("/command"), command) ? "ok" : fbdo.errorReason().c_str());
    }
  }
  else {
    Serial.print("Error reading duration value: ");
    Serial.println(fbdo.errorReason());
  }
}
struct programData {
  String gun;
  String starthour;
  String stophour;
  int starth;
  int startm;
  int stoph;
  int stopm;
};

struct valve {
  String names;
  programData program[5];//Her vananın max beş programı olsun
};

const int value = 5;
valve valves[5];

void writed() {

  String currentDay = daysOfTheWeek[timeClient.getDay()];
  int currentHour = timeClient.getHours();
  int currentMinutes = timeClient.getMinutes();
  int currentSeconds = timeClient.getSeconds();

  for (int j = 1; j <= 5; j++) {
    for (int i = 1; i <= 5; i++) {
      if (valves[j].program[i].gun.equals(currentDay) && valves[j].program[i].starth <= currentHour && currentMinutes >=  valves[j].program[i].startm && currentMinutes <=  valves[j].program[i].stopm && currentHour <= valves[j].program[i].stoph) {

        Serial.println("valve");
        Serial.print(j);
        Serial.print("program");
        Serial.print(i);
        delay(1000);

      }

    }
  }


}
void ProgramProcess() {
  Serial.println("ProgramProcess");
  String currentDay = daysOfTheWeek[timeClient.getDay()];
  int currentHour = timeClient.getHours();
  int currentMinutes = timeClient.getMinutes();
  int currentSeconds = timeClient.getSeconds();
  timeClient.update();
  if (Firebase.ready()) {
    for (int j = 1; j <= 5; j++) {
      String valveName = "/mannualmode/valve" + String(j);
      valves[j].names = "valve" + String(j);

      if (Firebase.get(fbdo, valveName)) {
        for (int i = 1; i <= 5; i++) {
          String programPath = "/mannualmode/" + valves[j].names + "/program" + String(i);

          if (Firebase.get(fbdo, programPath + "/day")) {
            if (fbdo.dataType() == "string") {
              valves[j].program[i].gun = fbdo.stringData();
            }
          }

          if (Firebase.get(fbdo, programPath + "/start")) {
            {

              String starth = fbdo.stringData().substring(0, 2);
              String startm = fbdo.stringData().substring(3);
              valves[j].program[i].starth = starth.toInt();
              valves[j].program[i].startm = startm.toInt();

              Serial.println(valves[j].program[i].starth);
              Serial.println(valves[j].program[i].startm);

            }

            if (Firebase.get(fbdo, programPath + "/stop")) {

              String stoph = fbdo.stringData().substring(0, 2);
              String stopm = fbdo.stringData().substring(3);
              valves[j].program[i].stoph = stoph.toInt();
              valves[j].program[i].stopm = stopm.toInt();

              Serial.println(valves[j].program[i].stoph);
              Serial.println(valves[j].program[i].stopm);

            }
          }
        }
      }
    }
  }
}

void setup() {

  Serial.begin(115200);
  ConnectWifi();
  ConnectFirebase();
  timeClient.begin();
  for (int i = 0; i < sizeof(ledPins) / sizeof(ledPins[0]); i++) {
    pinMode(ledPins[i], OUTPUT); // LED pinlerini çıkış olarak ayarla  }

  }
  ProgramProcess();
}

void loop() {

  writed();
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 300000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    String data2;
    if (Firebase.get(fbdo, F("/command")))
    {
      if (fbdo.dataType() == "string")
      {
        data2 = fbdo.stringData();

        Serial_Printf("Data: %s\n", data2.c_str());
        if (data2 == "test") {
          Serial.println("testmode");
          TestMode();
        }
        else if (data2 == "mannual") {
          Serial.println("mannualmode");
          MannualMode();
        }
        else if (data2 == "normal") {
          writed();
        }
      }
    }
    else
    {
      Serial_Printf("Error in reading data: %s\n", fbdo.errorReason().c_str());
    }
  }

}
