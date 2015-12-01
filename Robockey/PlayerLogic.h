/*
* PlayerLogic.cpp
*
* Created: 11/15/2015 11:14:10 AM
*  Author: Daniel Orol
*/
#pragma once

#include <stdint.h>

enum class Player : uint8_t{
	GOALIE = 0, DEFENSE = 1, SCORER = 2, ASSISTER = 3
};

void playerLogic(Player player);
void goalieLogic();
void leftCorner();
void rightCorner();
void avoidGoalie();
void fakeGoalie();
void followWall();
void charge();
void tryKick();
void requestHelp();
void defenseLogic();