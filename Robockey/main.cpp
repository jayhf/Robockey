/*
 * Robockey.cpp
 *
 * Created: 11/6/2015 3:59:51 PM
 * Author : Jay
 */ 

#define F_CPU 16000000
#include "m_general.h"
#include "Localization.h"
#include "ADC.h"
#include "time.h"
#include "Digital.h"
#include "GameState.h"
#include "wireless.h"

int main(void)
{
	m_clockdivide(0);
    initDigital();
	initClock();
	initADC();
	initGameState();
	initWireless();
	initLocalization();
	
	uint8_t batteryLowCount = 0;
    while (1) {
		beginADC();
		localizeRobot();
		updateGameState();
		
		if(batteryLow())
			batteryLowCount++;
		else
			batteryLowCount = 0;
		if(batteryLowCount >= 25)
			sleep();
		
		while(!adcUpdateCompleted());

		localizePuck();
		//invert all poses depending on the team. Probably should be dealt with in the localization code itself
		Pose desiredPose();
		
		//behavior code
		
		//move towards desired pose if allowed by game state
    }
}