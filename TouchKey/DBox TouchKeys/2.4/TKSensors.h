#ifndef TKSENSORS_H_
#define TKSENSORS_H_

// Change these settings here and in TKsensors.cpp depending on each specific setup
// -----------------------------------------------------------------------------------------------
#define NUM_SENSORS 2			// Number of sensors used
#define MAX_TOUCHES 5 			// Maximum number of touches on each sensor. Depends on sensor type (default = 5)
// -----------------------------------------------------------------------------------------------

#include <sys/time.h> // elapsed time
#include "I2c_TouchKey.h"

class TKSensors
{
public:
	int initSensors(int tk_bus, int tk0_address, int tk1_address, int tk_file, int sensorTypeToUse);
	int readSensors();
	int getTKTouchCount(int index);
	float *getTKXPositions(int index);
	float getTKYPosition(int index);
	float *getTKTouchSize(int index);

	TKSensors();

private:
	int sensorType;

	I2c_TouchKey TK[NUM_SENSORS];
	int tk_touchCnt[NUM_SENSORS];
	float tk_touchPosX[NUM_SENSORS][MAX_TOUCHES];
	float tk_touchPosY[NUM_SENSORS];
	float tk_touchSize[NUM_SENSORS][MAX_TOUCHES];

	void resetSensorsData();
};

// read interface
inline int TKSensors::getTKTouchCount(int index)
{
	return tk_touchCnt[index];
}

inline float *TKSensors::getTKXPositions(int index)
{
	return tk_touchPosX[index];
}

inline float TKSensors::getTKYPosition(int index)
{
	return tk_touchPosY[index];
}

inline float *TKSensors::getTKTouchSize(int index)
{
	return tk_touchSize[index];
}

#endif /* TKSENSORS_H_ */
