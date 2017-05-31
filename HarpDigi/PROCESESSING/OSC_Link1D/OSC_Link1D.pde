import processing.serial.*;
import oscP5.*;
import netP5.*;

OscP5 oscP5;
NetAddress puredata;

// Communication
Serial gPort;
int gPortNumber = 2; 
int gBaudRate = 115200;

// Sensor settings
int gMaxNumTouches = 5;
int gMaxTouchLocation = 3200;
int gMaxTouchSize = 7000;

// Graphics dimensions
// Preserve aspect ratio with real sensor
int gSensorHeight = 150;
int gSensorWidth = int(gSensorHeight * 4.7);
int gMargin = 50;

// Stored data
int[] gTouchLocations1 = new int[gMaxNumTouches];
int[] gTouchSizes1 = new int[gMaxNumTouches];
int gCurrentNumTouches1 = 0;

int[] gTouchLocations2 = new int[gMaxNumTouches];
int[] gTouchSizes2 = new int[gMaxNumTouches];
int gCurrentNumTouches2 = 0;

int[] gTouchLocations3 = new int[gMaxNumTouches];
int[] gTouchSizes3 = new int[gMaxNumTouches];
int gCurrentNumTouches3 = 0;

void setup() {
  size(805, 650);

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
  rectMode(CORNER);
  
  oscP5.send(new OscMessage("/touch1").add(gCurrentNumTouches3), puredata);
  oscP5.send(new OscMessage("/touch2").add(gCurrentNumTouches2), puredata);
  oscP5.send(new OscMessage("/touch3").add(gCurrentNumTouches1), puredata);

  //------------------------- sensor 1

  rect(50, 50, 705, 150, 10);

  for (int i = 0; i < gCurrentNumTouches3; i++) {
    float displayLoc1 = map(gTouchLocations3[i], 0, gMaxTouchLocation, 0, gSensorWidth);
    float displaySize1 = map(gTouchSizes3[i], 0, gMaxTouchSize, 
      gSensorHeight * 0.1, gSensorHeight * 1.2);

    noStroke();
    fill(0, 0, 255);

    ellipse(displayLoc1 + gMargin, 125, displaySize1, displaySize1);
    oscP5.send(new OscMessage("/pos1").add(displayLoc1), puredata);
    oscP5.send(new OscMessage("/size1").add(displaySize1), puredata);
  }

  //------------------------- sensor 2


  stroke(0);
  fill(200);
  rectMode(CORNER);

  rect(50, 250, 705, 150, 10);

  for (int i = 0; i < gCurrentNumTouches2; i++) {
    float displayLoc2 = map(gTouchLocations2[i], 0, gMaxTouchLocation, 0, gSensorWidth);
    float displaySize2 = map(gTouchSizes2[i], 0, gMaxTouchSize, 
      gSensorHeight * 0.1, gSensorHeight * 1.2);

    noStroke();
    fill(0, 0, 255);

    ellipse(displayLoc2 + gMargin, 325, displaySize2, displaySize2);
    oscP5.send(new OscMessage("/pos2").add(displayLoc2), puredata);
    oscP5.send(new OscMessage("/size2").add(displaySize2), puredata);
  }

  //------------------------- sensor 3

  stroke(0);
  fill(200);
  rectMode(CORNER);

  rect(50, 450, 705, 150, 10);

  for (int i = 0; i < gCurrentNumTouches1; i++) {
    float displayLoc3 = map(gTouchLocations1[i], 0, gMaxTouchLocation, 0, gSensorWidth);
    float displaySize3 = map(gTouchSizes1[i], 0, gMaxTouchSize, 
      gSensorHeight * 0.1, gSensorHeight * 1.2);

    noStroke();
    fill(0, 0, 255);

    ellipse(displayLoc3 + gMargin, 525, displaySize3, displaySize3);
    oscP5.send(new OscMessage("/pos3").add(displayLoc3), puredata);
    oscP5.send(new OscMessage("/size3").add(displaySize3), puredata);
  }
}

void serialEvent(Serial p) {
  // Trim whitespace off input string
  String inString = trim(p.readString());

  if (inString.charAt(0) == 'A') {

    //-----------------------------------------------------------

    inString = inString.substring(1);

    // Convert to array
    int[] values1 = int(split(inString, " "));
    int i;

    for (i = 0; i < values1.length - 1; i += 2) {
      gTouchLocations1[i/2] = values1[i];
      gTouchSizes1[i/2] = values1[i + 1];

      if (i/2 >= gMaxNumTouches)
        break;
    }

    if (i < 2)
      gCurrentNumTouches1 = 0;
    else if (gTouchLocations1[0] == 0 && gTouchSizes1[0] == 0)
      gCurrentNumTouches1 = 0;
    else
      gCurrentNumTouches1 = i/2;
    //-----------------------------------------------------------
  }

  if (inString.charAt(0) == 'B') {

    //-----------------------------------------------------------

    inString = inString.substring(1);

    // Convert to array
    int[] values2 = int(split(inString, " "));
    int i;


    for (i = 0; i < values2.length - 1; i += 2) {
      gTouchLocations2[i/2] = values2[i];
      gTouchSizes2[i/2] = values2[i + 1];

      if (i/2 >= gMaxNumTouches)
        break;
    }

    if (i < 2)
      gCurrentNumTouches2 = 0;
    else if (gTouchLocations2[0] == 0 && gTouchSizes2[0] == 0)
      gCurrentNumTouches2 = 0;
    else
      gCurrentNumTouches2 = i/2;
    //-----------------------------------------------------------
  }

  if (inString.charAt(0) == 'C') {

    //-----------------------------------------------------------

    inString = inString.substring(1);

    // Convert to array
    int[] values3 = int(split(inString, " "));
    int i;

    for (i = 0; i < values3.length - 1; i += 2) {
      gTouchLocations3[i/2] = values3[i];
      gTouchSizes3[i/2] = values3[i + 1];

      if (i/2 >= gMaxNumTouches)
        break;
    }

    if (i < 2)
      gCurrentNumTouches3 = 0;
    else if (gTouchLocations3[0] == 0 && gTouchSizes3[0] == 0)
      gCurrentNumTouches3 = 0;
    else
      gCurrentNumTouches3 = i/2;
    //-----------------------------------------------------------
  }
}