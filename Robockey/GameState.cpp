/*
 * GameState.cpp
 *
 *  Created on: Nov 15, 2015
 *      Author: Jay
 */

#include "GameState.h"
#include "ADC.h"
#include "Localization.h"
#include "Digital.h"
#include "time.h"

uint8_t redScore;
uint8_t blueScore;
GameState gameState = GameState::NONE;
Team team = Team::UNKNOWN;
bool pastHalfTime = false;

GameState getGameState(){
	return gameState;
}

uint8_t getRScore(){
	return redScore;
}
uint8_t getBScore(){
	return blueScore;
}

bool allowedToMove(){
	return gameState == GameState::PLAY;
}

Robot getThisRobot(){
	switch(switchPosition()){
		case SwitchPosition::DOWN:
			return Robot::ROBOT1;
		case SwitchPosition::MIDDLE:
			return Robot::ROBOT2;
		case SwitchPosition::UP:
			return Robot::ROBOT3;
	}
	return Robot::ROBOT1;
}

Team getTeam(){
	return team;
}

void goalScored(Team scorer){
	if(scorer == getTeam()){
		//tell main logic that it's strategy was successful, so it can do it more often
	}
}

void updateScores(uint8_t red, uint8_t blue){
	redScore = red;
	blueScore = blue;
}

void determineTeam(){
	static int8_t teamCount;
	Pose robotPose = getRobotPose();
	
	if(team == Team::UNKNOWN){
		if(robotPose != UNKNOWN_POSE){
			if(robotPose.x > XMAX/2)
				teamCount++;
			else if(teamCount > 0)
				teamCount = 0;
			else if(robotPose.x < XMIN/2)
				teamCount--;
			else
				teamCount = 0;

			if(teamCount == 10)
				team = Team::BLUE;
			else if(teamCount == -10)
				team = Team::RED;
		}
	}
}

void updateGameState(GameState state){
	gameState = state;
	if(state == GameState::HALFTIME)
		pastHalfTime = !pastHalfTime;
}

bool flipCoordinates(){
	return pastHalfTime ^ (getTeam() == Team::BLUE);
}

void setLEDToTeamColor(){
	switch(getTeam()){
		case Team::BLUE:
			setLED(LEDColor::BLUE);
			break;
		case Team::RED:
			setLED(LEDColor::RED);
			break;
		case Team::UNKNOWN:
			setLED(LEDColor::PURPLE);
			break;
	}
}

void updateLED(){
	static time startTime;
	static bool commTesting = false;
	if(gameState == GameState::COMM_TEST){
		commTesting = true;
		startTime = getTime();
		updateGameState(GameState::NONE);
	}
	if(gameState == GameState::NONE){
		if(commTesting){
			if(timePassed(startTime+256)){
				commTesting = false;
				setLED(LEDColor::OFF);
			}
			else{
				uint16_t elapsedTime = getTime()-startTime;
				if(elapsedTime & (1<<5))
					setLEDToTeamColor();
				else
					setLED(LEDColor::OFF);
			}
		}
	}
	else if(gameState == GameState::PLAY){
		setLEDToTeamColor();
	}
	else
		setLED(LEDColor::OFF);
}

uint8_t getAllyID(uint8_t address){
	switch(static_cast<Robot>(address)){
		case Robot::ROBOT1:
			return 0;
		case Robot::ROBOT3:
			return 1;
		default:
			return getThisRobot() == Robot::ROBOT3;
	}
}