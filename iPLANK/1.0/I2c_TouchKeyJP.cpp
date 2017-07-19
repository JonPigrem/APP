/*
 * I2c_TouchKey.cpp
 *
 *  Created on: Oct 14, 2013
 *      Author: Victor Zappi
 */



#include "I2c_TouchKeyJP.h"

#undef DEBUG_I2C_TOUCHKEY

I2c_TouchKeyJP::I2c_TouchKeyJP()
{
	isReady = false;
	sensorType = kSensorTypeTouchKey;
	touchCount = 0;
	sliderSize[0] = sliderSize[1] = sliderSize[2] = -1;
	sliderPosition[0] = sliderPosition[1] = sliderPosition[2] = -1;
	sliderPositionH = -1;
}

int I2c_TouchKeyJP::initTouchKey(int sensorTypeToUse)
{
	sensorType = sensorTypeToUse;
	if(sensorType > 2 || sensorType < 0)
		sensorType = 2;

	numBytesToRead = kSensorBytes[sensorType];

	char buf[3] = { 0x00, 0x01, 0x00 }; // code for centroid mode
	if(write(i2C_file, buf, 3) !=3)
	{
		cout << "Failed to set TouchKey in \"Centroid Mode\" " << endl;
		return 1;
	}

	usleep(5000); // need to give TouchKey enough time to process command

	char buf4[4] = { 0x00, 0x07, 0x00, 0x64}; // code for change minimum touch area
	if(write(i2C_file, buf4, 4) !=4)
	{
		cout << "Failed to set TouchKey minimum touch size" << endl;
		return 1;
	}

	usleep(5000); // need to give TouchKey enough time to process command

	if(sensorType == kSensorTypeDBox2)
		buf[0] = 0x04; // code for data collection
	else
		buf[0] = 0x06; // code for data collection

	if(write(i2C_file, buf, 1) !=1)
	{
		cout << "Failed to prepare data collection " << endl;
		return 2;
	}

	usleep(5000); // need to give TouchKey enough time to process command

	isReady = true;

	return 0;
}


int I2c_TouchKeyJP::readI2C()
{
	bytesRead = read(i2C_file, dataBuffer, numBytesToRead);
	if (bytesRead != numBytesToRead)
	{
		cout << "Failure to read Byte Stream" << endl;
		return 2;
	}
	/*cout << NUM_BYTES << " bytes read" << endl;
	for(int j=0; j<9; j++)
		cout << "\t" << (int)dataBuffer[j];
	cout << endl;
	*/

	touchCount = 0;

	// Old TouchKeys sensors have 3 touch locations plus horizontal positions
	// New D-Box sensors have 5 touch locations but no horizontal position
	// Later D-Box sensors have same data in a different format
	if(sensorType == kSensorTypeDBox1) {
		rawSliderPosition[0] = (((dataBuffer[0] & 0xF0) << 4) + dataBuffer[1]);
		rawSliderPosition[1] = (((dataBuffer[0] & 0x0F) << 8) + dataBuffer[2]);
		rawSliderPosition[2] = (((dataBuffer[3] & 0xF0) << 4) + dataBuffer[4]);
		rawSliderPosition[3] = (((dataBuffer[5] & 0xF0) << 4) + dataBuffer[6]);
		rawSliderPosition[4] = (((dataBuffer[5] & 0x0F) << 8) + dataBuffer[7]);
		rawSliderPositionH = 0x0FFF;
	}
	else if(sensorType == kSensorTypeDBox2) {                                     
		rawSliderPosition[0] = ((dataBuffer[0] << 8) + dataBuffer[1]) & 0x0FFF;
		rawSliderPosition[1] = ((dataBuffer[2] << 8) + dataBuffer[3]) & 0x0FFF;
		rawSliderPosition[2] = ((dataBuffer[4] << 8) + dataBuffer[5]) & 0x0FFF;
		rawSliderPosition[3] = ((dataBuffer[6] << 8) + dataBuffer[7]) & 0x0FFF;
		rawSliderPosition[4] = ((dataBuffer[16] << 8) + dataBuffer[17]) & 0x0FFF;
		rawSliderPosition[5] = ((dataBuffer[18] << 8) + dataBuffer[19]) & 0x0FFF;
		rawSliderPosition[6] = ((dataBuffer[20] << 8) + dataBuffer[21]) & 0x0FFF;
		rawSliderPosition[7] = ((dataBuffer[22] << 8) + dataBuffer[24]) & 0x0FFF;
		
		//prints raw slider values
		/*
		printf("\nrawX1: %i  ", rawSliderPosition[0]);
		printf("rawX2: %i  ", rawSliderPosition[1]);
		printf("rawX3: %i  ", rawSliderPosition[2]);
		printf("rawX4: %i  ", rawSliderPosition[3]);
		printf("rawY1: %i  ", rawSliderPosition[4]);
		printf("rawY2: %i  ", rawSliderPosition[5]);
		printf("rawY3: %i  ", rawSliderPosition[6]);
		printf("rawY4: %i  ", rawSliderPosition[7]);
		*/

	}
	else {
		rawSliderPosition[0] = (((dataBuffer[0] & 0xF0) << 4) + dataBuffer[1]);
		rawSliderPosition[1] = (((dataBuffer[0] & 0x0F) << 8) + dataBuffer[2]);
		rawSliderPosition[2] = (((dataBuffer[3] & 0xF0) << 4) + dataBuffer[4]);
		rawSliderPosition[3] = 0x0FFF;
		rawSliderPosition[4] = 0x0FFF;
		rawSliderPositionH   = (((dataBuffer[3] & 0x0F) << 8) + dataBuffer[5]);
	}

	for(int i = 0; i < 4; i++)
	{
		if(rawSliderPosition[i] != 0x0FFF) // 0x0FFF means no touch
		{
				sliderPosition[i] = (float)rawSliderPosition[i] / 1792.0;  
				sliderPositionY[i] = (float)rawSliderPosition[i+4] / 1792.0;  
	
				
		//prints slider values form 0.0-1.0
		printf("\naccX1: %f  ", sliderPosition[0]);
		printf("accX2: %f  ", sliderPosition[1]);
		printf("accX3: %f  ", sliderPosition[2]);
		printf("accX4: %f  ", sliderPosition[3]);
		printf("accY1: %f  ", sliderPositionY[0]);
		printf("accY2: %f  ", sliderPositionY[1]);
		printf("accY3: %f  ", sliderPositionY[2]);
		printf("accY4: %f  ", sliderPositionY[3]);


			if(sliderPosition[i] > 1.0)
				sliderPosition[i] = 1.0;
				
		//Slider Sizer
		sliderSize[0] = (float)((dataBuffer[8] << 8) + dataBuffer[9]) / 5000.0;
		sliderSize[1] = (float)((dataBuffer[10] << 8) + dataBuffer[11]) / 5000.0;
		sliderSize[2] = (float)((dataBuffer[12] << 8) + dataBuffer[13]) / 5000.0;
		sliderSize[3] = (float)((dataBuffer[14] << 8) + dataBuffer[15]) / 5000.0;
		
		sliderSizeY[0] = (float)((dataBuffer[24] << 8) + dataBuffer[25]) / 5000.0;
		sliderSizeY[1] = (float)((dataBuffer[26] << 8) + dataBuffer[27]) / 5000.0;
		sliderSizeY[2] = (float)((dataBuffer[28] << 8) + dataBuffer[29]) / 5000.0;
		sliderSizeY[3] = (float)((dataBuffer[30] << 8) + dataBuffer[31]) / 5000.0;
		
		printf("Touch1X: %f  ", sliderSize[0]);
		printf("Touch2X: %f  ", sliderSize[1]);
		printf("Touch3X: %f  ", sliderSize[2]);
		printf("Touch4X: %f  ", sliderSize[3]);
		
		printf("Touch1Y: %f  ", sliderSizeY[0]);
		printf("Touch2Y: %f  ", sliderSizeY[1]);
		printf("Touch3Y: %f  ", sliderSizeY[2]);
		printf("Touch4Y: %f  ", sliderSizeY[3]);

/*
			if(sensorType == kSensorTypeDBox2) {
				sliderSize[i]     = (float)((dataBuffer[2*i + 16] << 8) + dataBuffer[2*i + 17]) / 5000.0;
				if(sliderSize[i] > 1.0)
					sliderSize[i] = 1.0;
			}
			else if(sensorType == kSensorTypeDBox1)
				sliderSize[i]     = (float)dataBuffer[i + 8] / 255.0;
			else {
				if(i < 3)
					sliderSize[i]     = (float)dataBuffer[i + 6] / 255.0;
				else
					sliderSize[i]     = 0.0;
			}
			
			
*/			
			touchCount++;
			
			//printf("touchcount: %i  ", touchCount);
		}
		else {
			sliderPosition[i] = -1.0;
			sliderSize[i]     = 0.0;
		}
	}

	

	if(rawSliderPositionH != 0x0FFF)
	{
		sliderPositionH = (float)rawSliderPositionH / 256.0;			// White keys, horizontal (1 byte + 1 bit)
	}
	else
		sliderPositionH = -1.0;

#ifdef DEBUG_I2C_TOUCHKEY
	for(int i = 0; i < bytesRead; i++) {
		printf("%2X ", dataBuffer[i]);
	}
	cout << touchCount << " touches: ";
	for(int i = 0; i < touchCount; i++) {
		cout << "(" << sliderPosition[i] << ", " << sliderSize[i] << ") ";
	}
	cout << "H = " << sliderPositionH << endl;
#endif

	return 0;
}


I2c_TouchKeyJP::~I2c_TouchKeyJP()
{}

