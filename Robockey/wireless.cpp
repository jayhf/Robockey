#include "wireless.h"
#include "GameState.h"
#include "Localization.h"
#include "ADC.h"
#include "Strategies.h"

#define F_CPU 16000000
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

#define MAX_MESSAGES_PER_SECOND 32

extern "C"{
	#include "m_rf.h"
}

time lastAllyUpdateTime[] = {(time)-1000,(time)-1000};
bool allyHasPuck[] = {0,0};
bool allyIsGoalie[] = {0,0};
uint8_t allyStrategies[] = {PICK_SOMETHING,PICK_SOMETHING};
uint8_t allyStrategySuggestions[] = {PICK_SOMETHING,PICK_SOMETHING};
uint8_t gameCommandsToSend[] = {static_cast<uint8_t>(GameState::HALFTIME),static_cast<uint8_t>(GameState::HALFTIME)};
uint8_t gameCommandsToSendCount[] = {0,0};
time lastHalftimeUpdateTime = -10*ONE_SECOND-1;
Pose destinationPose;

void updateDestination(Pose destination){
	destinationPose = destination;
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
	nextMessage++;
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
		case 3:
			sendIR2();
			break;
		case 4:
			sendAllyMessage(Ally::ALLY1);
			break;
		case 5:
			sendAllyMessage(Ally::ALLY2);
			break;
		case 6:
			sendDestinationMessage();
			break;
		default:
			nextMessage = 0;
			goto case_0;
	}
}

void sendDestinationMessage(){
	uint8_t packet[10]={0,0,(uint8_t)destinationPose.x,(uint8_t)destinationPose.y,(uint8_t)(destinationPose.o>>8),(uint8_t)(destinationPose.o&0xFF),0,0,0,0};
	sendDebugPacket(Robot::CONTROLLER,0x22,packet);
}

void handleGameStateMessage(uint8_t id){
	switch(id){
		case 0xA0:
			updateGameState(GameState::COMM_TEST);
			break;
		case 0xA1:
			updateGameState(GameState::PLAY);
			break;
		case 0xA4:
			updateGameState(GameState::PAUSE);
			break;
		case 0xA6:
			if(timePassed(lastHalftimeUpdateTime+10*ONE_SECOND)){
				lastHalftimeUpdateTime = getTime();
				updateGameState(GameState::HALFTIME);
			}
			break;
		case 0xA7:
			updateGameState(GameState::GAME_OVER);
			break;
		//case 0xA8:
			//receivedEnemyLocations((int8_t*)(buffer + 1));
	}
}

uint8_t computeChecksum(uint8_t *packet){
	uint8_t checksum = 0;
	for(uint8_t i = 0; i<9; i++)
		checksum ^= packet[i];
	return checksum;
}
void sendPacket(Robot robot, uint8_t *packet){
	packet[0]=static_cast<uint8_t>(getThisRobot());
	packet[9]=computeChecksum(packet);
	m_rf_send(static_cast<uint8_t>(robot), (char*)packet, 10);
}


void sendDebugPacket(Robot robot, uint8_t messageID, uint8_t *packet){
	packet[0]=static_cast<uint8_t>(getThisRobot());
	packet[1]=messageID;
	m_rf_send(static_cast<uint8_t>(robot), (char*)packet, 10);
}

void sendRobotLocation(){
	uint8_t buffer[10];
	Pose pose = getRobotPose(); //getAllyLocations()[0].toPose();
	buffer[2]=pose.x>>8;
	buffer[3]=pose.x&0xFF;
	buffer[4]=pose.y>>8;
	buffer[5]=pose.y&0xFF;
	buffer[6]=pose.o>>8;
	buffer[7]=pose.o&0xFF;
	sendDebugPacket(Robot::CONTROLLER,0x10,buffer);
}

void sendIR(){
	uint8_t buffer[10];
	uint16_t *irData = getIRData();
	for(int i=0;i<8;i++)
		buffer[i+2] = (irData[i]>>2)&0xFF;
	sendDebugPacket(Robot::CONTROLLER, 0x11, buffer);
}

void sendIR2(){
	uint8_t buffer[10];
	uint16_t *irData = getIRData();
	for(int i=0;i<8;i++)
	buffer[i+2] = (irData[i+8]>>2)&0xFF;
	sendDebugPacket(Robot::CONTROLLER, 0x12, buffer);
}

void sendBattery(){
	uint8_t buffer[10];
	buffer[2] = getBattery() >> 8;
	buffer[3] = getBattery() & 0xFF;
	sendDebugPacket(Robot::CONTROLLER, 0x13, buffer);
}

void sendPuckPose(){
	uint8_t buffer[10];
	buffer[2] = getPuckHeading() >> 8;
	buffer[3] = getPuckHeading() & 0xFF;
	buffer[4] = getPuckLocation().x;
	buffer[5] = getPuckLocation().y;
	buffer[6] = static_cast<uint8_t>(getSelectedResistor());
	buffer[7] = getPuckDistance();
	sendDebugPacket(Robot::CONTROLLER, 0x14, buffer);
}

volatile bool hasMessage = false;
void updateWireless(){
	//m_green(!flipCoordinates());
	if(timePassed(lastHalftimeUpdateTime+10*ONE_SECOND)){
		lastHalftimeUpdateTime = getTime()-10*ONE_SECOND-1;
	}
	if(hasMessage){
		hasMessage = false;
		uint8_t buffer[10];
		m_rf_read((char*)buffer,10);
		switch(buffer[0]){
			case static_cast<uint8_t>(Robot::ROBOT1):
			case static_cast<uint8_t>(Robot::ROBOT2):
			case static_cast<uint8_t>(Robot::ROBOT3):
				//Received a message from another team robot
				if(buffer[9]==computeChecksum(buffer)){
					static uint8_t c = 0;
					if(c++%10 == 0)
						m_green(2);
					processTeamMessage(getAlly(static_cast<Robot>(buffer[0])),buffer);
				}
				break;
			case 0xA2:
				goalScored(Team::RED);
				updateScores(buffer[0],buffer[1]);
				break;
			case 0xA3:
				goalScored(Team::BLUE);
				updateScores(buffer[0],buffer[1]);
				break;
		}
		if(buffer[0] == 0xA0 || buffer[0] == 0xA1 || buffer[0] == 0xA4 || buffer[0] == 0xA6 || buffer[0] == 0xA7){
			gameCommandsToSend[0] = gameCommandsToSend[1] = buffer[0];
			gameCommandsToSendCount[0] = gameCommandsToSendCount[1] = 5;
			handleGameStateMessage(buffer[0]);
		}
	}
	sendNextMessage();
	if(timePassed(lastAllyUpdateTime[0]+ONE_SECOND))
		lastAllyUpdateTime[0] = getTime() - ONE_SECOND - 1;

	if(timePassed(lastAllyUpdateTime[1]+ONE_SECOND))
		lastAllyUpdateTime[1] = getTime() - ONE_SECOND - 1;
}

ISR(INT2_vect){
	hasMessage = true;
}

uint8_t getAllyStrategy(Ally ally){
	return allyStrategies[static_cast<uint8_t>(ally)];
}
uint8_t getAllySuggestedStrategy(Ally ally){
	return allyStrategySuggestions[static_cast<uint8_t>(ally)];
}

bool hasPuck(Ally ally){
	if(allyUpToDate(ally))
		return allyHasPuck[static_cast<uint8_t>(ally)];
	return false;
}

bool allyUpToDate(Ally ally){
	return !timePassed(lastAllyUpdateTime[static_cast<uint8_t>(ally)]+ONE_SECOND);
}

void sendAllyMessage(Ally ally){
	Pose robotPose = getRobotPose();
	Location puckLocation = getPuckLocation();
	uint8_t packet[10];
	packet[1] = robotPose.x;
	packet[2] = robotPose.y;
	packet[3] = puckLocation.x;
	packet[4] = puckLocation.y;
	packet[5] = recentlyHadPuck();
	packet[6] = getCurrentStrategyID();
	packet[7] = getOurSuggestedStrategy(ally);
	packet[8] = gameCommandsToSend[static_cast<uint8_t>(ally)];
	if(gameCommandsToSendCount[static_cast<uint8_t>(ally)] > 0)
		gameCommandsToSendCount[static_cast<uint8_t>(ally)]--;
	else
		gameCommandsToSend[static_cast<uint8_t>(ally)] = 0;
	sendPacket(getAllyRobot(ally),packet);
}

void processTeamMessage(Ally ally, uint8_t *data){
	Location allyLocation = Location(data[1],data[2]);
	Location allyPuckLocation = Location(data[3],data[4]);
	receivedAllyUpdate(allyLocation, allyPuckLocation, ally);
	uint8_t allyID = static_cast<uint8_t>(ally);
	allyHasPuck[allyID] = data[5];
	allyIsGoalie[allyID] = 0;//isGoalie(data[6]) && data[6] != UNKNOWN_STRATEGY && data[6] != PICK_SOMETHING;
	allyStrategies[allyID] = data[6];
	allyStrategySuggestions[allyID] = data[7];
	lastAllyUpdateTime[allyID] = getTime();
	if(data[8]!=0)
		handleGameStateMessage(data[8]);
}

Ally getHighestPriorityAlly(){
	if(!allyUpToDate(Ally::ALLY2))
		return Ally::ALLY1;
	if(!allyUpToDate(Ally::ALLY1))
		return Ally::ALLY2;
	if(allyHasPuck[0])
		return Ally::ALLY1;
	if(allyHasPuck[1])
		return Ally::ALLY2;
	if(allyIsGoalie[0])
		return Ally::ALLY1;
	if(allyIsGoalie[1])
		return Ally::ALLY2;
	return Ally::ALLY1;
}

bool allyHigherPriorityThanMe(Ally ally){
	if(!allyUpToDate(ally))
		return false;
	bool allyAddressHigher = static_cast<uint8_t>(getAllyRobot(ally)) > static_cast<uint8_t>(getThisRobot());
	uint8_t allyID = static_cast<uint8_t>(ally);
	if(allyAddressHigher){
		if(allyHasPuck[allyID])
			return true;
		if(hasPuck())
			return false;
		if(allyIsGoalie[allyID])
			return true;
		if(isGoalie(getCurrentStrategy()->getID()))
			return false;
		return true;
	}
	if(hasPuck())
		return false;
	if(allyHasPuck[allyID])
		return true;
	if(isGoalie(getCurrentStrategy()->getID()))
		return false;
	if(allyIsGoalie[allyID])
		return true;
	return false;
}