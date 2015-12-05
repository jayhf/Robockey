#include "wireless.h"
#include "GameState.h"
#include "Localization.h"
#include "ADC.h"
#define F_CPU 16000000
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

#define MAX_MESSAGES_PER_SECOND 32

extern "C"{
	#include "m_rf.h"
}

void initWireless(){
	m_rf_open(1, static_cast<uint8_t>(getThisRobot()), 10);
}

void sendNextMessage(){
	static time lastSend = 0;
	if(!timePassed(lastSend+(ONE_SECOND/MAX_MESSAGES_PER_SECOND)))
		return;
	else
		lastSend = getTime();
	static uint8_t nextMessage = 0;
	nextMessage = nextMessage + 1;
	switch(nextMessage){
		case_0:
		case 0:
			sendRobotLocation();
			break;
		case 1:
			sendPuckPose();
			break;
		case 2:
			sendIR();
			break;
		default:
			nextMessage = 0;
			goto case_0;
	}
}



void sendPacket(Robot robot, uint8_t *packet){
	packet[0]=static_cast<uint8_t>(getThisRobot());
	m_rf_send(static_cast<uint8_t>(robot), (char*)packet, 10);
}


void sendPacket(Robot robot, uint8_t messageID, uint8_t *packet){
	packet[1]=messageID;
	sendPacket(robot,packet);
}

void sendRobotLocation(){
	uint8_t buffer[10];
	Pose pose = getRobotPose();
	buffer[2]=pose.x>>8;
	buffer[3]=pose.x&0xFF;
	buffer[4]=pose.y>>8;
	buffer[5]=pose.y&0xFF;
	buffer[6]=pose.o>>8;
	buffer[7]=pose.o&0xFF;
	sendPacket(Robot::CONTROLLER,0x10,buffer);
}

void sendIR(){
	uint8_t buffer[10];
	uint16_t *irData = getIRData();
	for(int i=0;i<8;i++)
		buffer[i+2] = (irData[i]>>2)&0xFF;
	sendPacket(Robot::CONTROLLER, 0x11, buffer);
	for(int i=0;i<8;i++)
		buffer[i+2] = (irData[i+8]>>2)&0xFF;
	sendPacket(Robot::CONTROLLER, 0x12, buffer);
}

void sendBattery(){
	uint8_t buffer[10];
	buffer[2] = getBattery() >> 8;
	buffer[3] = getBattery() & 0xFF;
	sendPacket(Robot::CONTROLLER, 0x13, buffer);
}

void sendPuckPose(){
	uint8_t buffer[10];
	buffer[2] = getPuckHeading() >> 8;
	buffer[3] = getPuckHeading() & 0xFF;
	buffer[4] = getPuckLocation().x;
	buffer[5] = getPuckLocation().y;
	buffer[6] = static_cast<uint8_t>(getSelectedResistor());
	buffer[7] = getPuckDistance();
	sendPacket(Robot::CONTROLLER, 0x14, buffer);
}

ISR(INT2_vect){
	uint8_t buffer[10];
	m_rf_read((char*)buffer,10);
	switch(buffer[0]){
		case 0x20:
			//Set variable
			break;
		case 0x21:
			//Send back requested variable
			break;
		case static_cast<uint8_t>(Robot::ROBOT1):
		case static_cast<uint8_t>(Robot::ROBOT2):
		case static_cast<uint8_t>(Robot::ROBOT3):
			{
				//Received a message from another team robot
				processTeamMessage(getAlly(static_cast<Robot>(buffer[0])),buffer);
				break;
			}
		case 0xA0:
			updateGameState(GameState::COMM_TEST);
			break;
		case 0xA1:
			updateGameState(GameState::PLAY);
			break;
		case 0xA2:
			goalScored(Team::RED);
			updateScores(buffer[0],buffer[1]);
			break;
		case 0xA3:
			goalScored(Team::BLUE);
			updateScores(buffer[0],buffer[1]);
			break;
		case 0xA4:
			updateGameState(GameState::PAUSE);
			break;
		case 0xA6:
			updateGameState(GameState::HALFTIME);
			break;
		case 0xA7:
			updateGameState(GameState::GAME_OVER);
			break;
		case 0xA8:
			//receivedEnemyLocations((int8_t*)(buffer + 1));
			break;
	}
}

void sendAllyMessage(Ally ally){
	Pose robotPose = getRobotPose();
	Location puckLocation = getPuckLocation();
	uint8_t packet[10];
	packet[1] = robotPose.x;
	packet[2] = robotPose.y;
	packet[3] = puckLocation.x;
	packet[4] = puckLocation.y;
	packet[5] = hasPuck();
	sendPacket(getAllyRobot(ally),packet);
}

void processTeamMessage(Ally ally, uint8_t *data){
	Location allyLocation = Location(data[1],data[2]);
	Location allyPuckLocation = Location(data[3],data[4]);
	//bool allyHasPuck = data[5];
	receivedAllyUpdate(allyLocation, allyPuckLocation, ally);
}