#pragma once

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

enum SwitchPosition	{
	UP, MIDDLE, DOWN
};

enum SwitchPosition switchPosition();

enum resistor{
	R1K=0, R6K8=1, R47K=2, R330K=3
};

enum resistor getSelectedResistor();
uint16_t* getIRData();
