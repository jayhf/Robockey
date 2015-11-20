#include "wireless.h"
#include "GameState.h"
#include "Localization.h"
#include "ADC.h"

extern "C"{
	#include "m_rf.h"
}

void initWireless(){
	m_rf_open(1, static_cast<uint8_t>(getThisRobot()),10);
}


void sendPacket(Robot robot, uint8_t messageID, uint8_t *packet){
	packet[0]=static_cast<uint8_t>(getThisRobot());
	packet[1]=messageID;
	m_rf_send(static_cast<uint8_t>(robot), (char*)packet, 10);
}

void sendRobotLocation(){
	uint8_t buffer[10];
	Pose pose = getRobotPose();
	buffer[2]=pose.x>>8;
	buffer[3]=pose.x&0xFF;
	buffer[4]=pose.y>>8;
	buffer[5]=pose.x&0xFF;
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
		case 0x41:
		case 0x42:
		case 0x43:
			{
				//Received a message from another team robot
				Robot robot = static_cast<Robot>(buffer[0]);
				//processTeamMessage(buffer+1);
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
			updateEnemyLocations((int8_t*)(buffer + 1));
			break;
	}
}
