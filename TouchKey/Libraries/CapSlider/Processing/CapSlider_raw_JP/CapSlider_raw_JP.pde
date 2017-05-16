import processing.serial.*;

// Communication
Serial gPort;
int gPortNumber = 2; 
int gBaudRate = 115200;
int gNumSensors = 30;
int gMaxSensorValue = 4096;

// Graphics dimensions
int gBarWidth = 25; 
int gBarMaxHeight = 300;
int gBarSpacing = 3;
int gMargin = 20;
int gTextSize = 10;
int gTextMargin = 3;
int gBottomLine = gMargin + gBarMaxHeight;

// Stored data
int[] gData = new int[gNumSensors];

void setup() {
  size(877, 356);
  
  println("Available ports: ");
  println(Serial.list());
  
  String portName = Serial.list()[3];
  
  println("Opening port " + portName);
  gPort = new Serial(this, portName, gBaudRate);
  gPort.bufferUntil('\n');
}

void draw() {
  background(255);
  
  stroke(0);
  line(gMargin, gBottomLine, width - gMargin, gBottomLine);
  
  pushMatrix();
  translate(gMargin, 0);
  textSize(gTextSize);
  
  for(int i = 0; i < gNumSensors; i++) {
    if(gData[i] != 0) {
      float barSize = map(gData[i], 0, gMaxSensorValue, 0, gBarMaxHeight);
      
      noStroke();
      fill(255, 0, 0);
      rectMode(CORNERS);
      rect(0, gBottomLine - barSize, gBarWidth, gBottomLine);
    }    
    
    fill(0);
    text(gData[i], 0, gBottomLine + gTextMargin + gTextSize);
    
    translate(gBarWidth + gBarSpacing, 0);
  }
  popMatrix();
}

void serialEvent(Serial p) {
  // Trim whitespace off input string
  String inString = trim(p.readString());

  // Convert to array
  int[] values = int(split(inString, " "));
  
  for(int i = 0; i < values.length; i++) {
    if(i >= gNumSensors)
      break;  
    gData[i] = values[i];
  }
}