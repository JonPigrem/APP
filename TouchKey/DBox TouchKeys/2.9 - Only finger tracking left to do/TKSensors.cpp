// Change the settings in TKSensors.h before starting the patch
// ------------------------------------------------------------

#include "TKSensors.h"


// TO DO ----------------------------------------------------------------------
// rework this to figure out which data we exactly want
float gSensor0LatestTouchPos = 0;	// most recent pitch touch location [0-1] on sensor 0, used by render.cpp
int gSensor0LatestTouchNum	 = 0;	// most recent num of touches on sensor 0, used by render.cpp
int gSensor0LatestTouchIndex = 0;	// index of last touch in gSensor0LatestTouchPos[5], used by render.cpp
float gSensor1LatestTouchPos[5];	// most recent touche locations on sensor 1, used by render.cpp
int gSensor1LatestTouchCount;		// most recent number touches on sensor 1, used by render.cpp
int gSensor1LatestTouchIndex = 0;	// index of last touch in gSensor1LatestTouchPos[5], used by render.cpp
int loopIndex = 0;
// TO DO ----------------------------------------------------------------------




// DONE ----------------------------------------------------------------------
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
	int tk_bus[NUM_SENSORS]			; 
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
// DONE ----------------------------------------------------------------------



// TO DO ----------------------------------------------------------------------
void sensorLoop(void *)
{
	timeval start, end;
	unsigned long elapsedTime;
	//float touchSize		= 0;	// once used for timbre

	float *s0Touches;
	float s0YTouches;
	float *s0Size;
	int s0PrevTouchNum 	= 0;
	int s0SortedTouchIndices[MAX_TOUCHES];
	float s0SortedTouches[MAX_TOUCHES];
	float s0PrevSortedTouches[MAX_TOUCHES];

	float *s1Touches;
	float s1YTouches;
	float *s1Size;
	int s1PrevTouchNum 	= 0;
	int s1SortedTouchIndices[MAX_TOUCHES];
	float s1SortedTouches[MAX_TOUCHES];
	float s1PrevSortedTouches[MAX_TOUCHES];


	// init time vals
	gettimeofday(&start, NULL);

	// here we go, sensor loop until the end of the application
	while(!gShouldStop)
	{
		gettimeofday(&end, NULL);
		elapsedTime = ( (end.tv_sec*1000000+end.tv_usec) - (start.tv_sec*1000000+start.tv_usec) );
		if( elapsedTime<4000 )
			usleep(4000-elapsedTime);

		if(Sensors.readSensors()==0)
		{
			sTouchNum[0]	= Sensors.getTKTouchCount(0);
			s0Touches	= Sensors.getTKXPositions(0);
			s0YTouches	= Sensors.getTKYPosition(0);
			s0Size 		= Sensors.getTKTouchSize(0);

			sTouchNum[1]	= Sensors.getTKTouchCount(1);
			s1Touches	= Sensors.getTKXPositions(1);
			s1YTouches	= Sensors.getTKYPosition(1);
			s1Size 		= Sensors.getTKTouchSize(1);

			for(int i=0; i<MAX_TOUCHES; i++)
			{
				sTouches_[0][i]	= s0Touches[i];
				sSize_[0][i]		= s0Size[i];

				sTouches_[1][i]	= s1Touches[i];
				sSize_[1][i]		= s1Size[i];
			}

			gSensor0LatestTouchNum	= sTouchNum[0];
			if(sTouchNum[0] > 0)
			{
				//touchSize	 = 0;	\\ once used for timbre

				// if we have a number of touches different from previous round, track their order of arrival [calculated using distance comparison]
				if(s0PrevTouchNum!=sTouchNum[0])
				{
					float distances[MAX_TOUCHES*(MAX_TOUCHES-1)]; // maximum number of current+previous touches between rounds with different num of touches
					int ids[MAX_TOUCHES*(MAX_TOUCHES-1)];
					// calculate all distance permutations between previous and current touches
					for(int i=0; i<sTouchNum[0]; i++)
					{
						for(int p=0; p<s0PrevTouchNum; p++)
						{
							int index			= i*s0PrevTouchNum+p;	// permutation id [says between which touches we are calculating distance]
							distances[index]	= fabs(s0Touches[i]-s0PrevSortedTouches[p]);
							ids[index]			= index;
							if(index>0)
							{
								// sort, from min to max distance
								float tmp;
								while(distances[index]<distances[index-1])
								{
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

					// track touches assigning index according to shortest distance
					for(int i=0; i<sTouchNum[0]*s0PrevTouchNum; i++)
					{
						int currentIndex	= ids[i]/s0PrevTouchNum;
						int prevIndex		= ids[i]%s0PrevTouchNum;
						// avoid double assignment
						if(!currAssigned[currentIndex] && !prevAssigned[prevIndex])
						{
							currAssigned[currentIndex]	= true;
							prevAssigned[prevIndex]		= true;
							s0SortedTouchIndices[currentIndex] = prevIndex;
							sorted++;
						}
					}
					// we still have to assign a free index to new touches
					if(s0PrevTouchNum<sTouchNum[0])
					{
						for(int i=0; i<sTouchNum[0]; i++)
						{
							if(!currAssigned[i])
								s0SortedTouchIndices[i] = sorted++; // assign next free index

							// update tracked value
							s0SortedTouches[s0SortedTouchIndices[i]] = s0Touches[i];
							s0PrevSortedTouches[i]			         = s0SortedTouches[i];
							if(s0SortedTouchIndices[i]==sTouchNum[0]-1)
								sLastIndex[0] = i;

							// accumulate sizes for timbre
							//touchSize += s0Size[i];
						}
					}
					else // some touches have disappeared...
					{
						// ...we have to shift all indices...
						for(int i=s0PrevTouchNum-1; i>=0; i--)
						{
							if(!prevAssigned[i])
							{
								for(int j=0; j<sTouchNum[0]; j++)
								{
									// ...only if touches that disappeared were before the current one
									if(s0SortedTouchIndices[j]>i)
										s0SortedTouchIndices[j]--;
								}
							}
						}
						// done! now update
						for(int i=0; i<sTouchNum[0]; i++)
						{
							// update tracked value
							s0SortedTouches[s0SortedTouchIndices[i]] = s0Touches[i];
							s0PrevSortedTouches[i]			         = s0SortedTouches[i];
							if(s0SortedTouchIndices[i]==sTouchNum[0]-1)
								sLastIndex[0] = i;

							// accumulate sizes for timbre
							//touchSize += s0Size[i];
						}
					}
				}
				else // nothing's changed since last round
				{
					for(int i=0; i<sTouchNum[0]; i++)
					{
						// update tracked value
						s0SortedTouches[s0SortedTouchIndices[i]] = s0Touches[i];
						s0PrevSortedTouches[i]			         = s0SortedTouches[i];

						// accumulate sizes for timbre
						//touchSize += s0Size[i];
					}
				}

				if(sTouchNum[0] == 0)
					sLastIndex[0] = -1;


				gSensor0LatestTouchPos      = s0SortedTouches[sTouchNum[0]-1];
			
			}


			//-----------------------------------------------------------------------------------
			// sort touches on sensor 2
			if(sTouchNum[1] > 0)
			{
				// if we have a number of touches different from previous round, track their order of arrival [calculated using distance comparison]
				if(s1PrevTouchNum!=sTouchNum[1])
				{
					float distances[MAX_TOUCHES*(MAX_TOUCHES-1)]; // maximum number of current+previous touches between rounds with different num of touches
					int ids[MAX_TOUCHES*(MAX_TOUCHES-1)];
					// calculate all distance permutations between previous and current touches
					for(int i=0; i<sTouchNum[1]; i++)
					{
						for(int p=0; p<s1PrevTouchNum; p++)
						{
							int index 			= i*s1PrevTouchNum+p;	// permutation id [says between which touches we are calculating distance]
							distances[index]	= fabs(s1Touches[i]-s1PrevSortedTouches[p]);
							ids[index]			= index;
							if(index>0)
							{
								// sort, from min to max distance
								float tmp;
								while(distances[index]<distances[index-1])
								{
									tmp 				= ids[index-1];
									ids[index-1] 		= ids[index];
									ids[index] 			= tmp;

									tmp					= distances[index-1];
									distances[index-1]	= distances[index];
									distances[index] 	= tmp;

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

					// track touches assigning index according to shortest distance
					for(int i=0; i<sTouchNum[1]*s1PrevTouchNum; i++)
					{
						int currentIndex	= ids[i]/s1PrevTouchNum;
						int prevIndex		= ids[i]%s1PrevTouchNum;
						// avoid double assignment
						if(!currAssigned[currentIndex] && !prevAssigned[prevIndex])
						{
							currAssigned[currentIndex]			= true;
							prevAssigned[prevIndex]				= true;
							s1SortedTouchIndices[currentIndex] = prevIndex;
							sorted++;
						}
					}
					// we still have to assign a free index to new touches
					if(s1PrevTouchNum<sTouchNum[1])
					{
						for(int i=0; i<sTouchNum[1]; i++)
						{
							if(!currAssigned[i])
								s1SortedTouchIndices[i] = sorted++; // assign next free index

							// update tracked value
							s1SortedTouches[s1SortedTouchIndices[i]] = s1Touches[i];
							s1PrevSortedTouches[i]			       	 = s1SortedTouches[i];
							if(s1SortedTouchIndices[i]==sTouchNum[1]-1)
								sLastIndex[1] = i;
						}
					}
					else // some touches have disappeared...
					{
						// ...we have to shift all indices...
						for(int i=s1PrevTouchNum-1; i>=0; i--)
						{
							if(!prevAssigned[i])
							{
								for(int j=0; j<sTouchNum[1]; j++)
								{
									// ...only if touches that disappeared were before the current one
									if(s1SortedTouchIndices[j]>i)
										s1SortedTouchIndices[j]--;
								}
							}
						}
						// done! now update
						for(int i=0; i<sTouchNum[1]; i++)
						{
							// update tracked value
							s1SortedTouches[s1SortedTouchIndices[i]] = s1Touches[i];
							s1PrevSortedTouches[i]			       	 = s1SortedTouches[i];
							if(s1SortedTouchIndices[i]==sTouchNum[1]-1)
								sLastIndex[1] = i;
						}
					}
				}
				else // nothing's changed since last round
				{
					for(int i=0; i<sTouchNum[1]; i++)
					{
						// update tracked value
						s1SortedTouches[s1SortedTouchIndices[i]] = s1Touches[i];
						s1PrevSortedTouches[i]			       	 = s1SortedTouches[i];
					}
				}
			}

			if(sTouchNum[0] > 0)
			{
				gSensor0LatestTouchIndex = sLastIndex[0];
			}
			else
				sLastIndex[0] = -1;
				
			if(sTouchNum[1] > 0)
			{
				gSensor1LatestTouchIndex = sLastIndex[1];
			}
			else
				sLastIndex[1] = -1;

			// the loop index makes sure we don't print too often
			loopIndex++;
			if (loopIndex == 100) {
				loopIndex = 0;
				printf("---\nSENSOR 0:\tTouches: %d\tLast touch:%d\n\t\tX Position:\tTouch Size:\n", sTouchNum[0], gSensor0LatestTouchIndex + 1);
				for(int i=0; i<sTouchNum[0]; i++)
					printf("\t\t%f\t%f\n", s0SortedTouches[i], sSize_[0][i]);
				printf("SENSOR 1:\tTouches: %d\tLast touch: %d\n\t\tX Position:\tTouch Size:\n", sTouchNum[1], gSensor1LatestTouchIndex + 1);
				for(int i=0; i<sTouchNum[1]; i++)
					printf("\t\t%f\t%f\n", s1SortedTouches[i], sSize_[1][i]);
			}
		


			// update variables for both sensors
			s0PrevTouchNum	= sTouchNum[0];
			s1PrevTouchNum	= sTouchNum[1];
		}
		else
		//	rt_printf("Come on instrument!\n");	//break

		gettimeofday(&start, NULL);
	}

}
// TO DO ----------------------------------------------------------------------





// DONE ----------------------------------------------------------------------
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