#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void PinA();
void PinB();

static int pinA = 2;
static int pinB = 3;
static int buttonPin = 4;
volatile int encoderPos = 0;
volatile int oldEncPos = 0;
volatile byte aFlag = 0;
volatile byte bFlag = 0;
volatile byte reading = 0;
volatile bool recording = true;
volatile bool showWheelSpecs = false;
volatile bool wheelScaled = false;

void setup() {
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(0,PinA,RISING);
  attachInterrupt(1,PinB,RISING);

  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("RECORD");

  Serial.begin(115200);
}

void PinA(){
  cli();
  reading = PIND & 0xC;
  if(reading == B00001100 && aFlag) {
    if(encoderPos > 0) {
      encoderPos --;
    }
    bFlag = 0;
    aFlag = 0;
  }
  else if (reading == B00000100) bFlag = 1;
  sei();
}

void PinB(){
  cli();
  reading = PIND & 0xC;
  if (reading == B00001100 && bFlag) {
    encoderPos ++;
    bFlag = 0;
    aFlag = 0;
  }
  else if (reading == B00001000) aFlag = 1;
  sei();
}

void loop(){
  if(digitalRead(buttonPin) == LOW) {
    delay(20); // debounce
    if(digitalRead(buttonPin) == LOW) {
      if(recording && encoderPos > 0) { // Check if any rotations have been recorded
        recording = false;
        wheelScaled = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Recording Done");
        delay(1000);
      } else if(wheelScaled) {
        showWheelSpecs = !showWheelSpecs;
      }
      while(digitalRead(buttonPin) == LOW); // wait for button release
    }
  }

  if(!recording && oldEncPos != encoderPos && wheelScaled) {
    lcd.clear();
    lcd.setCursor(0, 0);
    if(showWheelSpecs) {
      float circumference = 1.0 / encoderPos; // 1 meter divided by number of rotations
      float diameter = circumference / PI;
      lcd.print("Diameter: ");
      lcd.print(diameter);
      lcd.print(" m");
      lcd.setCursor(0, 1);
      lcd.print("Circumference: ");
      lcd.print(circumference);
      lcd.print(" m");
    } else {
      lcd.print(encoderPos);
      lcd.setCursor(0, 1);
      lcd.print("Dist: " + String(encoderPos * PI));
      lcd.setCursor(14, 1);
      lcd.print("m");
    }
    oldEncPos = encoderPos;
  }
}
