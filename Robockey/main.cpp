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
#include "FastMath.h"

#define OFF 0

extern "C"{
	#include "m_usb.h"
	#include "m_wii.h"
}

void qualify();
void puckLocalizationTest();
bool startPos = getStartPositive();
Pose getEnemyGoal();

int main(void)
{
	uint16_t potato = 12;
//	uint8_t test = sqrt(potato);
//	PORTB = test;
	m_clockdivide(0);
	m_disableJTAG();
	sei();
	
	//m_usb_init();
	
	initDigital();
	initClock();
	initADC();
	initWireless();
	initLocalization();
	m_wait(100);
	localizeRobot();
	
	puckLocalizationTest();
}
void puckLocalizationTest(){
	setEnabled(false);
	while(1){
		localizeRobot();
		findPuck(getRobotPose());
		beginADC();
		while(!adcUpdateCompleted());
		sendRobotLocation();
		sendIR();
		sendPuckPose();
		_delay_ms(100);
		m_green(TOGGLE);
	}
}
void oldMain(){
	startPos = getRobotPose().x >= 0;
		//uint8_t batteryLowCount = 0;
	int i = 0;
	//uint16_t blobs[12]={0,0,0,0,0,0,0,0,0,0,0,0};
	uint16_t* values;
	Pose robot;
	Pose puck;
	while (1) {
		beginADC();
		localizeRobot();
		/*if(i<300){
		//goToPuck(Pose(0,0,0),getRobotPose());
		m_wait(15);
		i++;
		}
		else setMotors(0,0);
		*/
		
		robot = getRobotPose();
		m_usb_tx_int(robot.x);
		m_usb_tx_char(',');
		m_usb_tx_int(robot.y);
		m_usb_tx_char(',');
		m_usb_tx_int(robot.o);
		m_usb_tx_char('\n');
		
		findPuck(robot);
		
		puck = getPuckLocation();
		m_usb_tx_int(puck.x);
		m_usb_tx_char(',');
		m_usb_tx_int(puck.y);
		m_usb_tx_char(',');
		m_usb_tx_int(puck.o);
		m_usb_tx_char('\n');
		
		values = getIRData();
		for (int i = 0; i<16; i++){
			m_usb_tx_int(values[i]);
			m_usb_tx_char(',');
		}
		m_usb_tx_char('\n');
		m_wait(1000);
		
		/*
		if(i<100){
			faceAngle(getPuckLocation().o,robot);
		//faceAngle(0,robot);
		m_wait(15);
		i++;
		}
		else setMotors(0,0);
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
void qualify(){
	Pose robot = getRobotPose();
	if (startPos){
		goTo(Pose(-110,0,0),robot);
	}
	else goTo(Pose(110,0,0),robot);
}

