#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

void PinA();
void PinB();

LiquidCrystal_I2C lcd(0x27, 16, 2);

static int pinA = 2;
static int pinB = 3;
volatile byte aFlag = 0;
volatile byte bFlag = 0;
volatile byte encoderPos = 0;
volatile byte oldEncPos = 0;
volatile byte reading = 0;

void setup() {
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  attachInterrupt(0,PinA,RISING);
  attachInterrupt(1,PinB,RISING);
  Serial.begin(115200);

  lcd.init();
  lcd.clear();         
  lcd.backlight();
  
}

void PinA(){
  cli();
  reading = PIND & 0xC;
  if(reading == B00001100 && aFlag) {
    encoderPos --;
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
  if(oldEncPos != encoderPos) {
    Serial.println(encoderPos);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Encoder: ");
    lcd.print(encoderPos);

    oldEncPos = encoderPos;
  }
}


