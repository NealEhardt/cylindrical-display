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
byte sliceColumns[ColCount];
String inputString = "";


void setup() {
  Serial.begin(9600);
  Serial.println("Hello! I'm an Arduino.");
  
  motor.attach(MotorPin);
  pinMode(DebugPin, OUTPUT);
  pinMode(LatchPin, OUTPUT);
  pinMode(ShiftClockPin, OUTPUT);
  pinMode(ShiftDataPin, OUTPUT);
  pinMode(MagnetPin, INPUT);

  frameStartTime = micros();
  sliceNumber = 0;
}

void loop() {
  updateThrottle();
  updateDisplay();
}

void updateThrottle() {
  int throttle = analogRead(ThrottlePotPin);
  throttle = map(throttle, 0, 1023, 0, 180);
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

void getSerialSlice(byte slice[]) {
  int stringIdx = 0;
  for (int i = 0; i < ColCount; i++) {
    while (!isDigit(inputString[stringIdx])) {
      if (inputString[stringIdx] == '\n') {
        Serial.print(String()+"Slice terminated too soon (col "+i+")\n");
        return;
      }
      stringIdx++;
    }
    int start = stringIdx;
    while (isDigit(inputString[stringIdx])) {
      stringIdx++;
    }
    String substr = inputString.substring(start, stringIdx);
    slice[i] = substr.toInt();
  }
}

/*
 * Expected input is a list of base-10 numbers
 * followed by a newline.
 * 
 * For example, "128,5,2,1\n" or "64 32 16 8\n".
 * 
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    
    if (inChar == '\n') {
      byte slice[ColCount];
      getSerialSlice(slice);
      displaySlice(slice);
      inputString = "";
    }
  }
}

