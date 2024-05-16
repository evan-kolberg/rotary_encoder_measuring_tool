#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

void aFunction();
void bFunction();

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define PIN_A 2
#define PIN_B 3
#define BUTTON_PIN 4
volatile byte aFlag = 0;
volatile byte bFlag = 0;
volatile int encoderPos = 0;
volatile int oldEncPos = 0;
volatile byte reading = 0;
volatile int buttonValue = 0;
volatile float circum;
volatile float diam;
volatile unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 100;
volatile bool recordingPrinted = false;
volatile bool calculated = false;
volatile bool menuChange = false;
volatile bool buttonPressed = false;
volatile unsigned long buttonPressedTime = 0;
volatile bool buttonLongPressed = false;

void setup() {
  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(0, aFunction, RISING);
  attachInterrupt(1, bFunction, RISING);

  // Enable PCIE2 Bit3 = 1 (Port D)
  PCICR |= B00000100;
  // Select PCINT20 Bit4 = 1 (Pin D4)
  PCMSK2 |= B00010000;

  lcd.init();
  lcd.clear();
  lcd.backlight();

  lcd.print("RECORD");
}

void aFunction() {
  reading = PIND & 0xC;
  if (reading == B00001100 && aFlag) {
    if (encoderPos > 0) {
      encoderPos--;
    }
    bFlag = 0;
    aFlag = 0;
  } else if (reading == B00000100)
    bFlag = 1;
}

void bFunction() {
  reading = PIND & 0xC;
  if (reading == B00001100 && bFlag) {
    encoderPos++;
    bFlag = 0;
    aFlag = 0;
  } else if (reading == B00001000)
    aFlag = 1;
}

ISR(PCINT2_vect) {
  unsigned long currentTime = millis();

  if ((currentTime - lastDebounceTime) > debounceDelay) {
    if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
      buttonPressed = true;
      buttonPressedTime = currentTime;
    }
    lastDebounceTime = currentTime;
  }

  if (digitalRead(BUTTON_PIN) == HIGH && buttonPressed) {
    unsigned long holdingTime = currentTime - buttonPressedTime;
    if (holdingTime >= 3000) {
      encoderPos = 0;
    } else {
      buttonValue++;
      if (buttonValue >= 3) {
        menuChange = true;
      }
    }
    buttonPressed = false;
  }
}

void loop() {
  if ((buttonValue == 1) && (!recordingPrinted)) {
    encoderPos = 0;
    lcd.print("ING...");
    recordingPrinted = true;

  } else if ((buttonValue == 2) && (!calculated)) {
    lcd.clear();

    circum = (1.0 / (encoderPos / 20.0)) * 100; // 20 ticks per revolution
    diam = circum / 3.14159;
    encoderPos = 0;

    calculated = true;
    menuChange = true;

  } else if (calculated) {
    if ((oldEncPos != encoderPos) || (menuChange)) {
      if (buttonValue % 2 == 0) {
        Serial.println(encoderPos);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Encoder: ");
        lcd.print(encoderPos);
        lcd.setCursor(13, 0);
        lcd.print("tks");
        lcd.setCursor(0, 1);
        lcd.print("Dist: ");
        lcd.print((encoderPos * circum) / 20);
        lcd.setCursor(14, 1);
        lcd.print("cm");

      } else {

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Circum: ");
        lcd.print(circum);
        lcd.setCursor(14, 0);
        lcd.print("cm");
        lcd.setCursor(0, 1);
        lcd.print("Diam: ");
        lcd.print(diam);
        lcd.setCursor(14, 1);
        lcd.print("cm");

      }

      oldEncPos = encoderPos;
      menuChange = false;
    }
  }
}




