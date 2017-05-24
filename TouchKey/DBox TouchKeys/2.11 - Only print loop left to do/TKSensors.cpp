// Change the settings in TKSensors.h before starting the patch
// ------------------------------------------------------------

#include "TKSensors.h"

// Index for printing loop - To remove once the patch is connected to Pure Data (remove code in line 276 as well)
int loopIndex = 0;

// Class instance
TKSensors Sensors;

// Variables for reading sensors
int sTouchNum[NUM_SENSORS];
float sTouches_[NUM_SENSORS][MAX_TOUCHES];
float sSize_[NUM_SENSORS][MAX_TOUCHES];
int sLastIndex[NUM_SENSORS];

// Variable for application status
extern int gShouldStop;


// Initialise sensors
int TKSensors::initSensors(int tk_bus, int tk_address, int tk_file, int sensorTypeToUse)
{
	sensorType = sensorTypeToUse;
	for (int i = 0; i < NUM_SENSORS; i++) {
		if(tk_address >= 0) {
			if(TK[i].initI2C_RW(tk_bus, tk_address, tk_file)>0)
				return 1;
			if(TK[i].initTouchKey(sensorType)>0)
				return 2;
		}
	}
	return 0;
}
	

// Retrieve data if sensors are currently being used
int TKSensors::readSensors() {
	// Mark each ready sensor as currently used by retrieving touch count
	for (int i = 0; i < NUM_SENSORS; i++) {
		if(TK[i].ready()) {
			if(TK[i].readI2C()>0)
				return 1;
			TKTouchCount[i] = TK[i].getTouchCount();
		}
		else
			TKTouchCount[i] = 0;
	}
	
	// If no sensor currently used, reset sensor data
	int sensorsUsed = 0;
	for (int i = 0; i < NUM_SENSORS; i++) {
		if(TKTouchCount[i] != 0)
			sensorsUsed++;
	}
	if (sensorsUsed == 0)
		resetSensorsData();
	
	// But if any sensor is used, retrieve data for each sensor
	else {
		for (int i = 0; i < NUM_SENSORS; i++) {
			for(int j=0; j<MAX_TOUCHES; j++) {
				TKXPositions[i][j] = TK[i].getSliderPosition()[j];
				TKTouchSize[i][j] = TK[i].getSlidersize()[j];
			}
		TKYPosition[i] 	 = TK[i].getSliderPositionH();
		}
	}
	return 0;
}


// Reset data when no sensors are being used
TKSensors::TKSensors() {
	resetSensorsData();
}

void TKSensors::resetSensorsData() {
	for (int i = 0; i < NUM_SENSORS; i++) {
		for(int j=0; j<MAX_TOUCHES; j++) {
			TKXPositions[i][j] = -1;
			TKYPosition[i]	 = -1;
			TKTouchSize[i][j] = 0;
		}
	}
	return;
}


// Initialise the sensor loop (not sure what this does)
int initSensorLoop() {
	int tk_file	= 0;
	
	// Indicate each tk_bus as 1 for each sensor regardless of the size of the array
	int tk_bus[NUM_SENSORS]; 
	std::fill_n(tk_bus, NUM_SENSORS, 1);
	
	for (int i = 0; i < NUM_SENSORS; i++) {
		if(Sensors.initSensors(tk_bus[i], sensorAddress[i], tk_file, sensorType) > 0) {
			gShouldStop = 1;
			cout << "control cannot start" << endl;
			return -1;
		}
	}
	
	for (int i = 0; i < NUM_SENSORS; i++) {
		for(int j = 0; j < MAX_TOUCHES; j++) {
			sTouches_[i][j]	= 0.0;
			sSize_[i][j] = 0.0;
		}
	}
	return 0;
}


// Sensor loop
void sensorLoop(void *)
{
	timeval start, end;
	unsigned long elapsedTime;

	// Variables for the sensor loop
	float *sTouches[NUM_SENSORS];
	float sYTouches[NUM_SENSORS];
	float *sSize[NUM_SENSORS];
	int sPrevTouchNum[NUM_SENSORS];
	std::fill_n(sPrevTouchNum, NUM_SENSORS, 0);
	int sSortedTouchIndices[NUM_SENSORS][MAX_TOUCHES];
	float sSortedTouches[NUM_SENSORS][MAX_TOUCHES];
	float sPrevSortedTouches[NUM_SENSORS][MAX_TOUCHES];

	// Initialise the time values
	gettimeofday(&start, NULL);

	// Launch the sensor loop until the end of the application
	while(!gShouldStop)
	{
		gettimeofday(&end, NULL);
		elapsedTime = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));

		if(elapsedTime < 4000)
			usleep(4000 - elapsedTime);

		if(Sensors.readSensors() == 0) {

			for (int i = 0; i < NUM_SENSORS; i++) {

				sTouchNum[i] 	= Sensors.getTKTouchCount(i);
				sTouches[i]		= Sensors.getTKXPositions(i);
				sYTouches[i] 	= Sensors.getTKYPosition(i);
				sSize[i] 		= Sensors.getTKTouchSize(i);

				for (int j = 0; j < MAX_TOUCHES; j++) {
					sTouches_[i][j]	= sTouches[i][j];
					sSize_[i][j] = sSize[i][j];
				}

				if(sTouchNum[i] > 0) {
					
					// If number of touches different from previous round, track order of arrival using distance comparison
					if(sPrevTouchNum[i] != sTouchNum[i]) {
						
						// Max number of current+previous touches between rounds with different number of touches
						float distances[MAX_TOUCHES * (MAX_TOUCHES-1)]; 
						int ids[MAX_TOUCHES * (MAX_TOUCHES-1)];
						
						// Calculate all distance permutations between previous and current touches
						for(int j = 0; j < sTouchNum[i]; j++) {
							
							for(int p = 0; p < sPrevTouchNum[i]; p++) {
								
								// Permutation ID, to indicate between which touches we are calculating distance
								int index = j * sPrevTouchNum[i] + p;
								distances[index] = fabs(sTouches[i][j] - sPrevSortedTouches[i][p]);
								ids[index] = index;
								
								if(index > 0) {
									// Sort, from min to max distance
									float tmp;
									
									while(distances[index] < distances[index-1]) {
										tmp				= ids[index-1];
										ids[index-1]	= ids[index];
										ids[index]		= tmp;
										
										tmp				= distances[index-1];
										distances[index-1] = distances[index];
										distances[index] = tmp;

										index--;

										if(index == 0)
											break;
									}
								}
							}
						}

						int sorted = 0;
						bool currAssigned[MAX_TOUCHES] = {false};
						bool prevAssigned[MAX_TOUCHES] = {false};

						// Track touches, assigning index according to shortest distance
						for(int j = 0; j < sTouchNum[i] * sPrevTouchNum[i]; j++) {
							
							int currentIndex	= ids[j]/sPrevTouchNum[i];
							int prevIndex		= ids[j]%sPrevTouchNum[i];
							
							// Avoid double assignment
							if(!currAssigned[currentIndex] && !prevAssigned[prevIndex]) {
								currAssigned[currentIndex]	= true;
								prevAssigned[prevIndex]		= true;
								sSortedTouchIndices[i][currentIndex] = prevIndex;
								sorted++;
							}
						}
						
						// We still have to assign a free index to new touches
						if(sPrevTouchNum[i] < sTouchNum[i]) {
							
							for(int j = 0; j < sTouchNum[i]; j++) {
								
								if(!currAssigned[j])
									// Assign next free index
									sSortedTouchIndices[i][j] = sorted++;

								// Update tracked value
								sSortedTouches[i][sSortedTouchIndices[i][j]] = sTouches[i][j];
								sPrevSortedTouches[i][j]			         = sSortedTouches[i][j];
								
								if(sSortedTouchIndices[i][j] == sTouchNum[i]-1)
									sLastIndex[i] = j;
							}
						}

						// If some touches disappeared
						else {
							// Shift all the indices
							for(int j = sPrevTouchNum[i] - 1; j >= 0; j--) {
								if(!prevAssigned[j]) {
									for(int k = 0; k < sTouchNum[i]; k++) {
										// Only if touches that disappeared were before the current one
										if(sSortedTouchIndices[i][k] > j)
											sSortedTouchIndices[i][k]--;
									}
								}
							}
							
							for(int j = 0; j < sTouchNum[i]; j++) {
								// Update tracked value
								sSortedTouches[i][sSortedTouchIndices[i][j]] = sTouches[i][j];
								sPrevSortedTouches[i][j]			         = sSortedTouches[i][j];
								if(sSortedTouchIndices[i][j] == sTouchNum[i] - 1)
									sLastIndex[i] = j;
							}
						}
					}
					// If nothing changed since the last round
					else {
						for(int j = 0; j < sTouchNum[i]; j++) {
							// Update tracked value
							sSortedTouches[i][sSortedTouchIndices[i][j]] = sTouches[i][j];
							sPrevSortedTouches[i][j]			         = sSortedTouches[i][j];
						}
					}

					if(sTouchNum[i] == 0)
						sLastIndex[i] = -1;
				}

				if(sTouchNum[i] <= 0)
					sLastIndex[i] = -1;
					

				// Printing loop - To remove once the patch is connected to Pure Data ------------------
				loopIndex++;
				if (loopIndex == 100) {
					loopIndex = 0;
					printf("---\nSENSOR 0:\tTouches: %d\tLast touch:%d\n\t\tX Position:\tTouch Size:\n", sTouchNum[0], sLastIndex[0] + 1);
					for(int j = 0; j < sTouchNum[0]; j++)
						printf("\t\t%f\t%f\n", sSortedTouches[0][j], sSize_[0][j]);
					printf("SENSOR 1:\tTouches: %d\tLast touch: %d\n\t\tX Position:\tTouch Size:\n", sTouchNum[1], sLastIndex[1] + 1);
					for(int j = 0; j < sTouchNum[1]; j++)
						printf("\t\t%f\t%f\n", sSortedTouches[1][j], sSize_[1][j]);
				}
			// End of printing loop ----------------------------------------------------------------

				// Update variables for all sensors
				sPrevTouchNum[i] = sTouchNum[i];
			}
			
		gettimeofday(&start, NULL);
		}
	}
}


// Not sure what this does
void sensorLoop(void *);


// Not sure what this does
struct arg_data {
   int  argc;
   char **argv;
};


// Not sure what this does
arg_data args;


// Parse arguments (not sure what this does)
void parseArguments(arg_data args, BelaInitSettings *settings) {
	Bela_defaultSettings(settings);
}


// Auxiliary task (not sure what this does)
int main(int argc, char *argv[])
{
	BelaInitSettings settings;
	RT_TASK rtSensorThread;
	const char rtSensorThreadName[] = "sensor";
	int oscBankHopSize;

	// Parse command-line arguments
	args.argc = argc;
	args.argv = argv;
	parseArguments(args, &settings);

	// Initialise the audio device
	if(Bela_initAudio(&settings, &oscBankHopSize) != 0)
		return -1;

	if(initSensorLoop() != 0)
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
// DONE --------------------------------------------------------------------------