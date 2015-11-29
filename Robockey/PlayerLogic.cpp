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

#define robotRadius 10
#define puckRadius 3


enum class Player : uint8_t{
	GOALIE = 0, DEFENSE = 1, SCORER = 2, ASSISTER = 3
};

void goalieLogic();
void leftCorner();
void rightCorner();
void avoidGoalie();
void fakeGoalie();

void playerLogic(Player player){
	switch(player){
		case Player::GOALIE:{
			goalieLogic();
			break;
		}
		case Player::SCORER: {
			int rando = random() % 4; //change to number of strategies
			switch(rando){
				case 0:{
					leftCorner();
					break;
				}
				case 1:{
					rightCorner();
					break;
				}
				case 2:{
					avoidGoalie();
					break;
				}
				case 3:{
					fakeGoalie();
					break;
				}
				default:{
					break;
				}
			}
			break;
		}
		case Player::ASSISTER:{
			
		}
		case Player::DEFENSE:{
			
		}
	}
}

void goalieLogic(){
	Location puckPredict = predictPuck(getTime()-getPuckUpdateTime());
	if (puckPredict.x < XMAX / 2) {
		uint16_t yPos;
		if (puckPredict.y >= 0) {
			yPos = MIN(YMAX/2,puckPredict.y);
		}
		else{
			yPos = MAX(YMIN/2,puckPredict.y);
		}
		goToPosition(Pose(XMIN + robotRadius, yPos,getPuckHeading()),getRobotPose(), true);
		faceLocation(puckPredict, getRobotPose());
	}
	else if(puckPredict.x < 3*XMIN/4){
		faceLocation(puckPredict, getRobotPose());
		movement(100,100);
		//communicate to other robot to fill in
	}
	else {
		uint16_t yPos;
		if (puckPredict.y >= 0) {
			yPos = MIN(YMAX/2,puckPredict.y);
		}
		else{
			yPos = MAX(YMIN/2,puckPredict.y);
		}
		goToPosition(Pose(7*XMIN/8, yPos, getPuckHeading()), getRobotPose(), true);
		faceLocation(puckPredict, getRobotPose());
	}
}

void leftCorner(){
	Pose currentPose = getRobotPose();
	if (currentPose.x < XMAX - robotRadius && currentPose.y < YMAX - robotRadius - 5) {
		goToPositionPuck(Pose(XMAX - robotRadius - 5, YMAX - robotRadius - 5, -PI/3), currentPose);
	}
	else {
		goToPositionPuck(Pose(XMAX,YMAX/2 - puckRadius,0),currentPose);
	}
}

void rightCorner(){
	Pose currentPose = getRobotPose();
	if (currentPose.x < XMAX - robotRadius && currentPose.y > YMIN + robotRadius + 5) {
		goToPositionPuck(Pose(XMAX - robotRadius - 5, YMIN + robotRadius + 5, -PI/3), currentPose);
	}
	else {
		goToPositionPuck(Pose(XMAX,YMIN/2 + puckRadius,0),currentPose);
	}
}

void avoidGoalie(){
	//find if one of their players is in the goal and go towards the larger space based on his position
}

void fakeGoalie(){
	//do same as avoid goalie but move other direction then turn quickly
}
