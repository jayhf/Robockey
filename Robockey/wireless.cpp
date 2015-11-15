#include "GameState.h"
extern "C"{
	#include "m_rf.h"
}

void initWireless(){
	m_rf_open(1, 84 + getRobotID(),10);
}


void sendPacket(Robot robot, uint8_t *packet){
	m_rf_send(84 + robot, packet, 10);
}

ISR(INT2_vect){
	uint8_t *buffer[10];
	m_rf_read(buffer);
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
			//Received a message from another team robot
			enum Robot robot = buffer[0]-40;
			//processTeamMessage(buffer+1);
		case 0xA0:
			updateGameState(GameState.COMM_TEST);
			break;
		case 0xA1:
			updateGameState(GameState.PLAY);
			break;
		case 0xA2:
			goalScored(Team.RED);
			break;
		case 0xA3:
			goalScored(Team.BLUE);
			break;
		case 0xA4:
			updateGameState(GameState.PAUSE);
			break;
		case 0xA6:
			updateGameState(GameState.HALFTIME);
			break;
		case 0xA7:
			updateGameState(GameState.GAME_OVER);
			break;
		case 0xA8:
			updateEnemyLocations(buffer + 1)
			break;
	}
}