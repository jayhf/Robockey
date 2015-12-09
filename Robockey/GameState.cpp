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
#include <avr/io.h>
#define F_CPU 16000000
#include <util/delay.h>

volatile uint8_t redScore;
volatile uint8_t blueScore;
volatile GameState gameState = GameState::NONE;
Team team = Team::UNKNOWN;
volatile bool pastHalfTime = false;

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
	static uint8_t robotCount = 0;
	static Robot thisRobot = Robot::CONTROLLER;
	while(robotCount < 10){
		Robot selectedRobot;
		switch(switchPosition()){
			case SwitchPosition::DOWN:
				setLED(LEDColor::RED);
				selectedRobot = Robot::ROBOT1;
				break;
			case SwitchPosition::MIDDLE:
				setLED(LEDColor::BLUE);
				selectedRobot = Robot::ROBOT2;
				break;
			case SwitchPosition::UP:
				setLED(LEDColor::PURPLE);
				selectedRobot = Robot::ROBOT3;
				break;
			default:
				selectedRobot = Robot::CONTROLLER;
		}
		if(thisRobot != selectedRobot){
			robotCount = 0;
			thisRobot = selectedRobot;
		}
		else
			robotCount++;
		_delay_ms(100);
	}
	return thisRobot;
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
			if(robotPose.x > 0)
				teamCount++;
			else if(teamCount > 0)
				teamCount = 0;
			else if(robotPose.x < 0)
				teamCount--;
			else
				teamCount = 0;

			if(teamCount == 10)
				team = Team::BLUE;
			else if(teamCount == -10)
				team = Team::RED;
		}
		//if(switchesPressed()){
		//	pastHalfTime = !pastHalfTime;
		//}
	}
}

void updateGameState(GameState state){
	if(state == GameState::HALFTIME){
		pastHalfTime = !pastHalfTime;
		state = GameState::PAUSE;
		coordinatesFlipped();
	}
	else{
		gameState = state;
	}
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
		else
			setLED(LEDColor::OFF);
	}
	else if(gameState == GameState::PLAY){
		setLEDToTeamColor();
	}
	else
		setLED(LEDColor::OFF);
}

Ally getAlly(Robot ally){
	switch(ally){
		case Robot::ROBOT1:
			return Ally::ALLY1;
		case Robot::ROBOT3:
			return Ally::ALLY2;
		default:
			return getThisRobot() == Robot::ROBOT1 ? Ally::ALLY1 : Ally::ALLY2;
	}
}

Robot getAllyRobot(Ally ally){
	switch(getThisRobot()){
		case Robot::ROBOT1:
			return ally == Ally::ALLY1 ? Robot::ROBOT2 : Robot::ROBOT3;
		case Robot::ROBOT2:
			return ally == Ally::ALLY1 ? Robot::ROBOT1 : Robot::ROBOT3;
		default:
			return ally == Ally::ALLY1 ? Robot::ROBOT1 : Robot::ROBOT2;
	}
}