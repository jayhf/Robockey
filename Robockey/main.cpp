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
#include "Strategies.h"
#include <stdlib.h>


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
void localizationCalibration();
void strategyWirelessTest();

int main(void)
{
	//strategyWirelessTest();
	friendlies();
}

void localizationCalibration(){
	m_clockdivide(0);
	m_disableJTAG();
	initLocalization();
	m_usb_init();
	m_bus_init();
	m_wii_open();
	while(1){
		uint16_t buffer[12];
		m_wii_read(buffer);
		for(int i=0;i<12;i++){
			m_usb_tx_int(buffer[i]);
			m_usb_tx_char(',');
		}
		m_usb_tx_char('\n');
		m_usb_tx_char('\r');
		_delay_ms(100);
	}
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
		//m_green(2);
	}
}

void qualify(){
	Pose robot = getRobotPose();
	goTo(Pose(110,0,0),robot);
}
void strategyWirelessTest(){
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
	setLED(LEDColor::OFF);
	updateLocalization();
	//m_red(1);
	while(1){
		updateWireless();
		updateStrategies();
		//m_green(2);
		//m_red(2);
	}
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
	startKick();
	while(1){
		updateLocalization();
		updateWireless();
		updateKick();
		updateLED();
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
		
		if (true){
			if(getRobotPose()==UNKNOWN_POSE){
				setMotors(0,0);
			}
			else{
				
				/*if(stuck()&&!hasPuck()){
				faceLocation(Location(XMAX,0),getRobotPose());
				}
				else{*/
					
				/*	
					if (first == 0 && getPuckLocation().x-initPuck.x<6 && getPuckLocation().x-initPuck.x>-6 &&  getPuckLocation().y-initPuck.y<6 && getPuckLocation().y-initPuck.y>-6){
						faceoff();
					}
					else{
				*/		
						if (first == 0) first++;
						//goToBackwards(Pose(0,0,0),getRobotPose());
						//goToPosition(getPuckLocation().toPose(getPuckHeading()+getRobotPose().o),getRobotPose(),true);
						//goAndKick(Pose(XMAX,0,-PI/2));
						//goalieLogic();
						goBehindPuck();
						//setLED(LEDColor::RED);
					}
				//}
				//}
			}
			else setMotors(0,0);
		}
	}