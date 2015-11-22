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
extern "C"{
	//#include "m_usb.h"
	#include "m_wii.h"
}
int main(void)
{
	m_clockdivide(0);
	m_disableJTAG();
	sei();
	
	initDigital();
	initClock();
	initADC();
	initWireless();
	initLocalization();
	
	//m_usb_init();

	//uint8_t batteryLowCount = 0;
	setEnabled(true);
	int i = 0;
	//uint16_t blobs[12]={0,0,0,0,0,0,0,0,0,0,0,0};
	#define OFF 0
	while (1) {
		//beginADC();
		//while(!adcUpdateCompleted());
		m_red(TOGGLE);
		//_delay_ms(100);
		localizeRobot();
		/*if (allowedToMove()){
			m_usb_tx_int(1);
		}
		else m_usb_tx_int(0);
		*/
		/*
		Pose robot = getRobotPose();
		m_usb_tx_int(robot.x);
		m_usb_tx_char(',');
		m_usb_tx_int(robot.y);
		m_usb_tx_char(',');
		m_usb_tx_int(robot.o);
		m_usb_tx_char('\n');
		*/
		//sendIR();
		//sendBattery();
		sendRobotLocation();
		if(i<400){
			goTo(Pose(0,0,0),getRobotPose());
			//goToPositionSpin(Pose(0,0,0),getRobotPose());
			//facePose(Pose(0,0,0),getRobotPose());
			//m_wait(15);
			i++;
		}
		else setMotors(0,0);
		m_green(allowedToMove());
		//m_usb_tx_int(getRobotPose().o-o);
		//m_usb_tx_char('\n');
		//m_wait(500);
		//facePose(Pose(-52,-20,0),robot);
		//m_usb_tx_int(static_cast<int>(facingPose(Pose(-52,-20,0),robot)));
		//test code for localize
		
		/*
		m_wii_read(blobs);
		for(int i = 0; i<12;i++){
		m_usb_tx_int(blobs[i]);
		m_usb_tx_char(',');
		}
		m_usb_tx_char('\n');
		m_wait(1000);
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
