#ifndef TKSENSORS_H_
#define TKSENSORS_H_

// Change this if needed
#define MAX_TOUCHES 5

#include <sys/time.h>	// elapsed time
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

	I2c_TouchKey TK0;
	int tk0_touchCnt;
	float tk0_touchPosX[MAX_TOUCHES];
	float tk0_touchPosY;
	float tk0_touchSize[MAX_TOUCHES];

	I2c_TouchKey TK1;
	int tk1_touchCnt;
	float tk1_touchPosX[MAX_TOUCHES];
	float tk1_touchPosY;
	float tk1_touchSize[MAX_TOUCHES];

	void resetSensorsData();
};

// read interface
inline int TKSensors::getTKTouchCount(int index)
{
	if(index==0)
		return tk0_touchCnt;
	else
		return tk1_touchCnt;
}

inline float *TKSensors::getTKXPositions(int index)
{
	if(index==0)
		return tk0_touchPosX;
	else
		return tk1_touchPosX;
}

inline float TKSensors::getTKYPosition(int index)
{
	if(index==0)
		return tk0_touchPosY;
	else
		return tk1_touchPosY;
}

inline float *TKSensors::getTKTouchSize(int index)
{
	if(index==0)
		return tk0_touchSize;
	else
		return tk1_touchSize;
}

#endif /* TKSENSORS_H_ */
