
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include "time.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
using namespace std;
// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Muhammed"
#define WIFI_PASSWORD "12345678931"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDq0iM7txWmKMlEwcdx9cDL8yCQiMQ6yf4"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "mhemd.masa@gmail.com"
#define USER_PASSWORD "Muhammed139"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://smardrop13-default-rtdb.firebaseio.com/"
// The pin connected to my sensor
#define AOUT_PIN 33

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String devicePath = "/Devices/";
String MoistPath = "/moisture";
//String timePath = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

//int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

//Set the name of the dripper
int ID =  100;
String Device = "ESP2"; 

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 10000;

//The Chosen Plant and its Max and Min Moisture level
int MaxMoist;
int MinMoist;
String Plant;

//Set Path for the Plant in RTDB
String MaxPlantPath;
String MinPlantPath;
//Device Connected Plant

//String CurrPlantPath;


// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup(){
  Serial.begin(115200);

  // Initialize BME280 sensor
  initWiFi();
  configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }

  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path databasePath = "/UsersData/" + uid + "/readings";
   databasePath = "/UsersData/";
  //Set the name and the ID of my Device in Firebase
  String devicePath = "/Devices/";
  //Set the Id of your device in Firebase
  Firebase.RTDB.setInt(&fbdo, devicePath +"/"+ID + "/Id" , ID);
}

void loop(){

  parentPath= devicePath +"/"+ID+"/";
  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    int value = analogRead(AOUT_PIN); // read the analog value from sensor
    int moisture = map(value,997,3000,100,0); 

    if (Firebase.RTDB.getString(&fbdo, parentPath + "Plant")) {
        Plant = fbdo.stringData();
    }
    Plant.remove(0,2);
    int i=0;
    while(Plant[i] != '\0') {
        i++;
    }
    Plant.remove(i-1 , 1);
    Plant.remove(i-2 , 1);

    Serial.println(value);
    Serial.println(moisture);
    Serial.println(Plant);
    String MaxPlantPath = "/Plants/"+Plant+"/MaxMoist";
    String MinPlantPath = "/Plants/"+Plant+"/MinMoist";
    if (Firebase.RTDB.getInt(&fbdo, MaxPlantPath)) {
      if (fbdo.dataType() == "int") {
        MaxMoist = fbdo.intData();
      }
    }
    if (Firebase.RTDB.getInt(&fbdo, MinPlantPath)) {
      if (fbdo.dataType() == "int") {
        MinMoist = fbdo.intData();
      }
    }
    if(moisture > MaxMoist){
      Serial.println("Close");

    }
    else if(moisture < MinMoist){
      Serial.println("Open");
    }
    else{
      Serial.println("Moisture is within Range");
    }
    Serial.println(MaxMoist);

    //int MaxMoist = Firebase.RTDB.getInt(&fbdo ,"/Plants/Mint/MaxMoist");
    //Serial.println(MaxMoist);
    //json.set(MoistPath.c_str(), String(moisture));
    //json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setString(&fbdo, parentPath + "moisture" , String(moisture)) ? "ok" : fbdo.errorReason().c_str());
  }
}

