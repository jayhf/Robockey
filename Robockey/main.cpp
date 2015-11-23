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
#include <util/delay.h>
#include "m_general.h"
#endif

#include "time.h"
#include "Digital.h"
#include "Localization.h"
#include "GameState.h"
#include "wireless.h"
#include "ADC.h"
#include "PathPlanning.h"
#include "BAMSMath.h"
#define OFF 0
extern "C"{
	#include "m_usb.h"
	#include "m_wii.h"
}
void qualify();
bool startPos;
int main(void)
{
	m_clockdivide(0);
	m_disableJTAG();
	sei();
	
	m_usb_init();
	
	initDigital();
	initClock();
	initADC();
	initWireless();
	initLocalization();
	m_wait(100);
	localizeRobot();
	startPos = getRobotPose().x >= 0;
	
	
	

	//uint8_t batteryLowCount = 0;
	setEnabled(true);
	int i = 0;
	//uint16_t blobs[12]={0,0,0,0,0,0,0,0,0,0,0,0};
	uint16_t* values;
	while (1) {
		beginADC();
		localizeRobot();
		Pose robot = getRobotPose();
		m_usb_tx_int(robot.x);
		m_usb_tx_char(',');
		m_usb_tx_int(robot.y);
		m_usb_tx_char(',');
		m_usb_tx_int(robot.o);
		m_usb_tx_char('\n');
		
		findPuck(robot);
		values = getIRData();
		for (int i = 0; i<16; i++){
			m_usb_tx_int(values[i]);
			m_usb_tx_char(',');
		}
		m_usb_tx_char('\n');
		m_wait(1000);
	
	//real code
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

void qualify(){
	Pose robot = getRobotPose();
	if (startPos){
		goTo(Pose(-110,0,0),robot);
	}
	else goTo(Pose(110,0,0),robot);
}