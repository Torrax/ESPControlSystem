/////////////////////////////////////////////
//                                         //
//         Authors: Carson Morris          //
//             & Salim El Atache           //
//         Class: CAM8302E - 012           //
//               Lab: Final                //
//           Date: 07-03-2021              //
//                                         //
/////////////////////////////////////////////

// ESP8266 Libraries
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;

// Wifi Connection Variables
const char* ssid = "ControlSystem";    // WIFI SSID
const char* password = "12345678";     // WIFI Password

//Server Request Functions
const char* serverFanON = "http://192.168.4.1/fanON";
const char* serverFanOFF = "http://192.168.4.1/fanOFF";
const char* serverHeaterON = "http://192.168.4.1/heaterON";
const char* serverHeaterOFF = "http://192.168.4.1/heaterOFF";
const char* serverLockON = "http://192.168.4.1/lockON";
const char* serverLockOFF = "http://192.168.4.1/lockOFF";
const char* serverNameLight = "http://192.168.4.1/light";

// OLED Display Setup
#include <Wire.h>                     // I2C library
#include "SSD1306Wire.h"              // OLED display library          
SSD1306Wire display(0x3c, 4, 5);      // Set Display Address to talk on I2C Pins

//PIR Sensor Setup
int pirVal;
int pirPin = 12;                      // PIR PIN Number

//DHT Temperature Setup
#include "DHT.h"                     // DHT Library
#define DHTPIN 2                     // DHT Digtal Pin Number (Connect to the second pin from the left on the Sensor)
#define DHTTYPE DHT22                // Set DHT type to DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);            // Start DHT22 as object
float tempC;                         // Variable for holding current Room Temperature

//PN532 RFID Setup
#include <PN532.h>                   // Basic PN532 Library
#include <PN532_I2C.h>               // PN532 I2C Library
#include <NfcAdapter.h>              // NFC Library
PN532_I2C pn532(Wire);               // Set PN532 to run on I2C 
NfcAdapter nfc = NfcAdapter(pn532);  // Setup PN532 NFC Object

unsigned long delaytemp = 99999999999;      //Prevent Premature starting
unsigned long delaymotion = 99999999999;    //Prevent Premature starting
unsigned long delaynomotion = 99999999999;  //Prevent Premature starting

// Potentiometer Setup                    
const int analogInPin = A0;              // Pot Analog Pin Number
float sensorValue = 0;                   // Current Set Temperature by Potentiometer
float MIN = 18.0;                        // MIN value accessable by Thermostat 
float MAX = 30.0;                        // MAX value accessable by Thermostat 
float SetPoint = 0;                      // Current wanted SetPoint Temperature (Adjusted by Potentiometer)
int DeadBand = 2;                        // Deadband Temperature Oscillation Varaible

void setup() {
  Serial.begin(115200);                // Start Serial for Printing

  display.init();                      // Initialize OLED Display
  display.flipScreenVertically();      // Set Screen Orientation

  dht.begin();                                // Start DHT22 Object
  WiFi.mode(WIFI_STA);                        // Set up Wifi Client Communication
  WiFiMulti.addAP(ssid, password);            // Connect to Network Using Set Username and Password
  while((WiFiMulti.run() == WL_CONNECTED)) {  // While waiting to connect
    delay(500);
    Serial.print(".");                        //Print point to show pending
  }
  Serial.println("");
  Serial.println("Connected to WiFi");        // Print Connected when finished while loop
  
  Serial.println("Initializing Card Reader.......");
  delay(3000);
  nfc.begin();
}

void loop() {
  Serial.print(pirVal);
  if ((WiFiMulti.run() == WL_CONNECTED)) {   // If there is a currently active connection
    Motion();             // Call function - Read Current PIR Sensor for Motion
    Display();            // Call function - Read Current Temperature and print to Display
    tempTrigger();        // Call function - Poteniometer to Set Temperature Setpoint
    Fan_ON();             // Call function - Send Control Information to Other ESP
    cardScan();    
  } else {
      Serial.println("WiFi Disconnected");
 }
}

void tempTrigger() {
  sensorValue = analogRead(analogInPin);             // Set sensor variable to current potentiometer reading
  SetPoint = map(sensorValue, 0, 1023, MIN, MAX);    // Map Setpoint of current sensorValue to a value between selected MIN and MAX values
  if (((millis() - delaymotion) <= 15000 ) ) {  
    if (tempC >= (SetPoint + DeadBand / 2)) {        // Check if Temperature is between the desired setpoint with an allowable oscillation of the set deadband
       httpGETRequest(serverHeaterOFF);              //Send Request to Server to turn OFF Heater
   }
    if (tempC <= (SetPoint - DeadBand / 2)) {        // Check if Temperature is between the desired setpoint with an allowable oscillation of the set deadband
      tempC = dht.readTemperature();                 // Read Current Temperature from Temp Sensor 
      delaytemp = millis();                         // Set temp Timer Variable to current millis
      httpGETRequest(serverHeaterON);               //Send Request to Server to turn ON Heater
   }
  } else {
    httpGETRequest(serverHeaterOFF);               //Send Request to Server to turn OFF Heater
  }
}

void Motion(){
  pirVal = digitalRead(pirPin);    // Read the state of the PIR Sensor
  if ((pirVal == HIGH) ) {         // Motion is detected 
  delaymotion = millis();          // Set motion Timer Variable to current millis
  } 
  else {
  delaynomotion = millis();       // Set no motion Timer Variable to current millis
  }
}

void Fan_ON() {
  if (((millis() - delaytemp) <= 3000 ) && ((millis() - delaymotion) <= 15000 ) ) {  
        httpGETRequest(serverFanON);        //Send Request to Server to turn ON Fan
  }
  else { 
        httpGETRequest(serverFanOFF);        //Send Request to Server to turn ON Fan
  }
}

void Display() {
  /////////////// DHT 22 and OLED display ////////////////////////
  tempC = dht.readTemperature();                   // Read Current Temp in Degrees Celcius
  display.clear();                                 // Clear Display
  display.setTextAlignment(TEXT_ALIGN_LEFT);       // Set Text Allignment
  display.setFont(ArialMT_Plain_24);               // Adjust Font Size
  display.drawString(0, 0, String(tempC));         // Print the Temp to Screen
  display.drawString(59, 0, String((char)247));    // Print the Degrees Symbol
  display.drawString(63, 0, "C");                  // Print "C" to screen
  display.setFont(ArialMT_Plain_10);               // Adjust Font Size
  display.drawString(90, 0, "Set");                // Print "Set" to Screen
  display.drawString(86, 7, "point");              // Print "point" to screen
  display.setFont(ArialMT_Plain_16);               // Adjust Font Size
  display.setTextAlignment(TEXT_ALIGN_CENTER);     // Set Text Allignment
  display.drawString(98, 18, String(SetPoint));    // Write Current Setpoint to Display
  display.display();                               // Update Display
}

//HTTP GET Requests Function
String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  http.begin(client, serverName);     // Begins http connection to requested server 
  
  int httpResponseCode = http.GET();  // Send HTTP GET request
  
  String payload = "--"; 
  
  if (httpResponseCode>0) {                // If there is a response recieved from server
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);      // Print the HTTP response code (200 = Accepted Properly)
    payload = http.getString();            // Set payload to whatever what recieved from server.
  }
  else {                                   //Otherwise
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);      // Print off error code
  }

  http.end();                              //End Server Connection

  return payload;                          //Return Server Response
}

//RFID Reader Function
void cardScan() {
 
  if (nfc.tagPresent(5))                  //If card is detected at Reader with a Timeout of 5ms
  {
    NfcTag tag = nfc.read();              //Read Data off Card        
    Serial.println("New Scan Detected");
    Serial.print("UID: ");Serial.println(tag.getUidString()); // Print Card Data to Serial

    if (tag.getUidString() == "A2 C1 8C 5A")         // If Approved Card ID is detected
    {
         Serial.println("Access Granted"); 
         httpGETRequest(serverLockON);               // Send Request to Server to turn OFF Lock
         delay(500);
    }
   } else if ((millis() - delaymotion) >= 10000 ) { // If no motion is detect for 10 seconds
   httpGETRequest(serverLockOFF);                   //Send Request to Server to turn ON Lock
  }
}
