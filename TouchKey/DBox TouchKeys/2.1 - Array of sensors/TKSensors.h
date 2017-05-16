#ifndef TKSENSORS_H_
#define TKSENSORS_H_

#define MAX_TOUCHES 5 // default value = 5

#define NUMBER_OF_SENSORS 2 

#include <sys/time.h> // elapsed time
#include "I2c_TouchKey.h"

class TKSensors
{
public:
	int initSensors(int tk0_bus, int tk0_address, int tk1_bus, int tk1_address, int tk_file, int sensorTypeToUse);
	int readSensors();
	int getTKTouchCount(int index);
	float *getTKXPositions(int index);
	float getTKYPosition(int index);
	float *getTKTouchSize(int index);

	TKSensors();

private:
	int sensorType;

	I2c_TouchKey TK[NUMBER_OF_SENSORS];
	int tk_touchCnt[NUMBER_OF_SENSORS];
	float tk_touchPosX[NUMBER_OF_SENSORS][MAX_TOUCHES];
	float tk_touchPosY[NUMBER_OF_SENSORS];
	float tk_touchSize[NUMBER_OF_SENSORS][MAX_TOUCHES];

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
