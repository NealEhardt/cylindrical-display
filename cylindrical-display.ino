#include <Servo.h>

// slip ring color coding
// red    - V+
// yellow - latch
// green  - data
// blue   - (unused)
// white  - clock
// black  - ground

const int ThrottlePotPin = 0; // A0
const int MotorPin = 8;
Servo motor;

const int ShiftClockPin = 40; // white
const int ShiftDataPin = 41; // green
const int LatchPin = 42; // yellow

const float FramesPerSecond = .5;
const int ColCount = 8;
const int RowCount = 8;
const int SlicesPerFrame = 64;
const unsigned long MicrosPerFrame = 1000000 / FramesPerSecond;

// Data include depends on the dimensional constants above.
#include "scratch_data.h"

// global vars
unsigned long frameStartTime;
int sliceNumber;


void setup() {
  Serial.begin(9600);
  motor.attach(MotorPin);
  
  pinMode(LatchPin, OUTPUT);
  pinMode(ShiftClockPin, OUTPUT);
  pinMode(ShiftDataPin, OUTPUT);
  
  frameStartTime = micros();
  sliceNumber = 0;
}

void loop() {
  // shift out a slice if enough time has passed
  unsigned long nextSliceTime = frameStartTime
                  + MicrosPerFrame * (sliceNumber + 1) / SlicesPerFrame;
  if (nextSliceTime <= micros()) {
    byte slice[ColCount];
    getSlice(slice, sliceNumber);
    digitalWrite(LatchPin, LOW);
    for (int i = 0; i < ColCount; i++) {
      shiftOut(ShiftDataPin, ShiftClockPin, LSBFIRST, slice[i]);
    }
    digitalWrite(LatchPin, HIGH);
    sliceNumber++;
    if (sliceNumber >= SlicesPerFrame) {
      sliceNumber = 0;
      frameStartTime += MicrosPerFrame;
    }
  }
  
  // update throttle
  int throttle = analogRead(ThrottlePotPin);
  throttle = map(throttle, 0, 1023, 0, 180);
  motor.write(throttle);
}

void getSlice(byte slice[], int number) {
  for (int i = 0; i < ColCount; i++) {
    slice[i] = 0;
    for (int j = 0; j < RowCount; j++) {
      if (data[i][j][number] != ' ') {
        bitSet(slice[i], j);
      }
    }
  }
}


