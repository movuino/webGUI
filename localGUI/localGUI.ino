/*
 * ESP8266 NodeMCU Real Time Graphs Demo
 * Updates and Gets data from webpage without page refresh
 * https://circuits4you.com
 */
#include "FS.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>

#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include <Ticker.h>  //Ticker Library


#define LED 2  //On board LED


/* Set these to your desired credentials. */
const char *ssid = "Movuino-001";
const char *password = "";

MPU6050 accelgyro(0x69);

int16_t ax, ay, az; 
int16_t gx, gy, gz;

Ticker ticker1;


ESP8266WebServer server(80); //Server on port 80
WebSocketsServer webSocket = WebSocketsServer(81);

const char command[] = "requestIMU";

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
// lorsque le ticker est ativé, envoie les valeurs de l'accéléromètre au websocket et en serial
void onTick() {
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  webSocket.sendTXT(0, "accéléromètre: "+(String)ax+" "+(String)ay+" "+(String)az); // ne fonctionne pas dans WebSocketEvent
    Serial.print(ax); 
    Serial.print(" ");
    Serial.print(ay);
    Serial.print(" ");
    Serial.println(az);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // payload = contenu du message envoyé
        // Send the current LED status
    /*    if (movSTATUS) {
          Serial.println("movSTATUS");
          webSocket.sendTXT(num, avance, strlen(avance));
        }
        else {
          webSocket.sendTXT(num, halte, strlen(halte));
          Serial.println("movSTATUS else");
        }  */
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\r\n", num, payload);
      //received request of datza
      if (strcmp(command, (const char *)payload) == 0) {
        //webSocket.sendBIN(num, payload, length);
      }
      /*
      else if (strcmp(halte, (const char *)payload) == 0) {
      //  writeLED(false);     
         S();  
      }
      else if (strcmp(droite, (const char *)payload) == 0) {  
         D();  
      }      
      else if (strcmp(gauche, (const char *)payload) == 0) {
        G();
      }
      else if (strcmp(recule, (const char *)payload) == 0) {  
         R();  
      }      
      else {
        Serial.println("Unknown command");
      }*/
      // send data to all connected clients
      //webSocket.broadcastTXT(payload, length);  // envoie le message reçu à tous les appareils connectés
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      hexdump(payload, length);

      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
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
/*
void handleADC() {
 int a = analogRead(A0);
 String adcValue = String(a);
 digitalWrite(LED,!digitalRead(LED)); //Toggle LED on data request ajax
 server.send(200, "text/plane", adcValue); //Send ADC value only to client ajax request
} */
//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  Serial.begin(115200);

    // join I2C bus (I2Cdev library doesn't do this automatically)
    Wire.begin();

    // initialize device
    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
    
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
  
  if (!MDNS.begin("Movuino-001")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  MDNS.addService("ws", "tcp", 81);
  MDNS.addService("http", "tcp", 80);
  
  Serial.println("mDNS responder started");
 // IPAddress myIP = WiFi.softAPIP();
  //Serial.print("AP IP address: ");
  //Serial.println(myIP);
  //server.on("/", handleRoot);
//  server.on("/readADC", handleADC); //This page is called by java Script AJAX
  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/", SPIFFS, "/index.html");
  
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("HTTP server started");

   ticker1.attach_ms(50, onTick); // le ticker s'active toutes les secondes et exécute onTick
 
}
//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  webSocket.loop();
  server.handleClient();          //Handle client requests
}
