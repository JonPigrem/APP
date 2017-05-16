/*****************************
 * CapSlider library for Arduino
 * (c) 2015 Andrew McPherson 
 *
 * This library communicates with the CapSlider sensors
 * using I2C.
 *
 * BSD license
 */

#include "CapSlider.h"

#define MAX_TOUCH_1D_OR_2D ((device_type_ == CAPSLIDER_DEVICE_2D ? kMaxTouchNum2D : kMaxTouchNum1D))

CapSlider::CapSlider(uint8_t i2c_address, uint8_t reset_pin)
: i2c_address_(i2c_address), reset_pin_(reset_pin),
  device_type_(CAPSLIDER_DEVICE_NONE), firmware_version_(0),
  mode_(0xFF), last_read_loc_(0xFF), num_touches_(0),
  raw_bytes_left_(0)
{	
}

/* Initialise the hardware. Returns the type of device attached, or 0
   if none is attached. */
int CapSlider::begin() {
	/* De-assert reset if it is being used */
	if(reset_pin_ != 0) {
		pinMode(reset_pin_, OUTPUT);
		digitalWrite(reset_pin_, LOW);
	}
	
	/* Start I2C */
	Wire.begin();
	
	/* Put the device (assuming it exists) in normal mode */
	setMode(CAPSLIDER_MODE_NORMAL);
	
	/* Wait to process the command before sending the second identify command */
	delay(25);
	
	/* Check the type of device attached */
	return identify();
}

/* Return the type of device attached, or 0 if none is attached. */
int CapSlider::identify() {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandIdentify);
	Wire.endTransmission();
	
	/* Give CapSlider time to process this command */
	delay(25);	
	
	last_read_loc_ = kOffsetCommand;
	Wire.requestFrom(i2c_address_, (uint8_t)3);
	
	if(Wire.available() < 3) {
		/* Unexpected or no response; no valid device connected */
		device_type_ = CAPSLIDER_DEVICE_NONE;
		firmware_version_ = 0;
		return device_type_;
	}
	
	Wire.read();	// Discard first input
	device_type_ = Wire.read();
	firmware_version_ = Wire.read();
	
	return device_type_;
}

/* Return the type of device attached gathered from a previous call to identify() */
int CapSlider::deviceType() {
	return device_type_;
}

/* Read the latest scan value from the sensor. Returns true on success. */
boolean CapSlider::read() {
	uint8_t loc = 0;
	uint8_t length = kNormalLengthDefault;
		
	/* Set the read location to the right place if needed */
	prepareForDataRead();
	
	if(device_type_ == CAPSLIDER_DEVICE_2D)
		length = kNormalLength2D;
	
	Wire.requestFrom(i2c_address_, length);
	while(Wire.available()) {
		buffer_[loc++] = Wire.read();
	}
	
	/* Check for read error */
	if(loc < length) {
		num_touches_ = 0;
		return false;
	}
	
	/* Look for the first instance of 0xFFFF (no touch)
   	   in the buffer */
	for(loc = 0; loc < MAX_TOUCH_1D_OR_2D; loc++) {
		if(buffer_[2 * loc] == 0xFF && buffer_[2 * loc + 1] == 0xFF)
			break;
	}
	num_touches_ = loc;
	
	if(device_type_ == CAPSLIDER_DEVICE_2D) {
		/* Look for the number of horizontal touches in 2D sliders
		   which might be different from number of vertical touches */
		for(loc = 0; loc < kMaxTouchNum2D; loc++) {
			if(buffer_[2 * loc + 4 * kMaxTouchNum2D] == 0xFF && 
			   buffer_[2 * loc + 4 * kMaxTouchNum2D + 1] == 0xFF)
				break;
		}
		num_touches_ |= (loc << 4);
	}
	
	return true;
}

/* Update the baseline value on the sensor */
void CapSlider::updateBaseline() {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandBaselineUpdate);
	Wire.endTransmission();
	
	last_read_loc_ = kOffsetCommand;	
}

/* Reset the sensor, if reset pin is enabled */
void CapSlider::reset() {
	if(reset_pin_ == 0)
		return;
	/* Assert reset, hold for 10us, release */
	digitalWrite(reset_pin_, HIGH);
	delayMicroseconds(10);
	digitalWrite(reset_pin_, LOW);
}

/* How many touches? < 0 means error. */
int CapSlider::numberOfTouches() {
	if(mode_ != CAPSLIDER_MODE_NORMAL)
		return 0;
	
	/* Lower 4 bits hold number of 1-axis or vertical touches */
	return (num_touches_ & 0x0F);
}

/* How many horizontal touches for 2D? */
int CapSlider::numberOfHorizontalTouches() {
	if(mode_ != CAPSLIDER_MODE_NORMAL)
		return 0;
	if(device_type_ != CAPSLIDER_DEVICE_2D)
		return 0;
	
	/* Upper 4 bits hold number of horizontal touches */
	return (num_touches_ >> 4);
}

/* Location and size of a particular touch, ranging from 0 to N-1.
   Returns -1 if no such touch exists. */
int CapSlider::touchLocation(uint8_t touch_num) {
	int result;

	if(mode_ != CAPSLIDER_MODE_NORMAL)
		return -1;	
	if(touch_num >= MAX_TOUCH_1D_OR_2D)
		return -1;
	
	result = buffer_[2*touch_num] * 256;
	result += buffer_[2*touch_num + 1];
	
	return result;
}

int CapSlider::touchSize(uint8_t touch_num) {
	int result;

	if(mode_ != CAPSLIDER_MODE_NORMAL)
		return -1;	
	if(touch_num >= MAX_TOUCH_1D_OR_2D)
		return -1;
	
	result = buffer_[2*touch_num + 2*MAX_TOUCH_1D_OR_2D] * 256;
	result += buffer_[2*touch_num + 2*MAX_TOUCH_1D_OR_2D + 1];
	
	return result;
}

/* These methods for horizontal touches on 2D sliders */
int CapSlider::touchHorizontalLocation(uint8_t touch_num) {
	int result;

	if(mode_ != CAPSLIDER_MODE_NORMAL)
		return -1;	
	if(device_type_ != CAPSLIDER_DEVICE_2D)
		return -1;
	if(touch_num >= kMaxTouchNum2D)
		return -1;
	
	result = buffer_[2*touch_num + 4*kMaxTouchNum2D] * 256;
	result += buffer_[2*touch_num + 4*kMaxTouchNum2D + 1];
	
	return result;
}

int CapSlider::touchHorizontalSize(uint8_t touch_num) {
	int result;

	if(mode_ != CAPSLIDER_MODE_NORMAL)
		return -1;	
	if(device_type_ != CAPSLIDER_DEVICE_2D)
		return -1;
	if(touch_num >= kMaxTouchNum2D)
		return -1;
	
	result = buffer_[2*touch_num + 6*kMaxTouchNum2D] * 256;
	result += buffer_[2*touch_num + 6*kMaxTouchNum2D + 1];
	
	return result;
}

/* Request raw data; wrappers for Wire */
void CapSlider::requestRawData(uint8_t max_length) {
	uint8_t length;
	
	prepareForDataRead();
	
	if(max_length == 0xFF) {
		/* Use default for the particular device if no length specified */
		if(device_type_ == CAPSLIDER_DEVICE_1D)
			length = kRawLength1D;
		else if(device_type_ == CAPSLIDER_DEVICE_2D)
			length = kRawLength2D;
		else
			length = kRawLengthDefault;
	}
	if(length > kRawLengthMax)
		length = kRawLengthMax;
	
	/* The raw data might be longer than the Wire.h maximum buffer 
	 * (BUFFER_LENGTH in Wire.h).
	 * If so, split it into two reads. */
	if(length <= BUFFER_LENGTH) {
		Wire.requestFrom(i2c_address_, length);
		raw_bytes_left_ = 0;
	}
	else {
		Wire.requestFrom(i2c_address_, (uint8_t)BUFFER_LENGTH);
		raw_bytes_left_ = length - BUFFER_LENGTH;
	}
}

int CapSlider::rawDataAvailable() {
	/* Raw data items are 2 bytes long; return number of them available */
	return ((Wire.available() + raw_bytes_left_) >> 1);
}

/* Raw data is in 16-bit big-endian format */
int CapSlider::rawDataRead() {
	int result;
	
	if(Wire.available() < 2) {
		/* Read more bytes if we need it */
		if(raw_bytes_left_ > 0) {
			/* Move read pointer on device */
			Wire.beginTransmission(i2c_address_);
			Wire.write(kOffsetData + BUFFER_LENGTH);
			Wire.endTransmission();
			last_read_loc_ = kOffsetData + BUFFER_LENGTH;
			
			/* Now gather what's left */
			Wire.requestFrom(i2c_address_, raw_bytes_left_);
			raw_bytes_left_ = 0;		
		}

		/* Check again if we've got anything... */
		if(Wire.available() < 2)
			return 0;
	}
	
	result = (int)Wire.read() * 256;
	result += (int)Wire.read();
	return result;
}

/* Scan configuration settings */
void CapSlider::setMode(uint8_t mode) {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandMode);
	Wire.write(mode);
	Wire.endTransmission();
	
	mode_ = mode;
	last_read_loc_ = kOffsetCommand;		
}

void CapSlider::setScanSettings(uint8_t speed, uint8_t num_bits) {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandScanSettings);
	Wire.write(speed);
	Wire.write(num_bits);
	Wire.endTransmission();
	
	last_read_loc_ = kOffsetCommand;		
}

void CapSlider::setPrescaler(uint8_t prescaler) {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandPrescaler);
	Wire.write(prescaler);
	Wire.endTransmission();
	
	last_read_loc_ = kOffsetCommand;			
}

void CapSlider::setNoiseThreshold(uint8_t threshold) {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandNoiseThreshold);
	Wire.write(threshold);
	Wire.endTransmission();
	
	last_read_loc_ = kOffsetCommand;		
}

void CapSlider::setIDACValue(uint8_t value) {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandIdac);
	Wire.write(value);
	Wire.endTransmission();
	
	last_read_loc_ = kOffsetCommand;		
}

void CapSlider::setMinimumTouchSize(uint16_t size) {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandMinimumSize);
	Wire.write(size >> 8);
	Wire.write(size & 0xFF);
	Wire.endTransmission();
	
	last_read_loc_ = kOffsetCommand;		
}

/* Prepare the device to read data if it is not already prepared */
void CapSlider::prepareForDataRead() {
	if(last_read_loc_ != kOffsetData) {
		Wire.beginTransmission(i2c_address_);
		Wire.write(kOffsetData);
		Wire.endTransmission();

		last_read_loc_ = kOffsetData;
	}		
}
