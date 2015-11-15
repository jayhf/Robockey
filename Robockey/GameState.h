#pragma once

#include <stdint.h>

enum class GameState : uint8_t{
	NONE = 0, COMM_TEST = 0xA0, PLAY = 0xA1, PAUSE = 0xA4, HALFTIME = 0xA6, GAME_OVER = 0xA7
};

enum class Robot : uint8_t{
	CONTROLLER = 0, ROBOT1 = 1, ROBOT2 = 2, ROBOT3 = 3
};

enum class Team : uint8_t{
	RED, BLUE
};

uint8_t getRScore();
uint8_t getBScore();
//Returns the ID of this robot in the range [1,3]
uint8_t getRobotID();

void initGameState();
void updateGameState();
void updateGameState(GameState state);

void goalScored(Team team);

void updateEnemyLocations(uint8_t *positions);

bool allowedToMove();
