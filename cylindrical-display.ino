#include <Servo.h>
#include "dimensions.h"
#include "scratch_data.h"

// slip ring color coding
// red    - V+
// yellow - latch
// green  - data out
// blue   - (unused / data in)
// white  - clock
// black  - ground

// Analog in pins
const int ThrottlePotPin = 0; // A0

// PWM pins
const int MotorPin = 8;

// Digital pins
const int DebugPin = 13;
const int ShiftClockPin = 40; // white
const int ShiftDataPin = 41; // green
const int LatchPin = 42; // yellow
const int MagnetPin = 50;


// global state
unsigned long frameStartTime;
int sliceNumber;
Servo motor;
byte serialSlice[ColCount];
int serialSliceIdx = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Hello! I'm an Arduino.");
  
  motor.attach(MotorPin);
  pinMode(DebugPin, OUTPUT);
  pinMode(LatchPin, OUTPUT);
  pinMode(ShiftClockPin, OUTPUT);
  pinMode(ShiftDataPin, OUTPUT);
  pinMode(MagnetPin, INPUT);

  frameStartTime = micros();
  sliceNumber = 0;
  
  clearDisplay();
  clearDisplay();
}

void loop() {
  updateThrottle();
  //updateDisplay();
}

void clearDisplay() {
  static byte slice[ColCount] = {0};
  displaySlice(slice);
}

void updateThrottle() {
  int throttle = analogRead(ThrottlePotPin);
  throttle = map(throttle, 0, 1023, 10, 170);
  motor.write(throttle);
  
  bool isMagnet = digitalRead(MagnetPin);
  digitalWrite(DebugPin, isMagnet);
}

void updateDisplay() {
  // shift out a slice if enough time has passed
  unsigned long nextSliceTime = frameStartTime
                                + MicrosPerFrame * (sliceNumber + 1) / SlicesPerFrame;
  if (nextSliceTime <= micros()) {
    byte slice[ColCount];
    getScratchSlice(slice, sliceNumber);
    displaySlice(slice);
    
    sliceNumber++;
    if (sliceNumber >= SlicesPerFrame) {
      sliceNumber = 0;
      frameStartTime += MicrosPerFrame;
    }
  }
}

void getScratchSlice(byte slice[], int number) {
  for (int i = 0; i < ColCount; i++) {
    slice[i] = 0;
    for (int j = 0; j < RowCount; j++) {
      if (scratchData[number][j][i] != ' ') {
        bitSet(slice[i], j);
      }
    }
  }
}

void displaySlice(byte slice[]) {
  digitalWrite(LatchPin, LOW);
  for (int i = 0; i < ColCount; i++) {
    shiftOut(ShiftDataPin, ShiftClockPin, LSBFIRST, slice[i]);
  }
  digitalWrite(LatchPin, HIGH);
}

/*
 * Reads ColCount bytes at a time and shifts them out
 * 
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    serialSlice[serialSliceIdx++] = Serial.read();
    
    if (serialSliceIdx == ColCount) {
      serialSliceIdx = 0;
      displaySlice(serialSlice);
    }
  }
}

