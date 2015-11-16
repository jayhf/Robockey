/*
 * GameState.cpp
 *
 *  Created on: Nov 15, 2015
 *      Author: Jay
 */

#include "GameState.h"
#include "ADC.h"
#include "Localization.h"

uint8_t redScore;
uint8_t blueScore;
GameState gameState = GameState::NONE;
Team team = Team::UNKNOWN;

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
	Pose robotPose = getRobotPose();
	if(robotPose != UNKNOWN_POSE){
		if(robotPose.x > XMAX/2)
			team = Team::BLUE;
		else if(robotPose.x < XMIN/2)
			team = Team::RED;
	}
	
}

void updateGameState(GameState state){
	gameState = state;
}
