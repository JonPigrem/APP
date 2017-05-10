#include <math.h>
#include "TKSensors.h"
#include <native/task.h>
#include <Bela.h>

// change these values if needed
#define MAX_TOUCHES 5 // default value = 5
int touchSensorAddress = 0x18;	// I2c address
int sensorType = 2; // sensorType = 2 for 1D TouchKey

// rework this to figure out which data we exactly want
float gSensorLatestTouchPos = 0;	// most recent pitch touch location [0-1] on sensor 0, used by render.cpp
int gSensorLatestTouchNum	 = 0;	// most recent num of touches on sensor 0, used by render.cpp
int gSensorLatestTouchIndex = 0;	// index of last touch in gSensor0LatestTouchPos[5], used by render.cpp
int loopIndex = 0;

// don't touch this
TKSensors Sensor;
extern int gShouldStop;

// initialise touchkey sensor
int TKSensors::initSensors(int bus, int address, int file, int sensorTypeToUse)
{
	sensorType = sensorTypeToUse;
	// init touch key on i2c bus
	if(address >= 0) {
		if(TK.initI2C_RW(bus, address, file)>0)
			return 1;
		if(TK.initTouchKey(sensorType)>0)
			return 2;
	}

	return 0;
}

// this records raw sensor data if the touchkey is ready
int TKSensors::readSensors()
{
	if(TK.ready()) {
		if(TK.readI2C()>0)
			return 1;
		rawTouchCount = TK.getTouchCount();
		rawYPos = TK.getSliderPositionH();
		for(int i=0; i<MAX_TOUCHES; i++) {
			rawXPos[i] = TK.getSliderPosition()[i];
			rawTouchSize[i] = TK.getSlidersize()[i];
		}
	}
	else {
		rawTouchCount = 0;
		resetSensorsData();
	}
	return 0;
}

// this resets the raw data if the sensor is not ready
TKSensors::TKSensors()
{
	resetSensorsData();
}

void TKSensors::resetSensorsData()
{
	for(int i=0; i<MAX_TOUCHES; i++)
	{
		rawXPos[i] = -1;
		rawYPos	 = -1;
		rawTouchSize[i] = 0;
	}
	return;
}



// record number of touches and touch size for each finger
int initSensorLoop(int sensorAddress, int sensorType)
{
	int bus			= 1;
	int address		= sensorAddress;
	int file			= 0;

	if(Sensor.initSensors(bus, address, file, sensorType)>0)
	{
		gShouldStop = 1;
		cout << "control cannot start" << endl;
		return -1;
	}

	return 0;
}


void sensorLoop(void *)
{
	timeval start, end;
	unsigned long elapsedTime;
	//float touchSize		= 0;	// once used for timbre

	int newTouchCount;
	float* newTouchSize;
	float* newXPos;
	float newYPos;

	// init time vals
	gettimeofday(&start, NULL);

	// here we go, sensor loop until the end of the application
	while(!gShouldStop)
	{
		gettimeofday(&end, NULL);
		elapsedTime = ( (end.tv_sec*1000000+end.tv_usec) - (start.tv_sec*1000000+start.tv_usec) );
		if( elapsedTime<4000 )
			usleep(4000-elapsedTime);

		if(Sensor.readSensors()==0)
		{
			newTouchCount = Sensor.getTKTouchCount();
			newXPos	= Sensor.getTKXPositions();
			newYPos	= Sensor.getTKYPosition();
			newTouchSize = Sensor.getTKTouchSize();

			// the loop index makes sure we don't print too often
			loopIndex++;
			if (loopIndex == 100) {
				loopIndex = 0;
				printf("==========================================================\n");
				printf("Y Position:\t%f\tTouch Count:\t%d\n", newYPos, newTouchCount);
				printf("----------------------------------------------------------\n");
				printf("X Position:\t\t\tTouch Size:\n");
				for(int i=0; i<newTouchCount; i++)
					printf("\t\t\t%f\t\t\t%f\n", newXPos[i], newTouchSize[i]);
			}
		}
		gettimeofday(&start, NULL);
	}
}

void sensorLoop(void *);

// figure out what this does
struct arg_data
{
   int  argc;
   char **argv;
};

arg_data args;

void parseArguments(arg_data args, BelaInitSettings *settings)
{
	Bela_defaultSettings(settings);
}

int main(int argc, char *argv[])
{
	BelaInitSettings settings;	// Standard audio settings
	RT_TASK rtSensorThread;
	const char rtSensorThreadName[] = "dbox-sensor";
	int oscBankHopSize;

	// Parse command-line arguments
	args.argc = argc;
	args.argv = argv;
	parseArguments(args, &settings);

	// Initialise the audio device
	if(Bela_initAudio(&settings, &oscBankHopSize) != 0)
		return -1;

	if(initSensorLoop(touchSensorAddress, sensorType) != 0)
		return -1;

	if(Bela_startAudio()) {
		cout << "Error: unable to start real-time audio" << endl;
		return -1;
	}

	if(rt_task_create(&rtSensorThread, rtSensorThreadName, 0, BELA_AUDIO_PRIORITY - 5, T_JOINABLE | T_FPU)) {
		  cout << "Error:unable to create Xenomai control thread" << endl;
		  return -1;
	}
	
	if(rt_task_start(&rtSensorThread, &sensorLoop, 0)) {
		  cout << "Error:unable to start Xenomai control thread" << endl;
		  return -1;
	}
	
	// Run until told to stop
	while(!gShouldStop) {
		usleep(100000);
	}
	
	Bela_stopAudio();
	Bela_cleanupAudio();
	return 0;
}