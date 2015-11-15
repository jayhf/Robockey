#include "../Robockey/ADC.h"

bool updateCompleted;

Resistor irResistor;
uint8_t irIndex;
uint16_t irValues[16];
uint16_t battery;
uint16_t boost;
uint16_t leftMotor;
uint16_t rightMotor;
uint16_t switchValue;

bool adcUpdateCompleted(){
	return updateCompleted;
}

void initADC(){}

void beginADC(){}

bool batteryLow(){
	return battery < 450;
}

bool boostReady(){
	return boost > 800;
}

uint16_t leftMotorCurrent(){
	return (leftMotor * 147) >> 3;
}

uint16_t rightMotorCurrent(){
	return (rightMotor * 147) >> 3;
}

SwitchPosition switchPosition(){
	if(switchValue>768)
		return SwitchPosition::DOWN;
	else if(switchValue>256)
		return SwitchPosition::MIDDLE;
	else
		return SwitchPosition::UP;
};

Resistor getSelectedResistor(){
	return irResistor;
};

uint16_t* getIRData(){
	return irValues;
}
