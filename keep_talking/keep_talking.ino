#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_7segment timerDisplay = Adafruit_7segment();

int inputDataPin = 5;
int inputClockPin = 6;
int inputLatchPin = 7;

int outputDataPin = 2;
int outputClockPin = 3;
int outputLatchPin = 4;

unsigned long startMs;
int maxCounterSecs = 180;
int lastTimerSecs = 0;
bool explodedBlinkState = true;
 
void setup() {
  Serial.begin(9600);
  
  pinMode(inputDataPin, INPUT);
  pinMode(inputClockPin, OUTPUT);
  pinMode(inputLatchPin, OUTPUT);

  pinMode(outputDataPin, OUTPUT);
  pinMode(outputClockPin, OUTPUT);
  pinMode(outputLatchPin, OUTPUT);

  timerDisplay.begin(0x77);

  startMs = millis();
}

void loop() {
  handleModules();
    
  delay(10);
}

byte buttonState = 1;

void handleModules() {
  digitalWrite(inputClockPin, HIGH);
  digitalWrite(inputLatchPin, HIGH);
  delayMicroseconds(20);
  digitalWrite(inputLatchPin, LOW);

  digitalWrite(outputLatchPin, LOW);
  while (true) {
    byte value = shiftIn(inputDataPin, inputClockPin, MSBFIRST);
    Serial.print(value, BIN); Serial.print(" ");
    int moduleId = value & 0xF;
    if (moduleId == 0) {
      break;
    }
    switch (moduleId) {
      case 1: {  // Timer
        handleTimer();
        break;
      }
      default:
        Serial.print("Unknown module: "); Serial.println(moduleId);
        break;
    }
  }
  Serial.println();
  digitalWrite(outputLatchPin, HIGH);
}

void handleTimer() {
  // Reset button
  byte newButtonState = (value >> 4) & 1;
  if (!newButtonState && buttonState) { // Button is just pressed
    startMs = millis();
  }
  buttonState = newButtonState;
  shiftOut(outputDataPin, outputClockPin, MSBFIRST, buttonState);

  // Display
  unsigned long nowMs = millis();
  long passedMs = long(nowMs - startMs);
  int timerSecs = maxCounterSecs - passedMs / 1000;
  int mins = timerSecs / 60;
  int secs = timerSecs % 60;

  if (timerSecs >= 0) {
    int numToWrite = mins * 100 + secs;
    for (int i = 0; i < 4; ++i) {
      timerDisplay.writeDigitNum(i, 0);
    }
    timerDisplay.println(numToWrite, DEC);
    if (numToWrite < 1000) {
      timerDisplay.writeDigitNum(0, 0);
    }
    if (numToWrite < 100) {
      timerDisplay.writeDigitNum(1, 0);
    }
    if (numToWrite < 10) {
      timerDisplay.writeDigitNum(3, 0);
    }
    timerDisplay.drawColon(true);  
  } else {
    bool secondsChanged = (timerSecs != lastTimerSecs);
    lastTimerSecs = timerSecs;
    explodedBlinkState = (explodedBlinkState != secondsChanged);
    if (explodedBlinkState) {
      for (int i = 0; i < 5; ++i) {
        timerDisplay.writeDigitNum(i, 0);
      }
      timerDisplay.drawColon(true);  
    } else {
      timerDisplay.clear();
    }
  }
  timerDisplay.writeDisplay();
}
