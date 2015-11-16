#include "wireless.h"
#include "GameState.h"
#include "Localization.h"

extern "C"{
	#include "m_rf.h"
}

void initWireless(){
	m_rf_open(1, 84 + static_cast<uint8_t>(getThisRobot()),10);
}


void sendPacket(Robot robot, uint8_t *packet){
	m_rf_send(84 + static_cast<uint8_t>(robot), (char*)packet, 10);
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
				Robot robot = static_cast<Robot>(buffer[0]-40);
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
