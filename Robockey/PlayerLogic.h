/*
* PlayerLogic.cpp
*
* Created: 11/15/2015 11:14:10 AM
*  Author: Daniel Orol
*/
#pragma once

#include <stdint.h>

enum class Player : uint8_t{
	NONE = 0, GOALIE = 1, DEFENSE = 2, SCORER = 3, ASSISTER = 4
};

void playerLogic(Player player);
void goalieLogic();
void leftCorner();
void rightCorner();
void avoidGoalie();
void fakeGoalie();
void followWall();
void charge();
void bluffKick(bool corner);
void goAndKick(Pose target);
void tryKick();
void requestHelp();
void defenseLogic();
void updatePlayer(Player player);
Player getPlayer();
void scoreLogic();
void center();
void sPattern();
void crossDtoLeft();
void  crossDtoRight();
void crossOtoRight();
void crossOtoLeft();
void goBehindPuck();
void goBehindObject(Location object);
void pushGoalie();
void defenseLogic2();
void assistLogic();
void goalieLogic2();
void updateLogicTimes();