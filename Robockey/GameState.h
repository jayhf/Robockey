#pragma once

#include <stdint.h>

enum class GameState : uint8_t{
	NONE = 0, COMM_TEST = 0xA0, PLAY = 0xA1, PAUSE = 0xA4, HALFTIME = 0xA6, GAME_OVER = 0xA7
};

enum class Robot : uint8_t{
	CONTROLLER = 84, ROBOT1 = 85, ROBOT2 = 86, ROBOT3 = 87
};

enum class Team : uint8_t{
	UNKNOWN, RED, BLUE
};

uint8_t getRScore();
uint8_t getBScore();
Robot getThisRobot();
Team getTeam();

void initGameState();
void determineTeam();
void updateGameState(GameState state);
GameState getGameState();

void goalScored(Team team);
void updateScores(uint8_t red, uint8_t blue);

bool allowedToMove();

bool invertCoordinates();
