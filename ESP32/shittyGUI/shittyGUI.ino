#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WebSocketsServer.h>
#include<Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "FS.h"
#include "SPIFFS.h"
#define FORMAT_SPIFFS_IF_FAILED false
#define VERBOSE //comment for removing printf
#define SERVOMIN  150 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // This is the 'maximum' pulse length count (out of 4096)
#define USMIN  600 // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX  2400 // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50 
/*
 fully on
  pwm.setPWM(pin, 4096, 0);
 void  setPin (uint8_t num, uint16_t val, bool invert=false)
You can set the pin to be fully off with
pwm.setPWM(pin, 0, 4096);

treuth table : 

L L H : offf
LH h ou Hl H : avancer ou reculer
HH H/L : brake
 */

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
const int MPU_addr=0x69; // I2C address of the MPU-6050
//int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
int16_t ax, ay, az; 
int bpm = 60;
const char* ssid = "MovuinoESP32-01";
const char* password = "........";

const char command[] = "requestIMU";

WebServer server(80);
int PWMA = 0;
int INA1=2;
int INA2=1;
int PWMB = 5;
int INB1=3;
int INB2=4;
int commandDurationMs=200;
bool myTimer0Attached=false;
unsigned long myTimer0;


void setDCForward(int motorSpeed){
   pwm.setPin (INA1 , 4096, 0);
   pwm.setPin (INA2 , 0, 0);
   pwm.setPin (INB1 , 4096, 0);
   pwm.setPin (INB2, 0, 0);
   pwm.setPin (PWMA ,motorSpeed, 0);
   pwm.setPin (PWMB , motorSpeed, 0);
   myTimer0=millis();
   myTimer0Attached=true;
   Serial.println("DC Forward");
}
void setDCBackward(int motorSpeed){
   pwm.setPin (INA1 , 0, 0);
   pwm.setPin (INA2 , 4095, 0);
   pwm.setPin (INB1 , 0, 0);
   pwm.setPin (INB2, 4095, 0);
   pwm.setPin (PWMA ,motorSpeed, 0);
   pwm.setPin (PWMB , motorSpeed, 0);
   myTimer0=millis();
   myTimer0Attached=true;
   Serial.println("DC Backward");
}
void setDCLeft(int motorSpeed){
   pwm.setPin (INA1 , 4095, 0);
   pwm.setPin (INA2 , 0, 0);
   pwm.setPin (INB1 , 0, 0);
   pwm.setPin (INB2, 4095, 0);
   pwm.setPin (PWMA ,motorSpeed, 0);
   pwm.setPin (PWMB , motorSpeed, 0);
   myTimer0=millis();
   myTimer0Attached=true;
   Serial.println("DC Left");
}
void setDCRight(int motorSpeed){
   pwm.setPin (INA1 , 0, 0);
   pwm.setPin (INA2 , 4095, 0);
   pwm.setPin (INB1 , 4095, 0);
   pwm.setPin (INB2, 0, 0);
   pwm.setPin (PWMA ,motorSpeed, 0);
   pwm.setPin (PWMB , motorSpeed, 0);
   myTimer0=millis();
   myTimer0Attached=true;
   Serial.println("DC Right");
}
void setDCStop(){
   pwm.setPin (INA1 , 0, 0);
   pwm.setPin (INA2 , 0, 0);
   pwm.setPin (INB1 , 0, 0);
   pwm.setPin (INB2, 0, 0);
   pwm.setPin (PWMA , 4095, 0);
   pwm.setPin (PWMB , 4095, 0);
   Serial.println("stop");
   delay(5);
   //setDCBackward(2047);
}




const int led = 13;
WebSocketsServer webSocket = WebSocketsServer(81);

void checkTimer(){
  if(millis()-myTimer0>=commandDurationMs){
    setDCStop();
    myTimer0Attached=false;
  }
  
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
  WiFi.softAP(ssid);
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
      int frameType = payload[0];
      int command = payload[1];
      int speedValue = payload[2];
      //int outputValue = payload[3];      
      //0: frametype (0 command frame avant arrière stop
      //1: command 
      //2 : speed
         
      if(frameType==0){
        //0 : stop
        if(command==0){
          Serial.println("stop");
          setDCStop();
        }
        //1 : forward
        else if(command==1){
          Serial.print("forward : ");
          Serial.println(speedValue);
          Serial.print(" ");
          int motorSpeed=map(speedValue,0,100,0,4095);
          setDCForward(motorSpeed);
          Serial.println(motorSpeed);
       
        }
        //2 : backward
        else if(command==2){
          Serial.println("backward : ");
          Serial.print(speedValue);
          Serial.print(" ");
          int motorSpeed=map(speedValue,0,100,0,4095);
          setDCBackward(motorSpeed);
          Serial.println(motorSpeed);
       
        }
        //3 : left
        else if(command==3){
          Serial.print("left : ");
          Serial.print(speedValue);
          int motorSpeed=map(speedValue,0,100,0,4095);
          setDCLeft(motorSpeed);
          Serial.println(motorSpeed);
         
        }
        //4 : right
        else if(command==4){
          Serial.print("right : ");
          Serial.print(speedValue);
          Serial.print(" ");
          int motorSpeed=map(speedValue,0,100,0,4095);
          setDCRight(motorSpeed);
          Serial.println(motorSpeed);
       
        }
        
      }
      //onoff
      else if (frameType==1){
        //command 
        if(speedValue==255) pwm.setPin (command ,4096, 0);
        else  pwm.setPin (command ,0, 0);
      }
      //servos
      else if (frameType==2){
        //command 
        int mappedValue= map(speedValue,0,180,150,600);
        pwm.setPWM(command, 0,mappedValue);
      }
      //motor frame 
      /*if(outputType==0){
         if(outputId==0){
           Serial.print("Left motor ");
           Serial.print(outputValue);
           Serial.print(" % ");
           if (outputDirection) Serial.println("Forward");
           else Serial.println("Reverse");
         }
         else if(outputId==1){
           Serial.print("Right motor ");
           Serial.print(outputValue);
           Serial.println(" %");
           if (outputDirection) Serial.println("Forward");
           else Serial.println("Reverse");
         }
      }*/
      // echo data back to browser
      //webSocket.sendBIN(num, payload, length);
   }
      break;
      
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      //hexdump(payload, length);
      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
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
  pwm.begin();
  // In theory the internal oscillator is 25MHz but it really isn't
  // that precise. You can 'calibrate' by tweaking this number till
  // you get the frequency you're expecting!
 // pwm.setOscillatorFrequency(27000000);  // The int.osc. is closer to 27MHz  
  pwm.setPWMFreq(1000);
  pwm.setPin (0, 0, 0);
  setDCStop();
  
  /**/
  Serial.begin(115200);
  connect_wifi();
  /*WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
*/
 /* server.on("/", handleRoot);

  server.on("/inline", []() {
U    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");*/
  server.serveStatic("/js", SPIFFS, "/js");
   server.serveStatic("/css", SPIFFS, "/css");
     server.serveStatic("/img", SPIFFS, "/img");
        server.serveStatic("/webfonts", SPIFFS, "/webfonts");
                server.serveStatic("/svgs", SPIFFS, "/svgs");
  server.serveStatic("/", SPIFFS, "/index.html");
  
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
 // ticker1.attach_ms(50, onTick); // le ticker s'active toutes les secondes et exécute onTick
 setDCForward(2047);
}

void loop(void) {
   //listDir(SPIFFS, "/", 0);
   //delay(200);
 
  // ticker1.attach_ms(500,setDCStop);
  //delay(100);
  //setDCStop();
 // delay(500);
 if(myTimer0Attached) checkTimer();
 webSocket.loop();
 server.handleClient();
}
