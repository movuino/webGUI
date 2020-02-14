#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WebSocketsServer.h>
#include <Ticker.h>
#include<Wire.h>
#include "FS.h"
#include "SPIFFS.h"
#define FORMAT_SPIFFS_IF_FAILED false
#define VERBOSE //comment for removing printf

Ticker ticker1;

const int MPU_addr=0x69; // I2C address of the MPU-6050
//int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
int16_t ax, ay, az; 
int bpm = 60;
const char* ssid = "MovuinoESP32";
const char* password = "........";

const char command[] = "requestIMU";

WebServer server(80);

const int led = 13;
WebSocketsServer webSocket = WebSocketsServer(81);


void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
  const uint8_t* src = (const uint8_t*) mem;
  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for(uint32_t i = 0; i < len; i++) {
    if(i % cols == 0) {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    Serial.printf("%02X ", *src);
    src++;
  }
  Serial.printf("\n");
}


void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void connect_wifi(){
    Serial.println();
  Serial.println("Configuring access point...");
  WiFi.mode(WIFI_AP);
  delay(100);
  Serial.println("Set softAPConfig");
  IPAddress Ip(10, 10, 10, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  delay(100);
}
void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(led, 0);
}
void onTick() {
//  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
 Wire.beginTransmission(MPU_addr);
 Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
 Wire.endTransmission(false);
 Wire.requestFrom(MPU_addr,14,true); // request a total of 14 registers
 ax=Wire.read()<<8|Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
 ay=Wire.read()<<8|Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
 az=Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
 bpm=random(60,88);
 webSocket.sendTXT(0, "accéléromètre: "+(String)bpm+" "+(String)ay+" "+(String)az);
// webSocket.sendTXT(0, "bpm: "+(String)bpm); // ne fonctionne pas dans WebSocketEvent
  #ifdef VERBOSE
      Serial.println(bpm); 
      /*Serial.print(ax); 
      Serial.print(" ");
      Serial.print(ay);
      Serial.print(" ");
      Serial.println(az);*/
  #endif
   
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
    {
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
    }
    break;
    case WStype_BIN:
    {
     Serial.printf("[%u] get binary length: %u\r\n", num, length);
      /*#ifdef VERBOSE
        Serial.printf("[%u] get binary length: %u\r\n", num, length);
        Serial.println(" ");
        //hexdump(payload, length);
        Serial.println(length);
        Serial.println(payload[0]);
        Serial.println(payload[1]);
        Serial.println(payload[2]);
      #endif*/
      int outputType = payload[0];
      int outputId = payload[1];
      int outputValue = payload[2];      
      //motor frame 
      if(outputType==0){
         if(outputId==0){
           Serial.print("Left motor ");
           Serial.print(outputValue);
           Serial.println(" %");
         }
         else if(outputId==1){
           Serial.print("Right motor ");
           Serial.print(outputValue);
           Serial.println(" %");
         }
      }
      // echo data back to browser
      //webSocket.sendBIN(num, payload, length);
   }
      break;
   default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
   break;
  }
}
void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  randomSeed(analogRead(0));
 // pinMode(led, OUTPUT);
 // digitalWrite(led, 0);
  /*MPU6050 init*/
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true); 
  /**/
  Serial.begin(115200);
  connect_wifi();
  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/", SPIFFS, "/index.html");
  
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
 // ticker1.attach_ms(50, onTick); // le ticker s'active toutes les secondes et exécute onTick

}

void loop(void) {
   //listDir(SPIFFS, "/", 0);
   //delay(200);
  webSocket.loop();
  server.handleClient();
}
