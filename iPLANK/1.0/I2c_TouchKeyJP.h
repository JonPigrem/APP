/*
 * I2c.h
 *
 *  Created on: Oct 14, 2013
 *      Author: Victor Zappi
 */

#ifndef I2CTK_H_
#define I2CTK_H_

#include <I2c.h>

// #define NUM_BYTES_OLD 9
// #define NUM_BYTES_NEW 13

#define MAX_SENSOR_BYTES 32

enum {
	kSensorTypeTouchKey = 0,
	kSensorTypeDBox1 = 1,
	kSensorTypeDBox2 = 2
};

static const int kSensorBytes[3] = {9, 13, 32};

class I2c_TouchKeyJP : public I2c
{
private:
	bool isReady;
	int sensorType;
	int numBytesToRead;

	// read NUM_BYTES bytes, which have to be properly parsed
	char dataBuffer[MAX_SENSOR_BYTES];
	int bytesRead;

	int rawSliderPosition[8];
	int rawSliderPositionH;

	int touchCount;
	float sliderSize[4];
	float sliderSizeY[4];
	float sliderPosition[4];
	float sliderPositionY[4];
	float sliderPositionH;


public:
	int initTouchKey(int sensorTypeToUse = kSensorTypeTouchKey);
	int readI2C();
	int getTouchCount();
	float * getSlidersize();
	float * getSliderPosition();
	float getSliderPositionH();

	bool ready() { return isReady; }

	I2c_TouchKeyJP();
	~I2c_TouchKeyJP();

};

inline int I2c_TouchKeyJP::getTouchCount()
{
	return touchCount;
}

inline float * I2c_TouchKeyJP::getSlidersize()
{
	return sliderSize;
}

inline float * I2c_TouchKeyJP::getSliderPosition()
{
	return sliderPosition;
}

inline float I2c_TouchKeyJP::getSliderPositionH()
{
	return sliderPositionH;
}





#endif /* I2CTK_H_ */
