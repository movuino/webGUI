/* 
This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
It won't work with v1.x motor shields! Only for the v2's with built in PWM
control

For use with the Adafruit Motor Shield v2 
---->	http://www.adafruit.com/products/1438
*/

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <Adafruit_PWMServoDriver.h>


// Create the motor shield object with the default I2C address
//Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Select which 'port' M1, M2, M3 or M4. In this case, M1
//Adafruit_DCMotor *myMotor = AFMS.getMotor(1);
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// You can also make another motor on port M2
//Adafruit_DCMotor *myOtherMotor = AFMS.getMotor(2);

void setup() {
  Serial.begin(115200);           // set up Serial library at 9600 bps
  Serial.println("Adafruit Motorshield v2 - DC Motor test!");
 pwm.begin();
  pwm.setPWMFreq(1000); 
  //AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz
  
  // Set the speed to start, from 0 (off) to 255 (max speed)
 /* myMotor->setSpeed(150);
  myMotor->run(FORWARD);
  // turn on motor
  myMotor->run(RELEASE);*/
  pwm.setPWM(1,4096, 0);
  pwm.setPWM(2, 0, 0);
  pwm.setPWM(0, 2000, 0);
  
}

void loop() {
  pwm.setPWM(0, 2000, 0);
  pwm.setPWM(2, 0, 0);
  pwm.setPWM(1,4096, 0);
  delay(500);
  pwm.setPWM(0, 0, 0);
  pwm.setPWM(1,0, 0);
  pwm.setPWM(2, 0, 0);
  delay(500);
  pwm.setPWM(0, 2000, 0);
  pwm.setPWM(1,0, 0);
  pwm.setPWM(2, 4096, 0);
  delay(500);
  pwm.setPWM(0, 0, 0);
  pwm.setPWM(1,0, 0);
  pwm.setPWM(2, 0, 0);
  delay(500);
  /*uint8_t i;
  
  Serial.print("tick");

  myMotor->run(FORWARD);
  for (i=0; i<255; i++) {
    myMotor->setSpeed(i);  
    delay(10);
  }
  for (i=255; i!=0; i--) {
    myMotor->setSpeed(i);  
    delay(10);
  }
  
  Serial.print("tock");

  myMotor->run(BACKWARD);
  for (i=0; i<255; i++) {
    myMotor->setSpeed(i);  
    delay(10);
  }
  for (i=255; i!=0; i--) {
    myMotor->setSpeed(i);  
    delay(10);
  }

  Serial.print("tech");
  myMotor->run(RELEASE);
  delay(100);*/
  
}
