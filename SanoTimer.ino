//Define Libraries
#include <FirebaseESP8266.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <addons/TokenHelper.h> // Provide the token generation process info.
#include <addons/RTDBHelper.h> // Provide the RTDB payload printing info and other helper functions.
//Define necessary informations
/* 1. Define the WiFi credentials */
#define WIFI_SSID "Air5650v3"
#define WIFI_PASSWORD "saralarzu1980"
/* 2. Define the API Key */
#define API_KEY "AIzaSyB-8Gjiwm7yFAJvg0S4-0ZSz5cz-GRS2J4"//Firebase Apikey
/* 3. Define the RTDB URL */
#define DATABASE_URL "esp8266-bd268-default-rtdb.firebaseio.com"
/* 4. Define the user Email and password that alreadey registerd or added in your project in Authentication*/
#define USER_EMAIL "iremsaral2013@gmail.com"
#define USER_PASSWORD "19Hunat."
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

void ConnectFirebase() {
  Serial_Printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  ssl_client.setInsecure();
  /* Assign the api key (required) */
  config.api_key = API_KEY;
  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  /* fbdo.setExternalClient and fbdo.setExternalClientCallbacks must be called before Firebase.begin */
  /* Assign the pointer to global defined external SSL Client object */
  fbdo.setExternalClient(&ssl_client);
  /* Assign the required callback functions */
  fbdo.setExternalClientCallbacks(ConnectWifi, networkStatusRequestCallback);
  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
  Firebase.begin(&config, &auth);
}

void TestMode() {
  String command = "normal";
  for (int i = 0; i < sizeof(ledPins) / sizeof(ledPins[0]); i++) {
    digitalWrite(ledPins[i], HIGH); // LED'i yak
    delay(500); // 0.5 saniye bekle
    digitalWrite(ledPins[i], LOW); // LED'i söndür
  }
  Serial_Printf("Completed mode...\n", Firebase.set(fbdo, F("/command"), command) ? "ok" : fbdo.errorReason().c_str());
}

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

void writeData() {
  Serial.println("writedata");
  for (int i = 1; i <= 5; i++) {
    for (int j = 1; j <= 5; j++) {

      Serial.println(valves[i].program[j].starthour);
    }
  }
}

bool controlTime(int startTime, int stopTime) {
  Serial.print("ControlTime");
  timeClient.update();
  String currentDay = daysOfTheWeek[timeClient.getDay()];
  int currentHour = timeClient.getHours();
  int currentMinutes = timeClient.getMinutes();
  int currentSeconds = timeClient.getSeconds();
  if (startTime <= currentHour && stopTime >= currentHour) {
    Serial.println("true");
    return true;

  }
  else {
    Serial.println("false");
    return false;
  }
}

void ControlDate() {
  Serial.print("ControlDate");
    for (int i = 1; i <= 5; i++) { //valves
      for (int j = 1; j <= 5; j++) { //program
        bool conc = controlTime(valves[i].program[j].starth, valves[i].program[j].stoph);
        if (conc) {
          digitalWrite(ledPins[i - 1], HIGH);
          Serial.println("valves");
          Serial.print(i);
          Serial.println("Programs");
          Serial.print(j);
        }
      }
    }

}

void ConvertInt() {
  Serial.println("ConvertInt");
  //Bir önceki fonksiyonda String olarak okunan başlangıç ve duruş saatlerini saat,dakika olarak ayırıp integera çeviririm.
  for (int i = 1; i <= 5; i++) { //valves
    for (int j = 1; j <= 5; j++) { //program
      String readStartHour = valves[i].program[j].starthour;
      String hourVal = readStartHour.substring(0, 2);
      String MinVal = readStartHour.substring(3);
      valves[i].program [j].starth = hourVal.toInt();
      valves[i].program[j].startm = MinVal.toInt();

      String readStopHour = valves[i].program[j].stophour;
      String stopHourVal = readStopHour.substring(0, 2);
      String stopMinVal = readStopHour.substring(3);
      valves[i].program[j].stoph = stopHourVal.toInt();
      valves[i].program[j].stopm = stopMinVal.toInt();
      Serial.println(i);
      Serial.println(valves[i].program[j].starth);
      Serial.println( valves[i].program[j].stoph);

    }
  }
  Serial.println("Bitti");

    }




void  ProgramProcess() {
  Serial.println("ProgramProcess");
  //Serial.println(timeClient.getFormattedTime());
  if (Firebase.ready()) {
    for (int j = 1; j <= 5; j++)
    {
      String valveName = "/mannualmode/valve" + String(j);
      valves[j].names = "valve" + String(j);
      //Serial.println(valves[j].names);
      if (Firebase.get(fbdo, valveName)) {
        //Serial.println(valves[0].names);
        for (int i = 1; i <= 5; i++) {
          String programPath = "/mannualmode/" + valves[j].names + "/program" + String(i);
          //  Serial.println(programPath);
          if (Firebase.get(fbdo, programPath + "/day")) {
            if (fbdo.dataType() == "string") {
              valves[j].program[i].gun = fbdo.stringData();
              //    Serial.println(valves[j].program[i].gun.c_str());
            }
          }
          if (Firebase.get(fbdo, programPath + "/start" )) {
            if (fbdo.dataType() == "string") {
              valves[j].program[i].starthour = fbdo.stringData();
              //  Serial.println(valves[j].program[i].starthour.c_str());
            }
          }
          if (Firebase.get(fbdo, programPath + "/stop")) {
            if (fbdo.dataType() == "string") {
              valves[j].program[i].stophour = fbdo.stringData();
              //Serial.println(valves[j].program[i].stophour.c_str());
            }
          }
        }
      }
    }
  }
   writeData();
  //ConvertInt();
}



void setup() {

  Serial.begin(115200);
  ConnectWifi();
  ConnectFirebase();
  timeClient.begin();
  for (int i = 0; i < sizeof(ledPins) / sizeof(ledPins[0]); i++) {
    pinMode(ledPins[i], OUTPUT); // LED pinlerini çıkış olarak ayarla
    // Serial.println(i);
  }

}


void loop() {
  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1500 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();

    //String data="irem";
    //Serial_Printf("Set bool... %s\n", Firebase.set(fbdo, F("/test/data"),data) ? "ok" : fbdo.errorReason().c_str());
    //get data
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
          //Serial.println("normalmode");
          ProgramProcess();
        }
      }
    }
    else
    {
      Serial_Printf("Error in reading data: %s\n", fbdo.errorReason().c_str());
    }
  }

}
