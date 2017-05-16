import processing.serial.*;

// Communication
Serial gPort;
int gPortNumber = 3; 
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

// Cap settings
int gCapSliderSpeed = 0;
int gCapSliderResolution = 12;

String[] gSpeedDescriptions = {"Ultra Fast", "Fast", "Normal", "Slow"};

void setup() {
  size(877, 356);
  
  println("Available ports: ");
  println(Serial.list());
  
  String portName = Serial.list()[gPortNumber];
  
  println("Opening port " + portName);
  gPort = new Serial(this, portName, gBaudRate);
  gPort.bufferUntil('\n');
  
  noLoop();
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
  
  redraw();
}

void keyPressed() {
  // Use keys to change settings  
  if(key >= '1' && key <= '8') {
    // Change resolution
    gCapSliderResolution = key - '1' + 9;
    println("Setting speed to " + gSpeedDescriptions[gCapSliderSpeed] + " and " + gCapSliderResolution + "-bit resolution");
    gPort.write("2 " + gCapSliderSpeed + " " + gCapSliderResolution + "\n");  
  }
  
  if(key == 'q' || key == 'w' || key == 'e' || key == 'r') {
    // Change speed
    int speed = 0;
    if(key == 'w')
      speed = 1;
    else if(key == 'e')
      speed = 2;
    else if(key == 'r')
      speed = 3;
      
    gCapSliderSpeed = speed;
    println("Setting speed to " + gSpeedDescriptions[gCapSliderSpeed] + " and " + gCapSliderResolution + "-bit resolution");
    gPort.write("2 " + gCapSliderSpeed + " " + gCapSliderResolution + "\n");  
  }
  
  if(key == 'a' || key == 's' || key == 'd' || key == 'f' || key == 'g' || key == 'h') {
    // Change prescaler
    int prescaler = 1;
    if(key == 's')  
      prescaler = 2;
    else if(key == 'd')
      prescaler = 4;
    else if(key == 'f')
      prescaler = 8;
    else if(key == 'g')
      prescaler = 16;
    else if(key == 'h')
      prescaler = 32;
      
    println("Setting prescaler to " + prescaler);
    gPort.write("3 " + prescaler + "\n");
  }
  
  if(key == 'z' || key == 'x' || key == 'c' || key == 'v' || key == 'b' || key == 'n' || key == 'm') {
    // Set noise threshold
    int threshold = 0;
    if(key == 'x')  
      threshold = 10;
    else if(key == 'c')
      threshold = 20;
    else if(key == 'v')
      threshold = 30;
    else if(key == 'b')
      threshold = 40;
    else if(key == 'n')
      threshold = 50;
    else if(key == 'm')
      threshold = 60;
      
    println("Setting noise threshold to " + threshold);
    gPort.write("4 " + threshold + "\n");
  }
  
  if(key == ' ') {
    // Reset baseline
    println("Resetting baseline");
    gPort.write("6\n");
  }
  
  if(key == '[') {
    // Zoom out display
    if(gMaxSensorValue < 65536)
      gMaxSensorValue *= 2;
  }
  
  if(key == ']') {
    // Zoom in display
    if(gMaxSensorValue > 128)
      gMaxSensorValue /= 2;  
  }
  
  if(key == '\n') {
    // Print current values
    for(int i = 0; i < gNumSensors; i++)
      print(gData[i] + " ");
    println("");  
  }
}