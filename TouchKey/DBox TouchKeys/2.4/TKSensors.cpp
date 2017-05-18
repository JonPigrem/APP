#include <math.h>
#include "TKSensors.h"
#include <native/task.h>
#include <Bela.h>

// Change these settings here and in TKsensors.h depending on each specific setup
// -----------------------------------------------------------------------------------------------
int sensorType = 2; 			// Type of sensors used (default = 2)
int sensorAddress[NUM_SENSORS] = {0x18, 0x18};	// I2c addresses for each sensor
// -----------------------------------------------------------------------------------------------

// rework this to figure out which data we exactly want
float gSensor0LatestTouchPos = 0;	// most recent pitch touch location [0-1] on sensor 0, used by render.cpp
int gSensor0LatestTouchNum	 = 0;	// most recent num of touches on sensor 0, used by render.cpp
int gSensor0LatestTouchIndex = 0;	// index of last touch in gSensor0LatestTouchPos[5], used by render.cpp
float gSensor1LatestTouchPos[5];	// most recent touche locations on sensor 1, used by render.cpp
int gSensor1LatestTouchCount;		// most recent number touches on sensor 1, used by render.cpp
int gSensor1LatestTouchIndex = 0;	// index of last touch in gSensor1LatestTouchPos[5], used by render.cpp
int loopIndex = 0;

// don't touch this
TKSensors Sensors;
extern int gShouldStop;

// variables for touchkey
int sTouchNum[NUM_SENSORS];
float sTouches_[NUM_SENSORS][MAX_TOUCHES];
float sSize_[NUM_SENSORS][MAX_TOUCHES];
int sLastIndex[NUM_SENSORS];

// initialise touchkey sensors
int TKSensors::initSensors(int tk_bus, int tk0_address, int tk1_address, int tk_file, int sensorTypeToUse)
{
	sensorType = sensorTypeToUse;
	// init first touch key on i2c bus
	if(tk0_address >= 0) {
		if(TK[0].initI2C_RW(tk_bus, tk0_address, tk_file)>0)
			return 1;
		if(TK[0].initTouchKey(sensorType)>0)
			return 2;
	}

	// init second touch key on i2c bus
	if(tk1_address >= 0) {
		if(TK[1].initI2C_RW(tk_bus, tk1_address, tk_file)>0)
			return 1;
		if(TK[1].initTouchKey(sensorType)>0)
			return 2;
	}

	return 0;
}

// this records touch position Y, as well as touch position X and touch size for each finger
int TKSensors::readSensors()
{
	// identify if first touch key is used
	if(TK[0].ready()) {
		if(TK[0].readI2C()>0)
			return 1;
		// retrieve data from first touch key
		tk_touchCnt[0] = TK[0].getTouchCount();
	}
	else
		tk_touchCnt[0] = 0;


	// identify if second touch key is used
	if(TK[1].ready()) {
		if(TK[1].readI2C()>0)
			return 1;
		// retrieve data from second touch key
		tk_touchCnt[1] = TK[1].getTouchCount();
	}
	else
		tk_touchCnt[1] = 0;


	// if no touchkey is used, reset sensor data
	if(tk_touchCnt[0] == 0 && tk_touchCnt[1] == 0)
		resetSensorsData();
	// but if any touchkey is used, record data
	else
	{
		// touch position X and touch size for each finger
		for(int i=0; i<MAX_TOUCHES; i++)
		{
			tk_touchPosX[0][i] = TK[0].getSliderPosition()[i];
			tk_touchSize[0][i] = TK[0].getSlidersize()[i];

			tk_touchPosX[1][i] = TK[1].getSliderPosition()[i];
			tk_touchSize[1][i] = TK[1].getSlidersize()[i];
		}
		// touch position Y
		tk_touchPosY[0] 	 = TK[0].getSliderPositionH();
		tk_touchPosY[1] 	 = TK[1].getSliderPositionH();
	}
	
	return 0;
}

// this describes what data is reset when no touches are recorded
TKSensors::TKSensors()
{
	resetSensorsData();
}

void TKSensors::resetSensorsData()
{
	for(int i=0; i<MAX_TOUCHES; i++)
	{
		tk_touchPosX[0][i] = -1;
		tk_touchPosY[0]	 = -1;
		tk_touchSize[0][i] = 0;

		tk_touchPosX[1][i] = -1;
		tk_touchPosY[1]	 = -1;
		tk_touchSize[1][i] = 0;
	}
	return;
}



// record number of touches and touch size for each finger
int initSensorLoop(int sensorAddress0, int sensorAddress1, int sensorType)
{
	int tk_bus[NUM_SENSORS]			= {1, 1}; // NEED TO FIND A WAY TO DO THIS BETTER
	int tk0_address		= sensorAddress0;
	int tk1_address		= sensorAddress1;
	int tk_file			= 0;

	for (int i = 0; i<NUM_SENSORS; i++) 
	{
		if(Sensors.initSensors(tk_bus[i], tk0_address, tk1_address, tk_file, sensorType)>0)
		{
			gShouldStop = 1;
			cout << "control cannot start" << endl;
			return -1;
		}
	}

	for(int i=0; i<MAX_TOUCHES; i++)
	{
		sTouches_[0][i]	= 0.0;
		sSize_[0][i]		= 0.0;

		sTouches_[1][i]	= 0.0;
		sSize_[1][i]	= 0.0;
	}

	return 0;
}


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

	if(initSensorLoop(sensorAddress[0], sensorAddress[1], sensorType) != 0)
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