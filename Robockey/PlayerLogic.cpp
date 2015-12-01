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
#include "PlayerLogic.h"

bool helpRequested = false;

void playerLogic(Player player){
	switch(player){
		case Player::GOALIE:{
			goalieLogic();
			break;
		}
		case Player::SCORER: {
			int rando = random() % 6; //change to number of strategies
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
				case 4:{
					followWall();
					break;
				}
				case 5:{
					charge();
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
			defenseLogic();
		}
	}
}

void goalieLogic(){
	Location puckPredict = predictPuck(getTime()-getPuckUpdateTime());
	if (puckPredict.x < XMAX / 2) { //if puck closer than half field
		int16_t yPos;
		if (puckPredict.y >= 0) {
			yPos = MIN(YMAX/2,puckPredict.y);
		}
		else{
			yPos = MAX(YMIN/2,puckPredict.y);
		}
		goToPosition(Pose(XMIN + ROBOT_RADIUS, yPos,getPuckHeading()),getRobotPose(), true);
		faceLocation(puckPredict, getRobotPose());
	}
	else if(puckPredict.x < 3*XMIN/4){ //if puck closet than 3/4
		faceLocation(puckPredict, getRobotPose());
		setMotors(1600,1600); //charge
		//communicate to other robot to fill in
	}
	else {
		int16_t yPos;
		if (puckPredict.y >= 0) {
			yPos = MIN(YMAX/2,puckPredict.y);
		}
		else{
			yPos = MAX(YMIN/2,puckPredict.y);
		}
		//move out a bit
		goToPosition(Pose(7*XMIN/8, yPos, getPuckHeading()), getRobotPose(), true);
		faceLocation(puckPredict, getRobotPose());
	}
}

void leftCorner(){
	Pose currentPose = getRobotPose();
	if (currentPose.x < XMAX - ROBOT_RADIUS || currentPose.y < YMAX - ROBOT_RADIUS - 5) { //if not in corner go to corner
		goToPositionPuck(Pose(XMAX - ROBOT_RADIUS - 5, YMAX - ROBOT_RADIUS - 5, -PI/3), currentPose);
	}
	else {
		goToPositionPuck(Pose(XMAX+5,YMAX/2 - PUCK_RADIUS,0),currentPose); //charge into goal
	}
}

void rightCorner(){
	Pose currentPose = getRobotPose();
	if (currentPose.x < XMAX - ROBOT_RADIUS || currentPose.y > YMIN + ROBOT_RADIUS+ 5) {
		goToPositionPuck(Pose(XMAX - ROBOT_RADIUS - 5, YMIN + ROBOT_RADIUS + 5, -PI/3), currentPose);
	}
	else {
		goToPositionPuck(Pose(XMAX,YMIN/2 + PUCK_RADIUS,0),currentPose);
	}
}

void avoidGoalie(){
	//find if one of their players is in the goal and go towards the larger space based on his position
	Location* enemies = getEnemyLocations();
	
	//check to see if enemy has goalie in position
	bool enGoal = false;
	int16_t goalie[3] = {0,0,0};
	uint8_t count = 0;
	for (int i=0; i<3;i++){
		if (enemies[i].x > XMAX - 2*ROBOT_RADIUS){
			enGoal=true;
			goalie[i] = enemies[i].y;
			count++;
		}
	}
	if (enGoal){
		int16_t gap[3] = {0,0,0}; //store largest y1, y2, height of largest gap
		if (count == 1){
			for (int i = 0; i<3; i++){
				int16_t distance = YMAX - goalie[i];
				if (distance > gap[2]){
					gap[0] = YMAX;
					gap[1] = goalie[i];
					gap[2] = distance;
				}
			}
			if (gap[2] >=YMAX/2){ // if goalie is in bottom half of rink
				goToPositionPuck(Pose(XMAX + ROBOT_RADIUS, YMAX - (gap[0]+gap[1])*0.5, 0),getRobotPose());
			}
			else{
				goToPositionPuck(Pose(XMAX + ROBOT_RADIUS, YMIN + (gap[0]+gap[1])*0.5, 0),getRobotPose());
			}
		}
		else { //if more than one enemy goalie, find largest gap and go there
			int16_t max = 0;
			int16_t min = 0;
			for (int i = 0; i<3;i++){
				if (goalie[i] > max) max = goalie[i];
				else if (goalie[i] < min) min = goalie[i];
			}
			//check gaps to wall
			if(YMAX - max > abs(YMIN-min)){
				gap[0] = YMAX;
				gap[1] = max;
				gap[2] = YMAX - max;
			}
			else{
				gap[0] = YMIN;
				gap[1] = min;
				gap[2] = abs(YMIN - min);
			}
			
			//check gaps between robots
			//assume their robots aren't exactly at 0 to prevent array errors from initilalizing array at 0
			if(goalie[0] - goalie[1] > gap[2] && goalie[0] != 0 && goalie[1] !=0){
				gap[0] = goalie[0];
				gap[1] = goalie[1];
				gap[2] = goalie[0]-goalie[1];
			}
			else if(goalie[0] - goalie[2] > gap[2] && goalie[0] != 0 && goalie[2] !=0){
				gap[0] = goalie[0];
				gap[1] = goalie[2];
				gap[2] = goalie[0]-goalie[2];
			}
			else if(goalie[1] - goalie[2] > gap[2] && goalie[1] != 0 && goalie[2] !=0){
				gap[0] = goalie[1];
				gap[1] = goalie[2];
				gap[2] = goalie[1]-goalie[2];
			}
			goToPositionPuck(Pose(XMAX+ROBOT_RADIUS,(gap[0]+gap[1])*0.5,0),getRobotPose());
		}
	}
	else goToPositionPuck(Pose(XMAX+ROBOT_RADIUS,getRobotPose().y,0),getRobotPose());
}

void fakeGoalie(){
	//do same as avoid goalie but move other direction then turn quickly
}

void followWall(){
	if(!nearWall(getRobotPose())){
		if(getRobotPose().y >=0){
			goToPositionPuck(Pose(getRobotPose().x+ROBOT_RADIUS,YMAX,0), getRobotPose());
		}
		else goToPositionPuck(Pose(getRobotPose().x+ROBOT_RADIUS,YMIN,0), getRobotPose());
	}
	else{
		setMotors(1600,1600); //full steam ahead
	}
}

void charge(){
	goToPositionPuck(Pose(XMAX+ROBOT_RADIUS,getRobotPose().y,0),getRobotPose());
	if (getRobotPose().x > XMAX - ROBOT_RADIUS){
		startKick();
	}
	updateKick();
}

//Should be called in the move with puck
void tryKick(){
	Pose currentPose = getRobotPose();
	uint8_t dMax = ROBOT_RADIUS + 20;
	uint8_t dX = XMAX - currentPose.x;
	float dL = dX/cosb(currentPose.o);
	if(dL <= dMax){
		int16_t dY = dL * sinb(currentPose.o);
		int16_t goalY = currentPose.y + dY;
		if((goalY <= (YMAX/2 - PUCK_RADIUS)) && (goalY >= (YMIN/2 + PUCK_RADIUS))){	
			Location target = Location(currentPose.x, goalY);
			//if(!checkIntersection(currentPose.getLocation(), target, PUCK_RADIUS)){
				//startKick();
				//setLED(LEDColor::BLUE);
			//}	
		}
		//else
			//setLED(LEDColor::RED);
	}
	//else
		//setLED(LEDColor::PURPLE);
}
	
void defenseLogic(){
	if(helpRequested){
		if(getRobotPose().x>-110){
			Location puckPredict = predictPuck(getTime()-getPuckUpdateTime());
			int16_t yPos;
			if (puckPredict.y >= 0) {
				yPos = MIN(YMAX/2,puckPredict.y);
			}
			else{
				yPos = MAX(YMIN/2,puckPredict.y);
			}
			goToPosition(Pose(XMIN + ROBOT_RADIUS, yPos,getPuckHeading()),getRobotPose(), true);
			faceLocation(puckPredict, getRobotPose());
		}
		else{
			helpRequested = false;
		}
	}
	else{ //get in front of puck
		Location puck = getPuckLocation();
		goToPosition(Pose(puck.x-2*ROBOT_RADIUS,puck.y,0),getRobotPose(),true);
	}
}
void requestHelp(){
	helpRequested = true;
}