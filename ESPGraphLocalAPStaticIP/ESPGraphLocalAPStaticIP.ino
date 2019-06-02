/*
 * ESP8266 NodeMCU Real Time Graphs Demo
 * Updates and Gets data from webpage without page refresh
 * https://circuits4you.com
 */
#include "FS.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#define LED 2  //On board LED


/* Set these to your desired credentials. */
const char *ssid = "Movuino-001";
const char *password = "";


ESP8266WebServer server(80); //Server on port 80
void list_files(){
    Serial.print("Listing files");
    Dir dir = SPIFFS.openDir("/js/");
    while (dir.next()) {
       Serial.print(dir.fileName());
       File f = dir.openFile("r");
       Serial.println(f.size());
      }
    /*Dir dir = SPIFFS.openDir("/data");
    while (dir.next()) {
       Serial.print(dir.fileName());
       File f = dir.openFile("r");
       Serial.println(f.size());
      }*/
}
//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
/*void handleRoot() {
 //String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}*/
void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected</h1>");
}

void handleADC() {
 int a = analogRead(A0);
 String adcValue = String(a);
 digitalWrite(LED,!digitalRead(LED)); //Toggle LED on data request ajax
 server.send(200, "text/plane", adcValue); //Send ADC value only to client ajax request
}
//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  Serial.begin(115200);
  bool result = SPIFFS.begin();
  Serial.println(result);
  list_files();
  WiFi.mode(WIFI_AP);  //need both to serve the webpage and take commands via tcp
  IPAddress ip(10,10,10,1);
  IPAddress gateway(1,2,3,1);
  IPAddress subnet(255,255,255,0);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(ssid); //Access point is open
  delay(1000);
  /*IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);*/
  //WiFi.softAP(ssid, password);   //Connect to your WiFi router
  Serial.println("");

  //Onboard LED port Direction output
  pinMode(LED,OUTPUT); 
  
 // IPAddress myIP = WiFi.softAPIP();
  //Serial.print("AP IP address: ");
  //Serial.println(myIP);
  //server.on("/", handleRoot);
  server.on("/readADC", handleADC); //This page is called by java Script AJAX
  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/", SPIFFS, "/index.html");
  server.begin();
  Serial.println("HTTP server started");
 
}
//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  server.handleClient();          //Handle client requests
}
