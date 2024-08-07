/************************* Project Overview******************************************************************/
// Kevin Root
//4/11/23
//PlantCare
//Included libraries, setup, setup function, loop function
//WiFiConnect, AIOConnect, Sensor, Watering, Timer, Post, Get, Schedule
/************************* Included Libraries ***************************************************************/
#include <WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
/************************* Relay Setup **********************************************************************/
#define PUMP_PIN 14                                       // pin that controls pump
#define VALVE_PIN 13                                      // pin that controls first valve
#define VALVE_PIN2 12                                    //pin that controls second valve
/************************* Sensor Setup *********************************************************************/
#define AOUT_PIN 11                                      //moisture sensor pin
#define AOUT_PIN2 10                                     //second moisture sensor pin
/************************* Adafruit.io Setup ****************************************************************/
#define AIO_SERVER      "io.adafruit.com"                  // Adafruit.io MQTT server
#define AIO_SERVERPORT  1883                               // Adafruit.io MQTT server port
#define AIO_USERNAME    "kevinroot"                        // Adafruit.io username
#define AIO_KEY         "aio_OhUC65cMgTlbcgL7Ww8TIW7X3axv" // Adafruit.io key
#define AIO_FEED        "moist"                            // Adafruit.io feed name
#define AIO_FEED2       "moistier"                         // Adafruit.io second feed name
/************************* WiFi Setup ***********************************************************************/
#define WIFI_SSID       "Kevin's iPhone"                   // Wi-Fi SSID
#define WIFI_PASS       "abcdefgh"                         // Wi-Fi password                      
/************************* Server Setup *********************************************************************/
WiFiClient espClient;  
int serverPort = 443;
const char* postRequestUrl = "https://webhook.site/4de5f86d-e288-46a9-b379-957cf26fe7aa";
const char* websiteUrl = "https://h2bros.ddns.net/cards";
const char* websiteUrl2 = "https://h2bros.ddns.net/set_last_watered";
/************************* Adafruit.io MQTT Client Setup ****************************************************/
Adafruit_MQTT_Client mqttClient(&espClient, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
/************************* Adafruit.io MQTT Feeds ***********************************************************/
Adafruit_MQTT_Publish mqttPublishFeed = Adafruit_MQTT_Publish(&mqttClient, AIO_USERNAME "/feeds/" AIO_FEED);
Adafruit_MQTT_Publish mqttPublishFeed2 = Adafruit_MQTT_Publish(&mqttClient, AIO_USERNAME "/feeds/" AIO_FEED2);
/************************* Timer Setup **********************************************************************/
bool isRunning = true;
unsigned long startTime = 0;
unsigned long elapsedTime = 0;
/************************* JSON Setup **********************************************************************/
String CycleTime = "";
int ID1 = 0;
int ID2 = 0;
String PlantName1;
String plantName2;
String dID1;
String dID2;
int dur1;
int dur2;
int tpw1;
int tpd1;
int tpw2;
int tpd2;
int contpd1 = 0;
int contpd2 = 0;
String first;
String sec = "";














/************************* Setup Function *******************************************************************/
void setup() {
   Serial.begin(115200);
   delay(10000);
   connectToWiFi();
   connectToAdafruitIO();
   pinMode(PUMP_PIN, OUTPUT); //set output pin for relay 
   pinMode(VALVE_PIN, OUTPUT);
   pinMode(VALVE_PIN2, OUTPUT);
   startTime = millis();
   RunningTimer();
}
/************************* Loop Function ********************************************************************/
void loop() {
  //get sensor data and display it to the io feed
  SensorData();

  //get data from website and save it as a json string
  CycleTime = getRequest(); 


  //parse json file(s) to get useable data 
  int lastBraceIndex = CycleTime.indexOf('}');
  // CycleTime example -> [{"ID":43,"PlantName":"Plant 1","DeviceID":"abc_1","Duration":15,"TimesPerWeek":6,"TimesPerDay":5,"Show":0},{"ID":45,"PlantName":"Plant 2","DeviceID":"abc_2","Duration":90,"TimesPerWeek":9,"TimesPerDay":5,"Show":0}]
  if (CycleTime.charAt(lastBraceIndex + 2) != ']'){
  first = CycleTime.substring(0, lastBraceIndex + 1);
  first += "]"; //make sure first half is a valid json file
  sec = CycleTime.substring(lastBraceIndex + 1);
  sec[0] = '['; //make sure second half is a valid json file
  }else {
    first = CycleTime;
  }
  parseJsonString(first, ID1, PlantName1, dID1, dur1, tpw1, tpd1);
  if (sec != ""){
  parseJsonString(sec, ID2, plantName2, dID2, dur2, tpw2, tpd2);
  }
  
  /*
Serial.println(ID1);
Serial.println(ID2);
Serial.println(dur1);
Serial.println(dur2);
Serial.println(tpd1);
Serial.println(tpd2);
*/



  //edit times per day value from website to the same units as the timer
  if (tpd1 != 0 && tpd2 != 0){
  tpd1 = (24*3600000) / tpd1; //24 hours * 3600000 milliseconds / number of times per day = time between waters
  tpd2 = (24*3600000) / tpd2;
  } 
 // Serial.println(tpd1);
 // Serial.println(tpd2);     
  //watering functions that double check if its time to water. Water on first successful loop too 
 // if ((elapsedTime >= contpd1) && (ID1 != 0)(//moisture < upRail) && (moisture > lowRail)){
 // contpd1 = tpd1 + contpd1;
  WaterPlant(dur1);
 // postRequest(ID1, dID1);
 // } 
 // if ((elapsedTime >= contpd2) && (ID2 != 0) && (//moisture < upRail) && (moisture > lowRail)){
 // contpd2 = tpd2 + contpd2;
  WaterPlant(dur2);
 // postRequest(ID2, dID2); 
 // }
  //delay for one second. yolo 
//Serial.println(elapsedTime);
  delay(30000);
  Serial.println(dur1);
  Serial.println(dur2);
  Serial.println(elapsedTime);
  
}
/************************* Helper Functions *****************************************************************/











/************************* WiFi Connection ******************************************************************/
void connectToWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to Wi-Fi");
}
/************************* ADAfruitIO Connection ************************************************************/
void connectToAdafruitIO() {
  Serial.println("Connecting to Adafruit.io...");
  while (!mqttClient.connected()) {
  mqttClient.connect();
  if(mqttClient.connected()){
  Serial.println("connected to feed");
  }else{
  Serial.println("no connection");
  }
  }
}
/************************* Sensor function ******************************************************************/
void SensorData(){
 // Read data from 2 moisture sensors
  float value = analogRead(AOUT_PIN); //read moisture sensor from ESP32 pin 
  float value2 = analogRead(AOUT_PIN2);
  value = value / 360;
  value2 = value2 / 360;
  // Publish a message to Adafruit.io feed
  mqttPublishFeed.publish(value); // Publish the value of the first sensor to the feed
  Serial.println(value);
  mqttPublishFeed2.publish(value2); // Publish the value of the second sensor to the feed
  Serial.println(value2);
  delay(1000); // Publish every 1 minute
}
/************************* Water Plant1 Function ************************************************************/
void WaterPlant(int waTime){
// watering function aaaa
  //if (waTime >8000){
    //waTime = 8000;
 // }

  //digitalWrite(VALVE_PIN, LOW);
  //delay(1000);
  digitalWrite(PUMP_PIN, LOW);
  delay(waTime*1000);
  digitalWrite(PUMP_PIN, HIGH);
  //digitalWrite(VALVE_PIN, HIGH)  ;
  delay(10000);

  
  Serial.println("Plant watered!") ; 
  Serial.println(waTime) ; 
}
/************************* Timer Function *******************************************************************/
void RunningTimer(){
   // Check if the timer is running
  if (isRunning) {
  // Calculate the elapsed time
  elapsedTime = millis() - startTime;
  }
  // Display the elapsed time in seconds
  Serial.print("Elapsed Time (s): ");
  Serial.println(elapsedTime / 1000);
  // Delay to reduce CPU usage
  delay(1000);
}

/************************* Get Request Function *************************************************************/
String getRequest(){
  String response;
   HTTPClient http; // delcare http client

  http.begin(websiteUrl); 
  int httpCode = http.GET(); // get

  if (httpCode == HTTP_CODE_OK) { 
   response = http.getString(); // Get response body 
    Serial.println("Response:");
    Serial.println(response); 
  } else {
    Serial.println("Failed to get response");
  }

  http.end(); // End http

  return response;
    }
/************************* Post Function ********************************************************************/
void postRequest(int ID, String abc){
  HTTPClient http; // declare http client
//change this to device ID and abc_1
 String postData = "device_id=" + String(ID) + "&abc=" + abc; // post data 

  http.begin(websiteUrl2); // Begin HTTP session
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Set content type header
  int httpCode = http.POST(postData); // Post

  if (httpCode == HTTP_CODE_OK) { 
    String response = http.getString(); // Get response 
    Serial.println("Response:");
    Serial.println(response); 
  } else {
    Serial.println("Failed to get response");
  }

  http.end(); // End http
}
/************************* Schedule Function ****************************************************************/
void waterSchedule1(int cycle){  
 //done in loop
  }
/************************* JSON Parse Function ***************************************************************/
void parseJsonString(const String& jsonStr,int& ID, String& plantName, String& dID, int& dur, int& tpw, int& tpd) {
  // Parse JSON string
  DynamicJsonDocument jsonDoc(1024);
  DeserializationError error = deserializeJson(jsonDoc, jsonStr);

  // Check for parsing errors
  if (error) {
    Serial.println("Failed to parse JSON");
    return;
  }

  // Get the array of objects from the parsed JSON
  JsonArray jsonArray = jsonDoc.as<JsonArray>();

  // Loop through each object in the array
  for (JsonObject jsonObject : jsonArray) {
    // Extract data from each object
     ID = jsonObject["ID"];
     plantName = jsonObject["PlantName"].as<String>();
     dID = jsonObject["DeviceID"].as<String>();
     dur = jsonObject["Duration"];
     tpw = jsonObject["TimesPerWeek"];
     tpd = jsonObject["TimesPerDay"];
    int show = jsonObject["Show"];

   
  }
}



