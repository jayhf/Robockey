/*
* Robockey.cpp
*
* Created: 11/6/2015 3:59:51 PM
* Author : Jay
*/

#ifdef _MSC_VER
#include "../Simulator/m_general.h"
#else
#define F_CPU 16000000
#include "m_general.h"
#endif

#include "time.h"
#include "Digital.h"
#include "Localization.h"
#include "GameState.h"
#include "wireless.h"
#include "ADC.h"
extern "C"{
	#include "m_usb.h"
}

int main(void)
{
	m_clockdivide(0);
	initDigital();
	initClock();
	initADC();
	initWireless();
	initLocalization();
	
	m_usb_init();
	
	uint8_t batteryLowCount = 0;
	//test code
	setEnabled(true);
	setLED(LEDColor::BLUE);
	m_wait(50);
	setLED(LEDColor::OFF);
	m_wait(50);
	setLED(LEDColor::RED);
	m_wait(50);
	setLED(LEDColor::OFF);
	m_wait(50);
	setLED(LEDColor::PURPLE);
	m_wait(50);
	setLED(LEDColor::OFF);
	m_wait(50);
	movement(100,0);
	m_usb_tx_int(leftMotorCurrent());
	m_wait(100);
	movement(-100,0);
	m_usb_tx_int(leftMotorCurrent());
	m_wait(100);
	movement(0,100);
	m_usb_tx_int(rightMotorCurrent());
	m_wait(100);
	movement(0,-100);
	m_usb_tx_int(rightMotorCurrent());
	m_wait(100);
	movement(0,0);
	while(!switchesPressed()){
		setLED(LEDColor::PURPLE);
	}
	setLED(LEDColor::OFF);
	
	if(boostReady()){
		m_usb_tx_int(5000);
		startKick(15);
		m_wait(16);
		updateKick();
	}
	for (int i = 0; i < 3; i++){
		if(switchPosition() == SwitchPosition::DOWN){
			m_usb_tx_int(0);
		}
		else if(switchPosition() == SwitchPosition::MIDDLE){
			m_usb_tx_int(1);
		}
		else if(switchPosition() == SwitchPosition::UP){
			m_usb_tx_int(2);
		}
		m_wait(1000);
	}
	
	m_usb_tx_int(static_cast<int>(getSelectedResistor()));
	
	beginADC();
	//test code
	while (1) {
		uint16_t * values = getIRData();
		for (int i = 0; i < 16; i++)
		{
			m_usb_tx_int(i);
			m_usb_tx_char(' ');
			m_usb_tx_int(values[i]);
		}
		m_usb_tx_char('\n');
		m_wait(2000);
		
		/*
		beginADC();
		localizeRobot();
		if(getTeam() == Team::UNKNOWN)
		determineTeam();
		if(batteryLow())
		batteryLowCount++;
		else
		batteryLowCount = 0;
		if(batteryLowCount >= 25)
		sleep();
		
		while(!adcUpdateCompleted());
		Pose location = getRobotPose();
		findPuck(location);
		//invert all poses depending on the team. Probably should be dealt with in the localization code itself
		Pose desiredPose;
		
		//behavior code

		//move towards desired pose if allowed by game state
		*/
	}
}
