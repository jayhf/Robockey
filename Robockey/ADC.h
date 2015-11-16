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

Resistor& operator++(Resistor &r);
Resistor operator++(Resistor &r, int);
Resistor& operator--(Resistor &r);
Resistor operator--(Resistor &r, int);

Resistor getSelectedResistor();
uint16_t* getIRData();
