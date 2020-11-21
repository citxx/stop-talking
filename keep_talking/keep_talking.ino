#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_LEDBackpack.h"

Adafruit_7segment timerDisplay = Adafruit_7segment();
Adafruit_8x8matrix mazeDisplay = Adafruit_8x8matrix();

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

// Structs and static constants for the Maze module
static const uint8_t PROGMEM MAZE_BORDER[] = {
  0b11111111,
  0b10000001,
  0b10000001,
  0b10000001,
  0b10000001,
  0b10000001,
  0b10000001,
  0b11111111,
};

struct Maze {
  uint8_t id1;
  uint8_t id2;
  uint8_t walls[];
};

static const Maze MAZES[] = {
  Maze{
    0x10, 0x25,
    {
      0b11000101, 0b01101100, 0b01010111,
      0b10101100, 0b00111001, 0b01010110,
      0b10101001, 0b01101100, 0b01010010,
      0b10101101, 0b00010011, 0b11010010,
      0b10000101, 0b01101100, 0b01111010,
      0b10010111, 0b10010011, 0b11010011,
    }
  },
};

// --- Main functions ---
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

  // Maze module setup
  mazeDisplay.begin(0x71);
  mazeDisplay.setBrightness(0);
  validateMaze(&MAZES[0]);
  initMazeModule();
}

void loop() {
  handleModules();
    
  delay(10);
}

void handleModules() {
  digitalWrite(inputClockPin, HIGH);
  digitalWrite(inputLatchPin, HIGH);
  delayMicroseconds(20);
  digitalWrite(inputLatchPin, LOW);

  digitalWrite(outputLatchPin, LOW);
  while (true) {
    byte value = shiftIn(inputDataPin, inputClockPin, MSBFIRST);
    int moduleId = value & 0xF;
    if (moduleId == 0) {
      break;
    }
    // Add modules here
    switch (moduleId) {
      case 1: {  // Timer
        handleTimer(value);
        break;
      }
      case 2: {  // Maze
        handleMaze(value);
        break;
      }
      default:
        Serial.print("Unknown module: "); Serial.println(moduleId);
        break;
    }
  }
  digitalWrite(outputLatchPin, HIGH);
}

// --- Timer handling functions ---
byte buttonState = 1;

void handleTimer(byte value) {
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

// --- Maze handling functions ---

// Auxilary maze functions
uint8_t idToX(uint8_t id) {
  return id % 16;
}
uint8_t idToY(uint8_t id) {
  return id / 16;
}
uint8_t pointToId(uint8_t x, uint8_t y) {
  return x + y * 16;
}

// Main maze functions
void drawMaze(const Maze* m){
  mazeDisplay.clear();
  mazeDisplay.drawBitmap(0, 0, MAZE_BORDER, 8, 8, LED_ON);
  mazeDisplay.drawPixel(idToX(m->id1)+1, idToY(m->id1)+1, LED_ON);
  mazeDisplay.drawPixel(idToX(m->id2)+1, idToY(m->id2)+1, LED_ON);
}

void drawMazePoints(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY) {
  startX++;
  startY++;
  endX++;
  endY++;
  
  unsigned long nowMs = millis();
  int period = 500;
  int onTime = 250;
  if (nowMs % period < onTime) {
    mazeDisplay.drawPixel(startX, startY, LED_ON);
  } else {
    mazeDisplay.drawPixel(startX, startY, LED_OFF);
  }
  mazeDisplay.drawPixel(endX, endY, LED_ON);
  mazeDisplay.writeDisplay();
}

const Maze* maze;
uint8_t startX, startY;
uint8_t endX, endY;

void initMazeModule(){
  maze = &MAZES[0];
  startX = 0;
  startY = 0;
  endX = 5;
  endY = 5;
}

void validateMaze(const Maze* m) {
  return;
}

bool isValidPoint(uint8_t x, uint8_t y) {
  return (x >= 0) && (x < 6) && (y >=0) && (y < 6);
}

bool isValidMove(const Maze* maze, uint8_t startX, uint8_t startY, uint8_t moveDirection) {
  uint8_t wallByte = maze->walls[startY * 3 + startX / 2];
  uint8_t wallBits = (wallByte >> ((1 - startX % 2) * 4)) & 0xF;
  return (wallBits & (1 << (3 - moveDirection))) == 0;
}

#define LEFT 0
#define UP 1
#define RIGHT 2
#define DOWN 3

const byte buttonShift[4] = {0, 1, 3, 2};
int lastButtonState[4] = {1, 1, 1, 1};

void handleMaze(byte value) {
  drawMaze(maze);
  drawMazePoints(startX, startY, endX, endY);
  mazeDisplay.writeDisplay();
  
  byte buttons = value >> 4;
  bool buttonPressed[4];
  for (int i = 0; i < 4; ++i) {
    int newState = (buttons >> buttonShift[i]) & 1;
    buttonPressed[i] = (lastButtonState[i] == 0 && newState == 1);
    lastButtonState[i] = newState;
  }
  uint8_t dx = 0, dy = 0;
  if (buttonPressed[LEFT]) {
    Serial.println("LEFT");
    if (isValidMove(maze, startX, startY, LEFT)) {
      startX -= 1;
    } else {
      Serial.println("WRONG MAZE MOVE");
    }
  }
  if (buttonPressed[UP]) {
    Serial.println("UP");
    if (isValidMove(maze, startX, startY, UP)) {
      startY -= 1;
    } else {
      Serial.println("WRONG MAZE MOVE");
    }
  }
  if (buttonPressed[RIGHT]) {
    Serial.println("RIGHT");
    if (isValidMove(maze, startX, startY, RIGHT)) {
      startX += 1;
    } else {
      Serial.println("WRONG MAZE MOVE");
    }
  }
  if (buttonPressed[DOWN]) {
    Serial.println("DOWN");
    if (isValidMove(maze, startX, startY, DOWN)) {
      startY += 1;
    } else {
      Serial.println("WRONG MAZE MOVE");
    }
  }
  byte solved = (startX == endX) && (startY == endY);
  shiftOut(outputDataPin, outputClockPin, MSBFIRST, solved);
}
