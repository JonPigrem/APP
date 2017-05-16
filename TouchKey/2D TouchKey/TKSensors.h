#ifndef TKSENSORS_H_
#define TKSENSORS_H_

// Change this if needed
#define MAX_TOUCHES 5

#include <sys/time.h>	// elapsed time
#include "I2c_TouchKey.h"

class TKSensors
{
public:
	int initSensors(int bus, int address, int file, int sensorTypeToUse);
	int readSensors();
	int getTKTouchCount();
	float *getTKXPositions();
	float getTKYPosition();
	float *getTKTouchSize();

	TKSensors();

private:
	int sensorType;

	I2c_TouchKey TK;
	int rawTouchCount;
	float rawTouchSize[MAX_TOUCHES];
	float rawXPos[MAX_TOUCHES];
	float rawYPos;

	void resetSensorsData();
};

// read interface
inline int TKSensors::getTKTouchCount()
{
		return rawTouchCount;
}

inline float *TKSensors::getTKXPositions()
{
		return rawXPos;
}

inline float TKSensors::getTKYPosition()
{
		return rawYPos;
}

inline float *TKSensors::getTKTouchSize()
{
		return rawTouchSize;
}

#endif /* TKSENSORS_H_ */
