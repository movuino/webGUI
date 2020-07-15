/* 
This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
It won't work with v1.x motor shields! Only for the v2's with built in PWM
control

For use with the Adafruit Motor Shield v2 
---->	http://www.adafruit.com/products/1438
*/
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Ticker.h>
#include <Adafruit_NeoPixel.h>

// Select which 'port' M1, M2, M3 or M4. In this case, M1
//Adafruit_DCMotor *myMotor = AFMS.getMotor(1);
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// You can also make another motor on port M2
//Adafruit_DCMotor *myOtherMotor = AFMS.getMotor(2);
int PWMA = 0;
int INA1=2;
int INA2=1;
int PWMB = 5;
int INB1=3;
int INB2=4;
bool commandAvailable =true;
Ticker timer1;
float commandDurationSec =0.4;
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
#define PIN           15//gpio12
//gpio15 OK
//gpio2 OK
//gpio14 OK
//gpio12 BAD :-(

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS     1

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


void setDCForward(int motorSpeed){
   pwm.setPWM(INA1 , 0, 4096);
   pwm.setPWM(INA2 , 4096, 0);
   pwm.setPWM (INB1 , 0, 4096);
  pwm.setPWM (INB2, 4096, 0);
  pwm.setPWM (PWMA, 2000, 0);
  pwm.setPWM (PWMB , 2000, 0);
  delay(10);
   Serial.println("DC Forward");
   pixels.setPixelColor(0, pixels.Color(0,125,0));
   pixels.show();
   timer1.attach(commandDurationSec, setDCStop);
}
void setDCBackward(int motorSpeed){
   pwm.setPin (INA1 , 0, 0);
   pwm.setPin (INA2 , 4095, 0);
   pwm.setPin (INB1 , 0, 0);
   pwm.setPin (INB2, 4095, 0);
   pwm.setPin (PWMA ,motorSpeed, 0);
   pwm.setPin (PWMB , motorSpeed, 0);
   pixels.setPixelColor(0, pixels.Color(125,0,0));
   pixels.show();
   Serial.println("DC Backward");
   timer1.attach(commandDurationSec, setDCStop);
}
void setDCLeft(int motorSpeed){
   pwm.setPin (INA1 , 4095, 0);
   pwm.setPin (INA2 , 0, 0);
   pwm.setPin (INB1 , 0, 0);
   pwm.setPin (INB2, 4095, 0);
   pwm.setPin (PWMA ,motorSpeed, 0);
   pwm.setPin (PWMB , motorSpeed, 0);
   pixels.setPixelColor(0, pixels.Color(0,0,125));
   pixels.show();
   Serial.println("DC Left");
   timer1.attach(commandDurationSec, setDCStop);
}
void setDCRight(int motorSpeed){
   pwm.setPin (INA1 , 0, 0);
   pwm.setPin (INA2 , 4095, 0);
   pwm.setPin (INB1 , 4095, 0);
   pwm.setPin (INB2, 0, 0);
   pwm.setPin (PWMA ,motorSpeed, 0);
   pwm.setPin (PWMB , motorSpeed, 0);
   pixels.setPixelColor(0, pixels.Color(125,125,0));
   pixels.show();
   Serial.println("DC Right");
   timer1.attach(commandDurationSec, setDCStop);
}
void setDCStop(){
    delay(10);
    pwm.setPWM(INA1, 0, 4096);
    delay(1);
    pwm.setPWM(INA2 , 0, 4096);
     delay(1);
     pwm.setPWM (PWMA , 4096, 0);
      delay(1);
   pwm.setPWM (INB1 , 0, 4096);
    delay(1);
    pwm.setPWM (INB2, 0, 4096);
     delay(1);
    pwm.setPWM (PWMB , 4096, 0);
   delay(100);
   pixels.setPixelColor(0, pixels.Color(0,0,0));
   pixels.show();
   Serial.println("stop");
   if(!commandAvailable)  {
      timer1.detach();
      commandAvailable=true;
   }
}


void setup() {
  Serial.begin(115200);           // set up Serial library at 9600 bps
  Serial.println("Adafruit Motorshield v2 - DC Motor test!");
  pwm.begin();
  pwm.setPWMFreq(1000); 
  setDCStop();
  pixels.begin(); // This initializes the NeoPixel library.
  delay(2);
  pixels.setPixelColor(0, pixels.Color(125,0,0));
  pixels.show();
  delay(2);
  pixels.setPixelColor(0, pixels.Color(0,0,0));
  pixels.show();
  delay(2);
   setDCStop();
   delay(500);
}

void loop() {
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    //Ping
    if (inByte == '?') {
      Serial.println("?");
    }
    else if (inByte == 's') {
      Serial.println("s");
     // if(commandAvailable) {
     //   commandAvailable=false;
        setDCStop();
     //   }
      }
    else if (inByte == 'f') {
      Serial.println("f");
      if(commandAvailable) {
        commandAvailable=false;
        setDCForward(2000);
        }
      else Serial.println("command discarded busy");
    }
     else if (inByte == 'b') {
      Serial.println("b");
      if(commandAvailable) {
        setDCBackward(4096);
        commandAvailable=false;
      }
      else Serial.println("command discarded busy");
    }
    else if (inByte == 'l') {
      Serial.println("l");
      if(commandAvailable) {
        setDCLeft(4096);
        commandAvailable=false;
      }
      else Serial.println("command discarded busy");
    }
    else if (inByte == 'r') {
      Serial.println("r");
      if(commandAvailable) {
        setDCRight(4096);
        commandAvailable=false;
        }
      else Serial.println("command discarded busy");
    }
  }
  /*setDCForward(2047);
  delay(500);
  setDCStop();
  delay(500);*/
  }
