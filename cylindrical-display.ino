#include <Servo.h>

const int throttlePotPin = 0; // A0
const int motorPin = 8;
Servo motor;

const int shiftClockPin = 40;
const int shiftDataPin = 41;
const int latchPin = 42;

float framesPerSecond = 60;
const int slicesPerFrame = 64;
unsigned long microsPerFrame;
unsigned long frameStartTime;
int sliceNumber;

char data[8][65] = {
  "--     --   ---------   --          --            -----        ",
  "--     --   --          --          --           --   --       ",
  "--     --   --          --          --          --     --      ",
  "---------   ---------   --          --          --     --      ",
  "--     --   --          --          --          --     --      ",
  "--     --   --          --          --          --     --      ",
  "--     --   --          --          --           --   --       ",
  "--     --   ---------   ---------   ---------     -----        "
};
  
void setup() {
  Serial.begin(9600);
  motor.attach(motorPin);
  
  pinMode(latchPin, OUTPUT);
  pinMode(shiftClockPin, OUTPUT);
  pinMode(shiftDataPin, OUTPUT);
  
  microsPerFrame = 1000000 / framesPerSecond;
  frameStartTime = micros();
  sliceNumber = 0;
}

void loop() {
  // shift out a slice if enough time has passed
  unsigned long nextSliceTime = frameStartTime
                  + microsPerFrame * (sliceNumber + 1) / slicesPerFrame;
  if (nextSliceTime <= micros()) {
    byte slice = getSlice(sliceNumber);
    digitalWrite(latchPin, LOW);
    shiftOut(shiftDataPin, shiftClockPin, LSBFIRST, slice);
    digitalWrite(latchPin, HIGH);
    sliceNumber++;
    if (sliceNumber >= 64) {
      sliceNumber = 0;
      frameStartTime += microsPerFrame;
    }
  }
  
  // update throttle
  int throttle = analogRead(throttlePotPin);
  throttle = map(throttle, 0, 1023, 0, 180);
  motor.write(throttle);
}

byte getSlice(int number) {
  byte slice = 0;
  for (int i = 0; i < 8; i++) {
    if (data[i][number] != ' ') {
      bitSet(slice, i);
    }
  }
  return slice;
}


