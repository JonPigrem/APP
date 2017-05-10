#include <Wire.h>
#include <CapSlider.h>

CapSlider slider1(25);
CapSlider slider2(24);
CapSlider slider3(26);

boolean touchActive1 = false;
boolean touchActive2 = false;
boolean touchActive3 = false;


void setup() {
  // Initialise serial and touch sensor
  Serial.begin(115200);

  if (!slider1.begin())
    Serial.println("failed to initialise slider1");

  if (!slider2.begin())
    Serial.println("failed to initialise slider2");


  if (!slider3.begin())
    Serial.println("failed to initialise slider3");

  slider1.setMode(CAPSLIDER_MODE_NORMAL);
  slider2.setMode(CAPSLIDER_MODE_NORMAL);
  slider3.setMode(CAPSLIDER_MODE_NORMAL);
}

void loop() {
  // Read 20 times per second
  delay(50);

  slider1.read();

  if (slider1.numberOfTouches() > 0) {
    for (int i = 0; i < slider1.numberOfTouches(); i++) {
      Serial.print("A"); // REMOVE THIS LATER
      Serial.print(slider1.touchLocation(i));
      Serial.print(" ");
      Serial.print(slider1.touchSize(i));
      Serial.print(" ");
    }
    Serial.println("");
    touchActive1 = true;
  }
  else if (touchActive1) {
    // Print a single line when touch goes off
    Serial.println("A" "0 0");
    touchActive1 = false;
  }

  slider2.read();

  if (slider2.numberOfTouches() > 0) {
    for (int i = 0; i < slider2.numberOfTouches(); i++) {
      Serial.print("B"); // REMOVE THIS LATER
      Serial.print(slider2.touchLocation(i));
      Serial.print(" ");
      Serial.print(slider2.touchSize(i));
      Serial.print(" ");
    }
    Serial.println("");
    touchActive2 = true;
  }

  else if (touchActive2) {
    // Print a single line when touch goes off
    Serial.println("B" "0 0");
    touchActive2 = false;
  }

  slider3.read();

  if (slider3.numberOfTouches() > 0) {
    for (int i = 0; i < slider3.numberOfTouches(); i++) {
      Serial.print("C"); // REMOVE THIS LATER
      Serial.print(slider3.touchLocation(i));
      Serial.print(" ");
      Serial.print(slider3.touchSize(i));
      Serial.print(" ");
    }
    Serial.println("");
    touchActive3 = true;
  }

  else if (touchActive3) {
    // Print a single line when touch goes off
    Serial.println("C" "0 0");
    touchActive3 = false;
  }

}
