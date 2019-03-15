//Include Libraries.
#include <LiquidCrystal.h>
#include <Adafruit_MotorShield.h>
#include <Wire.h>

//Declare Variables.
volatile boolean encoderFired;
volatile boolean dir;
volatile bool pushButton;
volatile bool lastPushButton;
volatile boolean pushed;
volatile long period;
volatile long newRpm;
volatile boolean forward = false;
volatile boolean go = false;
volatile uint32_t start;
volatile uint32_t overallStart;
volatile uint32_t timeLeft;
volatile uint32_t timeToRun;
volatile int timerHour;
volatile int timerMin;
volatile boolean settingHour = true;
volatile boolean settingMin = false;
volatile boolean ended = true;
const int rs = 7, en = 8, d4 = 9, d5 = 10, d6 = 11, d7 = 12;
#define pushbutton 3
#define encoderPinA 2
#define encoderPinB 4
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void encoderisr(){ //Controls input from encoder.
  if (digitalRead(encoderPinA)){
    delay(20);
    dir = digitalRead(encoderPinB);
  }
  else{
    delay(20);
    dir = !digitalRead(encoderPinB);
  }
  encoderFired = true;
}

void pushbuttonisr(){ //Controls input from button.
  pushButton = digitalRead(pushbutton);
  if (pushButton != lastPushButton){
    if (pushButton == false){
      pushed = true;
    }
    delay(50);
  }
  lastPushButton = pushButton;
}



void setup() { //Setup Code. Runs once.
  pinMode(pushbutton, INPUT_PULLUP);
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  Serial.begin(9600);
  AFMS.begin();
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderisr, RISING);
  attachInterrupt(digitalPinToInterrupt(pushbutton), pushbuttonisr, CHANGE);
  timerHour = 0;
  timerMin = 0;
  lcd.begin(20,4);
  lcd.setCursor(0,0);
  lcd.print("Set Time to Run:");
  lcd.setCursor(0,1);
  lcd.print("-> " + String(timerHour) + " hrs <- ");
  lcd.setCursor(0,2);
  lcd.print("   " + String(timerMin) + "mins  ");
}

void loop() { //Main code. Runs in an infinite loop.
  if(encoderFired){
    if (settingHour){
      if(dir){
        timerHour++;
      }
      else{
        if (timerHour == 0){
          timerHour = 0;
        }
        else{
          timerHour--;
        }
      }
    }
    if (settingMin){
      if(dir){
        timerMin++;
      }
      else{
        if (timerMin == 0){
          timerMin = 0;
        }
        else{
          timerMin--;
        }
      }
    }
    lcd.clear();
    encoderFired = false;
  }

  if(pushed){
    if (settingHour == true && settingMin == false){
      settingHour = false;
      settingMin = true;
    }
    else if(settingMin == true && settingHour == false){
      settingMin = false;
    }
    else{
      overallStart = millis();
      start = millis();
      go = !go;
      settingHour= true;
      settingMin = false;
      timeToRun = (timerHour * 60 * 60000) + (timerMin * 60000);
      period = random(30, 61) * 1000;
      newRpm = random(3,7);
      myMotor->setSpeed(newRpm);
      forward = !forward;
    }
    lcd.clear();
    pushed = false;
  }
  if (go){
    if (millis() - overallStart <= timeToRun){
      if (millis() - start <= period){
        if(forward){
          myMotor->step(100, FORWARD, SINGLE);
        }
        else{
          myMotor->step(100, BACKWARD, SINGLE);
        }
      }
      else{
        start = millis();
        period = random(30, 61) * 1000;
        newRpm = random(3, 7);
        myMotor->setSpeed(newRpm);
        forward = !forward;
      }
    }
    else{
      go = false;
      settingHour = true;
      settingMin = false;
      timerHour = 0;
      timerMin = 0;
      ended = true;
    }
    if (!ended){
      timeLeft = timeToRun - millis() + overallStart;
      lcd.setCursor(0,0);
      lcd.print("                    ");
      lcd.setCursor(0,0);
      lcd.print("Time Left: " + String((timeLeft / 1000) / 60) + " mins");
      lcd.setCursor(0,1);
      lcd.print("Current RPM: " + String(newRpm));
      lcd.setCursor(0,2);
      if (forward){
        lcd.print("->");
      }
      else{
        lcd.print("<-");
      }
    }
    else{
      lcd.clear();
      ended = false;
    }
  }
  else{
    lcd.setCursor(0,0);
    lcd.print("Set Time to Run:");
    if(settingHour){
      lcd.setCursor(0,1);
      lcd.print("-> " + String(timerHour) + " hrs <- ");
      lcd.setCursor(0,2);
      lcd.print("   " + String(timerMin) + " mins");
    }
    else{
      lcd.setCursor(0,1);
      lcd.print("   " + String(timerHour) + " hrs");
      lcd.setCursor(0,2);
      lcd.print("-> " + String(timerMin) + " mins <-");
    }
  }
}
