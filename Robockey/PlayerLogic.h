/*
* PlayerLogic.cpp
*
* Created: 11/15/2015 11:14:10 AM
*  Author: Daniel Orol
*/

#include <stdint.h>
#include <stdlib.h>
#include "Localization.h"
#include "PathPlanning.h"
#include "BAMSMath.h"
#include "Digital.h"


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
void requestHelp();
void defenseLogic();