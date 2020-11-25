#include <Wire.h>

int outputDataPin = 8;
int outputClockPin = 9;
int outputLatchPin = 10;

#define COMMON_ANODE

static const byte COLORS[6] = {
  0b011, // red
  0b101, // green
  0b110, // blue
  0b001, // yellow
  0b000, // white
  0b111, // no wire
};

void setup()
{
  Serial.begin(9600);

  pinMode(outputDataPin, OUTPUT);
  pinMode(outputClockPin, OUTPUT);
  pinMode(outputLatchPin, OUTPUT);

  randomSeed(analogRead(0));
  generateColors();
}

uint8_t wireColors[6];

void generateColors() {
  for (int i = 0; i < 6; i++) {
    wireColors[i] = random(6);
  }
}

void showColors() {
  byte colors1 = COLORS[wireColors[0]] << 5
                 | COLORS[wireColors[1]] << 2
                 | COLORS[wireColors[2]] >> 1;
  byte colors2 = COLORS[wireColors[2]] << 7
                 | COLORS[wireColors[3]] << 4
                 | COLORS[wireColors[4]] << 1 
                 | COLORS[wireColors[5]] >> 2;
  byte colors3 = COLORS[wireColors[5]] << 6;

  shiftOut(outputDataPin, outputClockPin, LSBFIRST, colors1);
  shiftOut(outputDataPin, outputClockPin, LSBFIRST, colors2);
  shiftOut(outputDataPin, outputClockPin, LSBFIRST, colors3);
}

void loop()
{
  delayMicroseconds(20);
  digitalWrite(outputLatchPin, LOW);
  showColors();
  digitalWrite(outputLatchPin, HIGH);
}
