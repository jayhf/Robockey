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

void goalieLogic();
void leftCorner();
void rightCorner();
void avoidGoalie();
void fakeGoalie();
void followWall();
void charge();
void kick();
bool helpRequested = false;
bool inCorner = false;
Player player = Player::NONE;
Location lastPuck = getPuckLocation();
int i = 0;
int rando = 0;

void updatePlayer(Player play){
	player = play;
}

Player getPlayer(){
	return player;
}

void playerLogic(Player player){
	switch(player){
		case Player::NONE:
		break;
		case Player::GOALIE:{
			goalieLogic();
			break;
		}
		case Player::SCORER: {
			scoreLogic();
			break;
		}
		case Player::ASSISTER:{
			break;
		}
		case Player::DEFENSE:{
			defenseLogic();
			break;
		}
	}
}
Location a;
void goalieLogic(){
	if(puckVisible()){
		Location puck = getPuckLocation();
		Location temp = puck;
		//predictPuck(getTime()-getPuckUpdateTime());
		if (puck != UNKNOWN_LOCATION){
			puck.x=temp.x*0.90+lastPuck.x*0.10;
			puck.y=temp.y*0.90+lastPuck.y*0.10;
			if(puck.x < XMIN+4*ROBOT_RADIUS+2*PUCK_RADIUS){ //if puck closer than 3/4
				if(!facingLocation(puck,getRobotPose())){
					faceLocation(puck, getRobotPose());
				}
				else{
					setMotors(600,600);
				}
				//communicate to other robot to fill in
			}
			else if (puck.x < 0) { //if puck closer than half field
				int16_t yPos;
				if (puck.y >= 0) {
					yPos = MIN(YMAX/2,puck.y);
				}
				else{
					yPos = MAX(YMIN/2,puck.y);
				}
				if(getRobotPose().x<XMIN+4*ROBOT_RADIUS&&getRobotPose().y<yPos+ROBOT_RADIUS&&getRobotPose().y>yPos-ROBOT_RADIUS){
					faceLocation(puck, getRobotPose());
				}
				else{
					goToPosition(Pose(XMIN + 3*ROBOT_RADIUS, yPos,getPuckHeading()),getRobotPose());
				}
			}
			else {
				if(getRobotPose().x<XMIN+5*ROBOT_RADIUS&&getRobotPose().y<ROBOT_RADIUS&&getRobotPose().y>-ROBOT_RADIUS){
					faceLocation(puck, getRobotPose());
				}
				else{
					goToPosition(Pose(XMIN+4*ROBOT_RADIUS,0,0), getRobotPose());
				}
			}
		}
		else{
			goToPosition(Pose(-105,0,0),getRobotPose());
		}
		lastPuck = temp;
	}
	
}

void leftCorner(){
	Pose currentPose = getRobotPose();
	/*if (currentPose.x < XMAX - 3*ROBOT_RADIUS || currentPose.y < YMAX - 3*ROBOT_RADIUS) { //if not in corner go to corner
	setLED(LEDColor::RED);
	goToPositionPuck(Pose(XMAX - 3*ROBOT_RADIUS, YMAX - 3*ROBOT_RADIUS, -PI/3), currentPose);
	}
	else {
	inCorner = true;
	}
	if(inCorner){
	setLED(LEDColor::BLUE);
	goToPositionPuck(Pose(XMAX,YMAX/2 + ROBOT_RADIUS,0),currentPose); //charge into goal
	if(currentPose.x > XMAX - ROBOT_RADIUS-5 && (currentPose.y > YMAX/2 + ROBOT_RADIUS - 2 && currentPose.y < YMAX/2 + ROBOT_RADIUS + 2)){
	inCorner = false;
	}
	}*/
	goToPositionPuck(Pose(XMAX,YMAX/2,0),currentPose); //charge into goal
}

void rightCorner(){
	Pose currentPose = getRobotPose();
	/*if (currentPose.x < XMAX - 3*ROBOT_RADIUS || currentPose.y > YMIN + 3*ROBOT_RADIUS) {
	goToPositionPuck(Pose(XMAX - 3*ROBOT_RADIUS, YMIN + 3*ROBOT_RADIUS, -PI/3), currentPose);
	}
	else {
	inCorner = true;
	}
	if(inCorner){
	goToPositionPuck(Pose(XMAX,YMIN/2 + ROBOT_RADIUS,0),currentPose); //charge into goal
	if(currentPose.x > XMAX - ROBOT_RADIUS && (currentPose.y > YMIN/2 + ROBOT_RADIUS - 2 && currentPose.y < YMIN/2 + ROBOT_RADIUS + 2)){
	inCorner = false;
	}
	}*/
	goToPositionPuck(Pose(XMAX,YMIN/2,0),currentPose); //charge into goal
}

/*void avoidGoalie(){
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
}*/

void fakeGoalie(){
	//do same as avoid goalie but move other direction then turn quickly
}

void followWall(){
	if(getRobotPose().x>0){
		if(!nearWall(getRobotPose())){
			
			if(getRobotPose().y >=0){
				goToPositionPuck(Pose(getRobotPose().x+2*ROBOT_RADIUS,YMAX-2*ROBOT_RADIUS,0), getRobotPose());
			}
			else goToPositionPuck(Pose(getRobotPose().x+2*ROBOT_RADIUS,YMIN+2*ROBOT_RADIUS,0), getRobotPose());
		}
		else{
			if(getRobotPose().y >=0){
				faceAngle(-PI/6,getRobotPose());
			}
			else{
				faceAngle(PI/6,getRobotPose());
			}
			setMotors(900,900); //full steam ahead
		}
	}
	else{
		goToPosition(Pose(XMAX,0,0),getRobotPose());
	}
}

void charge(){
	goToPositionPuck(Pose(XMAX,getRobotPose().y+2*ROBOT_RADIUS,0),getRobotPose());
	if (getRobotPose().x > XMAX - 4*ROBOT_RADIUS){
		startKick();
	}
	updateKick();
}

//Should be called in the move with puck
void tryKick(){
	Pose currentPose = getRobotPose();
	uint8_t dMax = ROBOT_RADIUS + 20;
	uint8_t dX = XMAX - currentPose.x;
	if(dX <= dMax){
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
}

void defenseLogic(){
	if(helpRequested){
		if(getRobotPose().x>XMIN+4*ROBOT_RADIUS){
			Location puck = getPuckLocation();
			int16_t yPos;
			if (puck.y >= 0) {
				yPos = MIN(YMAX/2,puck.y);
			}
			else{
				yPos = MAX(YMIN/2,puck.y);
			}
			goToPosition(Pose(XMIN + 3*ROBOT_RADIUS, yPos,getPuckHeading()),getRobotPose());
			if(getRobotPose().x<XMIN+4*ROBOT_RADIUS&&getRobotPose().y<yPos+2*ROBOT_RADIUS&&getRobotPose().y>yPos-2*ROBOT_RADIUS){
				faceLocation(puck, getRobotPose());
			}
		}
		else{
			helpRequested = false;
		}
	}
	else{ //get in front of puck
		if(puckVisible()){
			Location puck = getPuckLocation();
			if(puck.x>0){
				scoreLogic();
			}
			else{
				goToPuck(puck.toPose(getPuckHeading()),getRobotPose());
			}
		}
		else{
			goToPosition(Pose(-100,25,0),getRobotPose());
		}
	}
}
void requestHelp(){
	helpRequested = true;
}

void scoreLogic(){
	if(puckVisible()){
		if(!hasPuck()){
			goToPuck(getPuckLocation().toPose(getPuckHeading()),getRobotPose());
		}
		else{
			
			if (i==0){
				rando = rand() % 3;
				i++;
			} //change to number of strategies
			else if (i==500){
				i=0;
			}
			else i++;
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
					followWall();
					break;
				}
				case 3:{
					charge();
					break;
				}
				default:{
					break;
				}
			}
		}
	}
	else{
		goToPosition(Pose(-100,-25,0),getRobotPose());
	}
}

void faceoff(){
	if(player==Player::SCORER){
		if(!timePassed(1000)){
			setMotors(1200,1200);
		}
		else{
			goToPuck(getPuckLocation().toPose(getPuckHeading()),getRobotPose());
		}
	}
	else if(player==Player::ASSISTER){
		goToPuck(getPuckLocation().toPose(getPuckHeading()),getRobotPose());
	}
	else playerLogic(player);
}