#pragma once

#include <stdint.h>

enum class GameState : uint8_t{
	NONE = 0, PLAY = 0xA1, PAUSE = 0xA4, HALFTIME = 0xA6, GAME_OVER = 0xA7
};

enum class Robot : uint8_t{
	CONTROLLER = 186, ROBOT1 = 0, ROBOT2 = 1, ROBOT3 = 2
};

enum class Ally : uint8_t{
	ALLY1 = 0, ALLY2 = 1
};

enum class Team : uint8_t{
	UNKNOWN, RED, BLUE
};

enum class StrategyType : uint8_t{
	GOALIE = 0b0000000, DEFENSE = 0b01000000, OFFENSE = 0b10000000, SCORER = 0b11000000
};

uint8_t getRScore();
uint8_t getBScore();
Robot getThisRobot();
Team getTeam();

void initGameState();
void determineTeam();
void updateGameState(GameState state);
GameState getGameState();
void updateLED();

void startCommTest();

void goalScored(Team team);
void updateScores(uint8_t red, uint8_t blue);

bool allowedToMove();

bool flipCoordinates();

//A way to identify which ally is which. One ally is always ally 0. The other is always ally 1.
Ally getAlly(Robot ally);
Robot getAllyRobot(Ally ally);