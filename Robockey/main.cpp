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
extern "C"{
	//#include "m_usb.h"
	#include "m_wii.h"
}

int main(void)
{
	m_clockdivide(0);
	sei();
	//initDigital();
	//initClock();
	initADC();
	initWireless();
	//initLocalization();
	
	//m_usb_init();
	
	m_green(ON);
	//uint8_t batteryLowCount = 0;
	//setEnabled(true);
	while (1) {
		m_red(TOGGLE);
		beginADC();
		while(!adcUpdateCompleted());
		sendIR();
		_delay_ms(100);
		//test code for localize
		/*
		uint16_t blobs[12];
		m_wii_read(blobs);
		for(int i = 0; i<12;i++){
		m_usb_tx_int(blobs[i]);
		m_usb_tx_char(',');
		}
		m_usb_tx_char('\n');
		localizeRobot2();
		Pose robot = getRobotPose2();
		m_usb_tx_int(robot.x);
		m_usb_tx_char(',');
		m_usb_tx_int(robot.y);
		m_usb_tx_char(',');
		m_usb_tx_int(robot.o);
		m_usb_tx_char('\n');
		m_wait(500);
		*/
		
		//test code for ADC
		/*
		uint16_t * values = getIRData();
		for (int i = 0; i < 16; i++)
		{
			m_usb_tx_int(i);
			m_usb_tx_char(' ');
			m_usb_tx_int(values[i]);
			m_usb_tx_char(' ');
		}
		m_usb_tx_char('\n');
		m_wait(2000);
		*/
	
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