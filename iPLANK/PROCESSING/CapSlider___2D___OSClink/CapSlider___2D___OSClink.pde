import processing.serial.*;
import oscP5.*;
import netP5.*;

// Communication
Serial gPort;
int gPortNumber = 2; 
int gBaudRate = 115200;

// Sensor settings
int gMaxNumTouches = 4;
int gMaxTouchLocation = 14*128;
int gMaxTouchSize = 7000;

// Graphics dimensions
// Preserve aspect ratio with real sensor
int gSensorHeight = 600;
int gSensorWidth = 600;
int gMargin = 50;

// Stored data
int[] gTouchLocations = new int[gMaxNumTouches];
int[] gTouchSizes = new int[gMaxNumTouches];
int[] gTouchHLocations = new int[gMaxNumTouches];
int[] gTouchHSizes = new int[gMaxNumTouches];
int gCurrentNumTouches = 0, gCurrentNumHTouches = 0;

void setup() {
  size(700, 700);

  println("Available ports: ");
  println(Serial.list());

  String portName = Serial.list()[3];

  println("Opening port " + portName);
  gPort = new Serial(this, portName, gBaudRate);
  gPort.bufferUntil('\n');

  oscP5 = new OscP5(this, 12000);
  // PureData address
  puredata = new NetAddress("127.0.0.1", 8000);
}

void draw() {
  background(255);

  stroke(0);
  fill(200);
  rectMode(CORNERS);

  oscP5.send(new OscMessage("/touch1").add(gCurrentNumTouches1), puredata);

  rect(gMargin, gMargin, width - gMargin, height - gMargin, 10);

  for (int i = 0; i < gCurrentNumTouches; i++) {
    for (int j = 0; j < gCurrentNumHTouches; j++) {
      float displayXLoc = map(gTouchHLocations[j], 0, gMaxTouchLocation, 0, gSensorWidth);
      float displayYLoc = map(gTouchLocations[i], 0, gMaxTouchLocation, gSensorHeight, 0);

      float displayXSize = map(gTouchHSizes[j], 0, gMaxTouchSize, 
        gSensorHeight * 0.05, gSensorHeight * 0.5);
      float displayYSize = map(gTouchSizes[i], 0, gMaxTouchSize, 
        gSensorHeight * 0.05, gSensorHeight * 0.5);

      noStroke();
      fill(0, 0, 255);

      ellipse(displayXLoc + gMargin, displayYLoc + gMargin, displayXSize, displayYSize);

      oscP5.send(new OscMessage("/pos1").add(displayLoc1), puredata);
      oscP5.send(new OscMessage("/size1").add(displaySize1), puredata);
    }
  }
}

void serialEvent(Serial p) {
  // Trim whitespace off input string
  String inString = trim(p.readString());

  // Convert to array
  int[] values = int(split(inString, " "));
  int i;

  // Look for first two numbers telling us number of H and V touches
  if (values.length < 2) {
    gCurrentNumTouches = gCurrentNumHTouches = 0;
    return;
  }

  gCurrentNumTouches = values[0];
  gCurrentNumHTouches = values[1];

  for (i = 0; i < gCurrentNumTouches; i++) {
    if (i*2 + 3 >= values.length) {
      // Malformed line...
      gCurrentNumTouches = gCurrentNumHTouches = 0;
      return;
    }

    gTouchLocations[i] = values[2 + i*2];
    gTouchSizes[i] = values[2 + i*2 + 1];
  }

  for (i = 0; i < gCurrentNumHTouches; i++) {
    if ((i + gCurrentNumTouches)*2 + 3 >= values.length) {
      // Malformed line...
      gCurrentNumTouches = gCurrentNumHTouches = 0;
      return;
    }

    gTouchHLocations[i] = values[2 + (i + gCurrentNumTouches)*2];
    gTouchHSizes[i] = values[2 + (i + gCurrentNumTouches)*2 + 1];
  }
}