// For I2C
#include <Wire.h>
// Libraries for Matrix
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

static const uint8_t PROGMEM border[] = {
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
  uint8_t maze[];
  //uint8_t startPoint;
  //uint8_t endPoint;
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

uint8_t idToX(uint8_t id) {
  return id % 16;
}
uint8_t idToY(uint8_t id) {
  return id / 16;
}

uint8_t pointToId(uint8_t x, uint8_t y) {
  return x + y * 16;
}

void setMaze(const Maze& m){
  matrix.clear();
  matrix.drawBitmap(0, 0, border, 8, 8, LED_ON);
  matrix.drawPixel(idToX(m.id1)+1, idToY(m.id1)+1, LED_ON);
  matrix.drawPixel(idToX(m.id2)+1, idToY(m.id2)+1, LED_ON);
}

void blinkPoints(uint8_t start_x, uint8_t start_y, uint8_t end_x, uint8_t end_y) {
  start_x++;
  start_y++;
  end_x++;
  end_y++;
  matrix.drawPixel(start_x, start_y, LED_ON);
  matrix.drawPixel(end_x, end_y, LED_ON);
  matrix.writeDisplay();
  delay(250);
  matrix.drawPixel(start_x, start_y, LED_OFF);
  matrix.writeDisplay();
  delay(250);
  matrix.drawPixel(start_x, start_y, LED_ON);
  matrix.writeDisplay();
}

void validate(const Maze& m) {
  return;
}

void setup() {
  Serial.begin(9600);
  Serial.println("8x8 LED Matrix Test");
  matrix.begin(0x71);
  matrix.setBrightness(0);
  validate(MAZES[0]);
}

void loop() {
  setMaze(MAZES[0]);
  uint8_t start_x = 0, start_y = 0;
  uint8_t end_x = 5, end_y = 5;
  blinkPoints(start_x, start_y, end_x, end_y);
  matrix.writeDisplay();
  delay(500);
}
