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
#include "Strategies.h"
#include "wireless.h"
#include "miscellaneous.h"
#define F_CPU 16000000
#include <util/delay.h>


bool helpRequested = false;
bool needHelp = false;
bool inCorner = false;
Player player = Player::NONE;
Location lastPuck = getPuckLocation();
bool point1 = false;
bool point2 = false;
bool targetSet = false;
Pose targetPose;

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
			goalieLogic3();
			break;
		}
		case Player::SCORER: {
			scoreLogic();
			break;
		}
		case Player::ASSISTER:{
			assistLogic();
			break;
		}
		case Player::DEFENSE:{
			defenseLogic3();
			break;
		}
	}
}

void goalieLogic(){
	needHelp = false;
	if(puckVisible()){
		Location puck = getPuckLocation();
		if (puck != UNKNOWN_LOCATION){
			if(puck.x < XMIN+5*ROBOT_RADIUS+PUCK_RADIUS){ //if puck closer than 3/4
				if(puck.x<getRobotPose().x-2*ROBOT_RADIUS){
					goToPositionSpin(Pose(XMIN+ROBOT_RADIUS,puck.y/2,0),getRobotPose());
				}
				else{
					goToPosition(puck.toPose(getPuckHeading()+getRobotPose().o),getRobotPose(),true,false);
					if(hasPuck()&&getRobotPose().x>XMIN+3*ROBOT_RADIUS) startKick();
				}
				//communicate to other robot to fill in
				needHelp = true;
			}
			else if (puck.x < 0) { //if puck closer than half field
				int16_t yPos;
				if (puck.y >= 0) {
					yPos = MIN(YMAX/2,puck.y);
				}
				else{
					yPos = MAX(YMIN/2,puck.y);
				}
				if(yPos>getRobotPose().y+ROBOT_RADIUS) goToPosition(Pose(XMIN + 2*ROBOT_RADIUS, yPos,getPuckHeading()+getRobotPose().o),getRobotPose(),false,false);
				else goToPosition(Pose(XMIN + 2*ROBOT_RADIUS, yPos,getPuckHeading()+getRobotPose().o),getRobotPose(),false,true);
			}
			else {
				if(getRobotPose().x<XMIN+5*ROBOT_RADIUS&&getRobotPose().y<2*ROBOT_RADIUS&&getRobotPose().y>-2*ROBOT_RADIUS){
					faceLocation(puck, getRobotPose());
				}
				else{
					goToPositionSpin(Pose(XMIN+3*ROBOT_RADIUS,0,0), getRobotPose());
				}
			}
		}
		else{
			goToPositionSpin(Pose(XMIN+3*ROBOT_RADIUS,0,0),getRobotPose());
			if(getRobotPose().x<XMIN+4*ROBOT_RADIUS&&getRobotPose().x>XMIN+2*ROBOT_RADIUS) faceAngle(0,getRobotPose());
		}
	}
	else{
		goToPositionSpin(Pose(XMIN+3*ROBOT_RADIUS,0,0),getRobotPose());
	}
}

void goalieLogic2(){
	if(puckVisible()){
		Location puck = getPuckLocation();
		if(puck != UNKNOWN_LOCATION){
			if(puck.x<XMIN+7*ROBOT_RADIUS){
				goToPosition(puck.toPose(getPuckHeading()+getRobotPose().o),getRobotPose(),true,false,1400);
			}
			else{
				if(puck.y>getRobotPose().y){
					goToPosition(Pose(XMIN+2*ROBOT_RADIUS,MIN(YMAX/2+PUCK_RADIUS,puck.y),0),getRobotPose(),false,false,1400);
				}
				else{
					goToPosition(Pose(XMIN+2*ROBOT_RADIUS,MAX(YMIN/2-PUCK_RADIUS,puck.y),0),getRobotPose(),false,true,1400);
					
				}
			}
		}
		else{
			goToPositionSpin(Pose(XMIN+3*ROBOT_RADIUS,0,0),getRobotPose());
			if(getRobotPose().x<XMIN+4*ROBOT_RADIUS&&getRobotPose().x>XMIN+2*ROBOT_RADIUS) faceAngle(0,getRobotPose());
		}
	}
	else{
		goToPositionSpin(Pose(XMIN+3*ROBOT_RADIUS,0,0),getRobotPose());
	}
}

void rightCorner(){
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

	goToPositionPuck(Pose(XMAX+5,YMIN/2+2*ROBOT_RADIUS,0),currentPose); //charge into goal

}

void leftCorner(){
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

	goToPositionPuck(Pose(XMAX+5,YMAX/2-2*ROBOT_RADIUS,0),currentPose); //charge into goal

}
void center(){
	Pose currentPose = getRobotPose();
	goToPositionPuck(Pose(XMAX+5,0,0),currentPose);
}

void sPattern(){
	Pose currentPose = getRobotPose();
	if(currentPose.x<XMAX-6*ROBOT_RADIUS&&(!point1||!point2)){
		if(!targetSet){
			if(currentPose.y<=0 && !point1){
				targetPose = Pose(currentPose.x+6*ROBOT_RADIUS,YMAX-5*ROBOT_RADIUS,0);
				targetSet = true;
			}
			else if(!point1){
				targetPose = Pose(currentPose.x+6*ROBOT_RADIUS,YMIN+5*ROBOT_RADIUS,0);
				targetSet = true;
			}
			else if (currentPose.y>=0&&!point2){
				targetPose = Pose(currentPose.x+8*ROBOT_RADIUS,YMIN+5*ROBOT_RADIUS,0);
				targetSet = true;
			}
			else if(!point2){
				targetPose = Pose(currentPose.x+8*ROBOT_RADIUS,YMAX-5*ROBOT_RADIUS,0);
				targetSet = true;
			}
			else{
				targetPose = Pose(XMAX+5,0,0);
				targetSet = true;
			}
		}
		if(!atLocationWide(Location(targetPose.x,targetPose.y),Location(currentPose.x,currentPose.y))){
			goToPositionPuck(targetPose,currentPose);
		}
		else if (!point1){
			point1 = true;
			targetSet = false;
		}
		else if (!point2){
			point2 = true;
			targetSet = false;
		}
		else{
			targetPose = Pose(XMAX+5,0,0);
		}
	}
	else goToPositionPuck(Pose(XMAX+5,0,0),currentPose);
	//uint8_t packet[10]={0,0,targetPose.x,targetPose.y,targetPose.o>>8,targetPose.o&0xFF,0,0,0,0};
	//sendPacket(Robot::CONTROLLER,0x22,packet);
	
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
	uint8_t yPos;
	if(getRobotPose().y >=0) yPos = YMAX-ROBOT_RADIUS-2*PUCK_RADIUS;
	else yPos = YMIN+ROBOT_RADIUS+2*PUCK_RADIUS;
	if(!nearWall(getRobotPose())){
		if(!targetSet){
			targetPose = Pose(getRobotPose().x+4*ROBOT_RADIUS,yPos,0);
			targetSet = true;
		}
		if(!atLocation(Location(targetPose.x,targetPose.y),Location(getRobotPose().x,getRobotPose().y))){
			goToPositionPuck(Pose(targetPose.x,targetPose.y,0), getRobotPose());
		}
	}
	else{
		if(getRobotPose().x<XMAX-5*ROBOT_RADIUS){
			goToPositionPuck(Pose(getRobotPose().x+2*ROBOT_RADIUS,yPos,0),getRobotPose());
		}
		else{
			if(getRobotPose().y>0) goToPositionPuck(Pose(XMAX+5,YMAX/2-ROBOT_RADIUS,0),getRobotPose());
			else goToPositionPuck(Pose(XMAX+5,YMIN/2+ROBOT_RADIUS,0),getRobotPose());
		}
	}

}

void charge(){
	goToPositionPuck(Pose(XMAX,getRobotPose().y,0),getRobotPose());
}

void bluffKick(bool corner){
	int yTarget = 20;
	int xBluff = XMAX - 40;
	if(!corner){
		yTarget = -yTarget;
	}
	if(hasPuck()){
		if(getRobotPose().x < xBluff){
			goToPositionPuck(Pose(xBluff, -yTarget, -PI/2), getRobotPose());
		}
		else{
			if(facingLocation(Location(XMAX, yTarget),getRobotPose())){
				tryKick();
				goToPositionPuck(Pose(XMAX,yTarget,-PI/2),getRobotPose());
			}
			else{
				faceLocation(Location(XMAX, yTarget),getRobotPose());
			}
		}
	}
	else{
		goToPosition(getPuckLocation().toPose(getPuckHeading()+getRobotPose().o),getRobotPose(),false);
	}
}

void goAndKick(Pose target){
	if(hasPuck()){
		tryKick();
		goToPositionPuck(target, getRobotPose());
	}
	else{
		goToPosition(getPuckLocation().toPose(getPuckHeading()+getRobotPose().o),getRobotPose(),true);
	}
}

void goBehindPuck(){
	Pose puck = getPuckLocation().toPose(getPuckHeading()+getRobotPose().o);
	if(puck.x<getRobotPose().x) {
		point1 = false;
		point2 = false;
	}
	else if (puck.x>getRobotPose().x+ROBOT_RADIUS){
		point1 = true;
	}
	if(!point1){
		if(puck.y>YMAX/2) targetPose = Pose(MAX(puck.x-2*ROBOT_RADIUS,XMIN+ROBOT_RADIUS),puck.y-2*ROBOT_RADIUS,puck.o);
		else if (puck.y>0) targetPose = Pose(MAX(puck.x-2*ROBOT_RADIUS,XMIN+ROBOT_RADIUS),MIN(puck.y+2*ROBOT_RADIUS,YMAX),puck.o);
		else if (puck.y < YMIN/2) targetPose = Pose(MAX(puck.x-2*ROBOT_RADIUS,XMIN+ROBOT_RADIUS),puck.y+2*ROBOT_RADIUS,puck.o);
		else targetPose = Pose(MAX(puck.x-2*ROBOT_RADIUS,XMIN+ROBOT_RADIUS),MAX(puck.y-2*ROBOT_RADIUS,YMIN),puck.o);
		if(!atLocationWide(Location(targetPose.x,targetPose.y),Location(getRobotPose().x,getRobotPose().y))){
			goToPosition(targetPose,getRobotPose(),false);
		}
		else{
			point1 = true;
		}
	}
	else if(!point2) {
		if(!atLocationWide(Location(puck.x-ROBOT_RADIUS-PUCK_RADIUS,puck.y),Location(getRobotPose().x,getRobotPose().y))){
			goToPosition(Pose(puck.x-ROBOT_RADIUS-PUCK_RADIUS,puck.y,0),getRobotPose(),false);
		}
		else point2 = true;
	}
	else goToPosition(puck,getRobotPose(),true);
}

void goBehindObject(Location object){
	Pose puck = object.toPose(0);
	if(puck.x<getRobotPose().x-ROBOT_RADIUS) point1 = false;
	else if (puck.x>getRobotPose().x+ROBOT_RADIUS) point1 = true;
	if(!point1){
		if(puck.y>=0)
		targetPose = Pose(MAX(puck.x-4*ROBOT_RADIUS,XMIN+ROBOT_RADIUS),puck.y-3*ROBOT_RADIUS,puck.o);
		else
		targetPose = Pose(MAX(puck.x-4*ROBOT_RADIUS,XMIN+ROBOT_RADIUS),puck.y+3*ROBOT_RADIUS,puck.o);
		if(!atLocation(Location(targetPose.x,targetPose.y),Location(getRobotPose().x,getRobotPose().y))){
			goToPosition(targetPose,getRobotPose(),false);
		}
		else{
			point1 = true;
		}
	}
	else{
		goToPosition(puck,getRobotPose(),true);
	}
}

//Should be called in the move with puck
void tryKick(){
	Pose currentPose = getRobotPose();
	uint8_t dMax = ROBOT_RADIUS + 70;
	uint8_t dXMax = ROBOT_RADIUS + PUCK_RADIUS + 50;
	uint8_t dX = XMAX - currentPose.x;
	if(dX <= dXMax){
		float dL = dX/cosb(currentPose.o);
		if(dL <= dMax){
			int16_t dY = dL * sinb(currentPose.o);
			int16_t goalY = currentPose.y + dY;
			int yMax = (YMAX/2) - 5;
			int yMin = (YMIN/2) + 5;
			if((goalY <= yMax - PUCK_RADIUS) && (goalY >= yMin + PUCK_RADIUS)){
				Location target = Location(currentPose.x, goalY);
				if(currentPose.y > yMax - PUCK_RADIUS || currentPose.y < yMin + PUCK_RADIUS){
					Location corner = Location(XMAX,yMax);
					if(currentPose.y < 0){
						corner = Location(XMAX,yMin);
					}
					if(!circleIntersectsSegment(currentPose.getLocation(),Location(XMAX, goalY), corner, PUCK_RADIUS)){
						startKick();
					}
				}
				else{
					startKick();
				}
			}
		}
	}
}

void crossDtoLeft(){
	//get signal to send left or right. go to 5 times radius of ally position and then cross
	if(!point1){
		if(!targetSet) {
			Location* allies = getAllyLocations();
			uint8_t xPos;
			if (hasPuck(Ally::ALLY1)) xPos = allies[0].x+6*ROBOT_RADIUS;
			else if(hasPuck(Ally::ALLY2)) xPos = allies[1].x+6*ROBOT_RADIUS;
			else xPos = 50;
			targetPose = Pose(MIN(XMAX,xPos),YMIN/2,0);
		}
		if(!atLocationWide(Location(targetPose.x,targetPose.y),Location(getRobotPose().x,getRobotPose().y))){
			goToPosition(targetPose,getRobotPose(),false);
		}
		else {
			point1 = true;
		}
	}
	else{
		goToPosition(Pose(XMAX - 4*ROBOT_RADIUS,YMAX/2,0),getRobotPose(),false);
	}
}

void crossDtoRight(){
	//get signal to send left or right. go to 5 times radius of ally position and then cross
	if(!point1){
		if(!targetSet) {
			Location* allies = getAllyLocations();
			uint8_t xPos;
			if (hasPuck(Ally::ALLY1)) xPos = allies[0].x+6*ROBOT_RADIUS;
			else if(hasPuck(Ally::ALLY2)) xPos = allies[1].x+6*ROBOT_RADIUS;
			else xPos = 50;
			targetPose = Pose(MIN(XMAX,xPos),YMAX/2,0);
		}
		if(!atLocationWide(Location(targetPose.x,targetPose.y),Location(getRobotPose().x,getRobotPose().y))){
			goToPosition(targetPose,getRobotPose(),false);
		}
		else {
			point1 = true;
		}
	}
	else{
		goToPosition(Pose(XMAX - 4*ROBOT_RADIUS,YMIN/2,0),getRobotPose(),false);
	}
}

void crossOtoRight(){
	//get signal to send left or right. go to 5 times radius of ally position and then cross
	if(!point1){
		if(!targetSet) {
			targetPose = Pose(MIN(XMAX,getRobotPose().x+12*ROBOT_RADIUS),YMAX/2,0);
			targetSet = true;
		}
		if(!atLocationWide(Location(targetPose.x,targetPose.y),Location(getRobotPose().x,getRobotPose().y))){
			goToPositionPuck(targetPose,getRobotPose());
		}
		else {
			point1 = true;
		}
	}
	else{
		goToPositionPuck(Pose(XMAX,YMIN/2+4*ROBOT_RADIUS,0),getRobotPose());
	}
}

void crossOtoLeft(){
	//get signal to send left or right. go to 5 times radius of ally position and then cross
	if(!point1){
		if(!targetSet) {
			targetPose = Pose(MIN(XMAX,getRobotPose().x+12*ROBOT_RADIUS),YMIN/2,0);
			targetSet = true;
		}
		if(!atLocationWide(Location(targetPose.x,targetPose.y),Location(getRobotPose().x,getRobotPose().y))){
			goToPositionPuck(targetPose,getRobotPose());
		}
		else {
			point1 = true;
		}
	}
	else{
		goToPositionPuck(Pose(XMAX,YMAX/2-4*ROBOT_RADIUS,0),getRobotPose());
	}
}

void requestHelp(){
	helpRequested = true;
}
int i = 0;
int rando = 0;
time lastNewStrategyTime = -3 * ONE_SECOND;
void scoreLogic(){
	if(puckVisible()&& getPuckLocation()!=UNKNOWN_LOCATION){
		if(!hasPuck()){
			goBehindPuck();
		}
		else{
			
			if(timePassed(lastNewStrategyTime+3*ONE_SECOND)){
				lastNewStrategyTime = getTime();
				srand(getTime()^rand());
				rando = rand() % 6;
				point1 = false;
				point2 = false;
				targetSet = false;
			} //change to number of strategies
			
			switch(rando){
				case 0:{
					leftCorner();
					tryKick();
					break;
				}
				case 1:{
					rightCorner();
					tryKick();
					break;
				}
				case 2:{
					center();
					tryKick();
					break;
				}
				case 3:{
					charge();
					tryKick();
					break;
				}
				case 4:{
					crossOtoLeft();
					tryKick();
					break;
				}
				case 5:{
					crossOtoRight();
					tryKick();
					break;
				}
				case 6:{
					sPattern();
					tryKick();
					break;
				}
				case 7:{
					followWall();
					tryKick();
					break;
				}
				default:{
					tryKick();
					break;
					
				}
			}
		}
	}
	else{
		goToPosition(Pose(XMIN+4*ROBOT_RADIUS,-25,0),getRobotPose(),false);
	}
}

void updateLogicTimes(){
	if(timePassed(lastNewStrategyTime+3*ONE_SECOND)){
		lastNewStrategyTime = getTime()-3*ONE_SECOND -1;
	}
}

void faceoff(){
	Pose puck = getPuckLocation().toPose(getPuckHeading()+getRobotPose().o);
	if(player==Player::SCORER){
		goToPosition(puck,getRobotPose(),true);
	}
	else if(player==Player::DEFENSE){
		goToPosition(Pose(puck.x,10,0),getRobotPose(),false);
	}
	else playerLogic(player);
}

//returns true if finished
bool pushGoalie(){
	if(!point1){
		if(!atLocationWide(Location(XMAX-ROBOT_RADIUS-PUCK_RADIUS,YMAX/2),Location(getRobotPose().x,getRobotPose().y))){
			goToPosition(Pose(XMAX-ROBOT_RADIUS-PUCK_RADIUS,YMAX/2,0),getRobotPose(),false);
		}
		else point1 = true;
	}
	else if(!point2){
		if(!facingHeading(-PI/2,getRobotPose())){
			faceAngle(-PI/2,getRobotPose());
		}
		else {
			point2 = true;
		}
		
	}
	else if(getRobotPose().y > YMIN + 4*ROBOT_RADIUS && abs(getRobotPose().o+PI/2) < PI/8){
		if(recentlyMoved())
		setMotors(800,800);
		else
		setMotors(1600,1600);
	}
	else{
		setMotors(0,0);
		return true;
	}
	return false;
}

void defenseLogic2(){
	if(getPuckLocation()!=UNKNOWN_LOCATION){
		if(!hasPuck(Ally::ALLY1)&&!hasPuck(Ally::ALLY2)){
			goBehindPuck();
		}
		else {
			assistLogic();
		}
	}
	else goToPosition(Pose(XMIN+4*ROBOT_RADIUS,25,0),getRobotPose(),false);
}
void assistLogic(){
	pushGoalie();
}
//Returns true if it should stop doing this strategy
bool pushAlly(){
	/*Location allyLocation;
	if(getAllySuggestedStrategy(Ally::ALLY1) == Strategy::PUSH_ALLY){
	allyLocation = getAllyLocations()[0];
	}
	else if(getAllySuggestedStrategy(Ally::ALLY2) == Strategy::PUSH_ALLY){
	allyLocation = getAllyLocations()[1];
	}*/
	//else
	//	return true;
	goBehindObject(getAllyLocations()[1]);
	return false;
}

void resetPoints(){
	point1 = false;
	point2 = false;
	targetSet = false;
}

void goalieLogicJ(){
	Location puck = getPuckLocation();
	Pose robot = getRobotPose();
	if(puck.x < XMIN/2){
		if(robot.distanceToSquared(puck) < 225)
		goToPosition(puck.toPose(0),robot,true);
		else{
			Pose blockPosition = puck.toPose(0);
			blockPosition.x = (blockPosition.x>>1) + (XMIN>>1);
			goToPosition(blockPosition,robot,false);
		}
	}
	else{
		int8_t targetY = puck.y/2;
		if(robot.y > targetY + 5)
		goToPosition(Pose(XMIN+ROBOT_RADIUS*2,YMIN,0),robot,true);
		else if(robot.y < targetY - 5)
		goToPosition(Pose(XMIN+ROBOT_RADIUS*2,YMAX,0),robot,false);
		else
		setMotors(0,0);
	}
}

void goalieLogic3(){
	Location puck = getPuckLocation();
	Pose robot = getRobotPose();
	if(puck != UNKNOWN_LOCATION){
		int16_t xPos;
		if(abs(getRobotPose().o) > PI/3) xPos = XMIN;
		else xPos = XMIN+2*ROBOT_RADIUS;
		if (puck.y>robot.y) goToPosition2(Pose(xPos,puck.y+PUCK_RADIUS,0),robot,false,false,900);
		else if (puck.y<robot.y) goToPosition2(Pose(xPos,puck.y-PUCK_RADIUS,0),robot,false,true,900);
		else setMotors(0,0);
	}
	else goToPosition(Pose(XMIN+2*ROBOT_RADIUS,0,0),getRobotPose(),false,true,900);
}

void goalieLogic4(){
	Location puck = getPuckLocation();
	Pose robot = getRobotPose();
	if(puck != UNKNOWN_LOCATION){
		uint8_t threshold = 2;
		if(puck.y>(robot.y + threshold)){
			if(robot.y <= (YMAX/2)){
				goToPosition2(Pose(XMIN+2*ROBOT_RADIUS,YMAX,0),robot,false,false,1000);
			}
			else{
				setMotors(0,0);
			}
		}
		else if(puck.y<(robot.y - threshold)){
			if(robot.y >= (YMIN/2)){
				goToPosition2(Pose(XMIN+2*ROBOT_RADIUS,YMIN,0),robot,false,true,1000);
			}
			else{
				setMotors(0,0);
			}
		}
		else{
			setMotors(0,0);
		}
	}
	else{
		goToPosition(Pose(XMIN+2*ROBOT_RADIUS,0,0),getRobotPose(),false,true,900);
	}
}

void defenseLogic3(){
	if(getPuckLocation()!=UNKNOWN_LOCATION){
		Location* allies = getAllyLocations();
		if(!hasPuck(Ally::ALLY2)&&!hasPuck(Ally::ALLY1)){
			if(getPuckLocation().x<XMIN+6*ROBOT_RADIUS){
				goBehindPuck();
			}
			else {
				if ((abs(allies[0].x-getPuckLocation().x)>10&&abs(allies[0].y<getPuckLocation().y)>10)&&(abs(allies[1].y-getPuckLocation().x)>10&&abs(allies[1].y<getPuckLocation().y)>10)){
					scoreLogic();
				}
				else goToPosition(Pose(MAX(allies[0].x,allies[1].x)-3*ROBOT_RADIUS,getPuckLocation().y,0),getRobotPose(),false,false);
			}
		}
		else {
			assistLogic();
		}
	}
	else goToPosition(Pose(XMIN+4*ROBOT_RADIUS,25,0),getRobotPose(),false);
}