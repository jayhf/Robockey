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

int main(void)
{
	m_clockdivide(0);
    initDigital();
	initClock();
	initADC();
	initWireless();
	initLocalization();
	
	uint8_t batteryLowCount = 0;
    while (1) {
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
    }
}
