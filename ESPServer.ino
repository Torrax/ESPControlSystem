/////////////////////////////////////////////
//                                         //
//         Authors: Salim El Atache        //
//             & Carson Morris             //
//         Class: CAM8302E - 012           //
//                 Lab: 5                  //
//           Date: 07-03-2021              //
//                                         //
/////////////////////////////////////////////

// Access Point Setup
#include <ESP8266WiFi.h>
const char WiFiPassword[] = "12345678";
const char AP_NameChar[] = "ControlSystem" ;
 
WiFiServer server(80);

// Webpage HTML Headings
String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
String html_1 = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/><meta charset='utf-8'><style>body {font-size:140%;} #main {display: table; margin: auto;  padding: 0 10px 0 10px; } h2,{text-align:center; } .button { padding:10px 10px 10px 10px; width:100%;  background-color: #4CAF50; font-size: 120%;}</style><title>Control System</title></head><body><div id='main'><h2>Control System</h2>";
String html_2 = "";
String html_3 = "";
String html_4 = "";
String html_5 = "</div></body></html>";
 
String request = "";

//Output Variables//
#define fanRelay 12            // Fan Relay Digital Pin Number
#define heatRelay 13           // Heat Relay Digital Pin Number
#define lockRelay 14           // Lock Relay Digital Pin Number
 
void setup() 
{
  // Outputs //
  pinMode(fanRelay, OUTPUT);         // Delclare fanRelay as an Output
  digitalWrite(fanRelay, HIGH);      // Start fanRelay OFF
  pinMode(heatRelay, OUTPUT);        // Delclare heatRelay as an Output
  digitalWrite(heatRelay, HIGH);     // Start heatRelay OFF
  pinMode(lockRelay, OUTPUT);        // Delclare lockRelay as an Output
  digitalWrite(lockRelay, HIGH);     // Start lockRelay OFF
 
    boolean conn = WiFi.softAP(AP_NameChar, WiFiPassword);
    server.begin();
 
}
 
 
 
void loop() 
{
    WiFiClient client = server.available();         // If Client is connected
    if (!client)  {  return;  }
 
    // Read the first line of the request
    request = client.readStringUntil('\r');        //Get Request From Client
 
    if       ( request.indexOf("fanON") > 0 )  {  //If Request Recieved "fanON"
      digitalWrite(fanRelay, LOW);
   } else if  ( request.indexOf("fanOFF") > 0 ) {  //If Request Recieved "fanOFF"
        digitalWrite(fanRelay, HIGH);
   } else if  ( request.indexOf("heaterON") > 0 ) {  //If Request Recieved "heaterON"
    digitalWrite(heatRelay, LOW);
    } else if  ( request.indexOf("heaterOFF") > 0 ) {  //If Request Recieved "fanOFF"
      digitalWrite(heatRelay, HIGH);   
   } else if  ( request.indexOf("lockON") > 0 ) {  //If Request Recieved "lockON"
    digitalWrite(lockRelay, LOW);
    } else if  ( request.indexOf("lockOFF") > 0 ) {  //If Request Recieved "lockOFF"
      digitalWrite(lockRelay, HIGH);   
    } else{
    }
    
    // Print HTML Buttons to Website
    if (digitalRead(fanRelay) == LOW){                                                                                         //If Fan Relay is Currently ON
       html_2 = "<form id='F1' action='fanOFF'><input class='button' type='submit' value='Turn OFF Fan' ></form><br>";         //Create Button for Turning OFF
    } else {                                                                                                                   //Otherwise
        html_2 = "<form id='F1' action='fanON'><input class='button' type='submit' value='Turn ON Fan' ></form><br>";          //Create Button for Turning ON
    }
    
    if (digitalRead(heatRelay) == LOW) {                                                                                      //If Heat Relay is Currently ON
       html_3 = "<form id='F2' action='heaterOFF'><input class='button' type='submit' value='Turn OFF Heater' ></form><br>";  //Create Button for Turning OFF
    } else {                                                                                                                  //Otherwise
        html_3 = "<form id='F2' action='heaterON'><input class='button' type='submit' value='Turn ON Heater' ></form><br>";   //Create Button for Turning ON
    }

    if (digitalRead(lockRelay) == LOW) {                                                                                     //If Lock Relay is Currently ON
       html_4 = "<form id='F3' action='lockOFF'><input class='button' type='submit' value='Turn OFF Lock' ></form><br>";      //Create Button for Turning OFF
    } else {                                                                                                                  //Otherwise
        html_4 = "<form id='F3' action='lockON'><input class='button' type='submit' value='Turn ON Lock' ></form><br>";       //Create Button for Turning ON
    }

    client.flush();
    
    // Print HTML Elements to Webpage
    client.print( header );
    client.print( html_1 );    
    client.print( html_2 );
    client.print( html_3 );
    client.print( html_4);
    client.print( html_5 );
 
    delay(5); 
}
