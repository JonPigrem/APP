#ifndef TKSENSORS_H_
#define TKSENSORS_H_


// Change these settings before starting the patch ------------------------------------------------------------------------------
#define NUM_SENSORS 1 // Number of sensors used (default = 2)
#define MAX_TOUCHES 1 // Maximum number of touches on each sensor. Depends on sensor type (black TouchKey = 3, DBox TouchKey = 5)
int sensorType = 0; // Type of sensors used (black TouchKey = 0, DBox TouchKey = 2)
int sensorAddress[NUM_SENSORS] = {0x01}; // I2c address for each sensor (default = 0x18, 0x19)
// ------------------------------------------------------------------------------------------------------------------------------


// Libraries
#include <Bela.h>
#include <math.h>
#include "I2c_TouchKey.h"
#include <native/task.h>
#include <sys/time.h>


// Class
class TKSensors {
	public:
		int initSensors(int tk_bus, int tk_address, int tk_file, int sensorTypeToUse);
		int readSensors();
		int getTKTouchCount(int index);
		float *getTKXPositions(int index);
		float getTKYPosition(int index);
		float *getTKTouchSize(int index);
		TKSensors();
	private:
		int sensorType;
		I2c_TouchKey TK[NUM_SENSORS];
		int TKTouchCount[NUM_SENSORS];
		float TKXPositions[NUM_SENSORS][MAX_TOUCHES];
		float TKYPosition[NUM_SENSORS];
		float TKTouchSize[NUM_SENSORS][MAX_TOUCHES];
		void resetSensorsData();
};	

// Read sensors
inline int TKSensors::getTKTouchCount(int index) {
	return TKTouchCount[index];
}
inline float *TKSensors::getTKXPositions(int index) {
	return TKXPositions[index];
}
inline float TKSensors::getTKYPosition(int index) {
	return TKYPosition[index];
}
inline float *TKSensors::getTKTouchSize(int index) {
	return TKTouchSize[index];
}


#endif /* TKSENSORS_H_ */
