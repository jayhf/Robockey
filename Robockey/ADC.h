#pragma once

#include <stdint.h>

void initADC();
void beginADC();
bool adcUpdateCompleted();

//Threshold is just under 3.1V
bool batteryLow();
//Threshold is over 29V
bool boostReady();
//Current values are in mA
uint16_t leftMotorCurrent();
uint16_t rightMotorCurrent();

enum class SwitchPosition : uint8_t	{
	UP, MIDDLE, DOWN
};

SwitchPosition switchPosition();

enum class Resistor : uint8_t{
	R1K=0, R6K8=1, R47K=2, R330K=3
};
Resistor& operator++(Resistor &r){
	if(r==Resistor::R330K)
		return r;
	return r = static_cast<Resistor>(static_cast<uint8_t>(r)+1);
}
Resistor operator++(Resistor &r, int){
	Resistor result = r;
	++r;
	return result;
}
Resistor& operator--(Resistor &r){
	if(r==Resistor::R1K)
		return r;
	return r = static_cast<Resistor>(static_cast<uint8_t>(r)-1);
}
Resistor operator--(Resistor &r, int){
	Resistor result = r;
	--r;
	return result;
}
Resistor getSelectedResistor();
uint16_t* getIRData();
