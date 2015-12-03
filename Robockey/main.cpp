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
#include "miscellaneous.h"
#endif

#include <avr/interrupt.h>
#include "time.h"
#include "Digital.h"
#include "Localization.h"
#include "GameState.h"
#include "wireless.h"
#include "ADC.h"
#include "PathPlanning.h"
#include "BAMSMath.h"
#include "FastMath.h"
#include "PlayerLogic.h"


extern "C"{
	#include "m_usb.h"
	#include "m_wii.h"
	#include "m_bus.h"
	#include "m_rf.h"
}
int first = 0;


void qualify();
void puckLocalizationTest();
Pose getEnemyGoal();
void friendlies();
void faceoff();

int main(void)
{
	friendlies();
	m_clockdivide(0);
	m_disableJTAG();
	sei();
	m_bus_init();
	initDigital();
	initClock();
	initADC();
	initWireless();
	initLocalization();
	updateLocalization();
	_delay_ms(500);
	updateLocalization();
	while(1){
		updateLocalization();
		sendRobotLocation();
		sendPuckPose();
		sendIR();
		if(!timePassed(1500)){
			if(!hasPuck()){
				setMotors(0,0);
				//goToPuck(getPuckLocation().toPose(getPuckHeading()),getRobotPose());
			}
			else setMotors(0,0);
		}
		else setMotors(0,0);
		updateLED();
		//setMotors(-400,400);
		//tryKick();
		//updateKick();
		/*sendRobotLocation();
		m_green(0);
		if(!timePassed(500)){
		m_green(1);
		//setMotors(900,550);
		goToPositionPuck(Pose(0,0,0),getRobotPose());
		}
		else setMotors(0,0);
		*/
	}
	//puckLocalizationTest();
	friendlies();
}

void puckLocalizationTest(){
	setEnabled(false);
	while(1){
		updateLocalization();
		beginADC();
		while(!adcUpdateCompleted());
		sendRobotLocation();
		sendIR();
		sendPuckPose();
		_delay_ms(100);
		m_green(2);
	}
}
void oldMain(){
	//uint8_t batteryLowCount = 0;
	//uint16_t blobs[12]={0,0,0,0,0,0,0,0,0,0,0,0};
	uint16_t* values;
	Pose robot;
	Location puck;
	while (1) {
		m_red(flipCoordinates());
		beginADC();
		updateLocalization();
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
		
		findPuck();
		
		puck = getPuckLocation();
		m_usb_tx_int(puck.x);
		m_usb_tx_char(',');
		m_usb_tx_int(puck.y);
		m_usb_tx_char(',');
		m_usb_tx_int(getPuckHeading());
		m_usb_tx_char('\n');
		
		values = getIRData();
		for (int i = 0; i<16; i++){
			m_usb_tx_int(values[i]);
			m_usb_tx_char(',');
		}
		m_usb_tx_char('\n');
		_delay_ms(1000);
		
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
	goTo(Pose(110,0,0),robot);
}

void friendlies(){
	m_clockdivide(0);
	m_disableJTAG();
	sei();
	m_bus_init();
	initDigital();
	initClock();
	initADC();
	initWireless();
	initLocalization();
	updateLocalization();
	_delay_ms(500);
	updateLocalization();
	Location initPuck = getPuckLocation();
	while(1){
		updateLocalization();
		updateLED();
		sendRobotLocation();
		sendPuckPose();
		switch(getThisRobot()){
			case Robot::ROBOT1:
				updatePlayer(Player::GOALIE);
				//setLED(LEDColor::RED);
				break;
			case Robot::ROBOT2:
				updatePlayer(Player::SCORER);
				//setLED(LEDColor::BLUE);
				break;
			case Robot::ROBOT3:
				updatePlayer(Player::DEFENSE);
				//setLED(LEDColor::PURPLE);
				break;
			default:
				updatePlayer(Player::NONE);
			break;
		}
		if (allowedToMove()){
			if(stuck()&&!hasPuck()){
				m_green(1);
				setMotors(-900,-900);
				_delay_ms(10);
			}
			else{
				m_green(0);
				if (first == 0 && getPuckLocation().x-initPuck.x<3 && getPuckLocation().x-initPuck.x>-3 &&  getPuckLocation().y-initPuck.y<3 && getPuckLocation().y-initPuck.y>-3){
					faceoff();
				}
				else{
					if (first == 0) first++;
						//goToPuck(getPuckLocation().toPose(getPuckHeading()),getRobotPose());
					playerLogic(getPlayer());
				}
			}
		}
	}
}