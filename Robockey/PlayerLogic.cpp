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

void goalieLogic(){
	needHelp = false;
	if(puckVisible()){
		Location puck = getPuckLocation();
		//predictPuck(getTime()-getPuckUpdateTime());
		if (puck != UNKNOWN_LOCATION){
			if(puck.x < XMIN+5*ROBOT_RADIUS+PUCK_RADIUS){ //if puck closer than 3/4
				if(puck.x<getRobotPose().x-2*ROBOT_RADIUS){
					goToPositionSpin(Pose(XMIN+ROBOT_RADIUS,puck.y/2,0),getRobotPose());
				}
				else{
					goToPosition(puck.toPose(getPuckHeading()+getRobotPose().o),getRobotPose(),true);
				}
				//communicate to other robot to fill in
				needHelp = true;
			}
			else if (puck.x < 0) { //if puck closer than half field
				int16_t yPos;
				if (puck.y >= 0) {
					yPos = MIN(YMAX/2-ROBOT_RADIUS,puck.y);
				}
				else{
					yPos = MAX(YMIN/2+ROBOT_RADIUS,puck.y);
				}
				if(getRobotPose().x<XMIN+4*ROBOT_RADIUS&&getRobotPose().y<yPos+2*ROBOT_RADIUS&&getRobotPose().y>yPos-2*ROBOT_RADIUS){
					faceLocation(puck, getRobotPose());
				}
				else{
					goToPositionSpin(Pose(XMIN + 2*ROBOT_RADIUS, yPos,getPuckHeading()+getRobotPose().o),getRobotPose());
				}
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
		goToPosition(Pose(XMIN+3*ROBOT_RADIUS,0,0),getRobotPose(),false);
	}
}
class GoalieStrategy : public Strategy{
	private:
	bool needHelp = false;
	public:
	GoalieStrategy(uint8_t id) : Strategy(StrategyType::GOALIE, id){
		
	}

	uint8_t run(uint8_t* allyStrategies) override{
		needHelp = false;
		if(puckVisible()){
			Location puck = getPuckLocation();
			//predictPuck(getTime()-getPuckUpdateTime());
			if (puck != UNKNOWN_LOCATION){
				if(puck.x < XMIN+5*ROBOT_RADIUS+PUCK_RADIUS){ //if puck closer than 3/4
					goToPosition(puck.toPose(getPuckHeading()+getRobotPose().o),getRobotPose(),false);
					//communicate to other robot to fill in
					needHelp = true;
				}
				else if (puck.x < 0) { //if puck closer than half field
					int16_t yPos;
					if (puck.y >= 0) {
						yPos = MIN(YMAX/2-ROBOT_RADIUS,puck.y);
					}
					else{
						yPos = MAX(YMIN/2+ROBOT_RADIUS,puck.y);
					}
					if(getRobotPose().x<XMIN+4*ROBOT_RADIUS&&getRobotPose().y<yPos+2*ROBOT_RADIUS&&getRobotPose().y>yPos-2*ROBOT_RADIUS){
						faceLocation(puck, getRobotPose());
					}
					else{
						goToPosition(Pose(XMIN + 3*ROBOT_RADIUS, yPos,getPuckHeading()+getRobotPose().o),getRobotPose(),false);
					}
				}
				else {
					if(getRobotPose().x<XMIN+5*ROBOT_RADIUS&&getRobotPose().y<2*ROBOT_RADIUS&&getRobotPose().y>-2*ROBOT_RADIUS){
						faceLocation(puck, getRobotPose());
					}
					else{
						goToPosition(Pose(XMIN+4*ROBOT_RADIUS,0,0), getRobotPose(),false);
					}
				}
			}
			else{
				goToPosition(Pose(XMIN+3*ROBOT_RADIUS,0,0),getRobotPose(),false);
				if(getRobotPose().x<XMIN+4*ROBOT_RADIUS&&getRobotPose().x>XMIN+2*ROBOT_RADIUS) faceAngle(0,getRobotPose());
			}
		}
		return getID();
	}
	
	uint8_t getPriority(){
		return 0;
	}
	
};

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

	goToPosition(Pose(XMAX+5,YMIN/2+2*ROBOT_RADIUS,0),currentPose,false); //charge into goal

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
				targetPose = Pose(currentPose.x+6*ROBOT_RADIUS,YMAX-4*ROBOT_RADIUS,0);
				targetSet = true;
			}
			else if(!point1){
				targetPose = Pose(currentPose.x+6*ROBOT_RADIUS,YMIN+4*ROBOT_RADIUS,0);
				targetSet = true;
			}
			else if (currentPose.y>=0&&!point2){
				targetPose = Pose(currentPose.x+6*ROBOT_RADIUS,YMIN+4*ROBOT_RADIUS,0);
				targetSet = true;
			}
			else if(!point2){
				targetPose = Pose(currentPose.x+6*ROBOT_RADIUS,YMAX-4*ROBOT_RADIUS,0);
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
	if(!nearWall(getRobotPose())){
		if(!targetSet){
			uint8_t yPos;
			if(getRobotPose().y >=0) yPos = YMAX-ROBOT_RADIUS;
			else yPos = YMIN+ROBOT_RADIUS;
			targetPose = Pose(getRobotPose().x+4*ROBOT_RADIUS,yPos,0);
			targetSet = true;
		}
		if(!atLocation(Location(targetPose.x,targetPose.y),Location(getRobotPose().x,getRobotPose().y))){
			goToPositionPuck(Pose(targetPose.x,targetPose.y,0), getRobotPose());
		}
	}
	else{
		point1 = false;
		if(getRobotPose().y >=0){
			faceAngle(-PI/6,getRobotPose());
		}
		else{
			faceAngle(PI/6,getRobotPose());
		}
		setMotors(1600,1600); //full steam ahead
	}

}

void charge(){
	goToPositionPuck(Pose(XMAX,getRobotPose().y,0),getRobotPose());
	if (getRobotPose().x > XMAX - 3*ROBOT_RADIUS){
		startKick();
	}
	updateKick();
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
	if(puck.x<getRobotPose().x-2*ROBOT_RADIUS) point1 = false;
	if(!point1){
		if(puck.y>=0) targetPose = Pose(puck.x-3*ROBOT_RADIUS,puck.y-3*ROBOT_RADIUS,puck.o);
		else targetPose = Pose(puck.x-3*ROBOT_RADIUS,puck.y+3*ROBOT_RADIUS,puck.o);
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

void goBehindObject(Location object){
	if(object.x<getRobotPose().x-2*ROBOT_RADIUS) point1 = false;
	if(!point1){
		if(object.y>=0) targetPose = Pose(object.x-4*ROBOT_RADIUS,object.y-4*ROBOT_RADIUS,0);
		else targetPose = Pose(object.x-4*ROBOT_RADIUS,object.y+4*ROBOT_RADIUS,0);
		if(!atLocation(Location(targetPose.x,targetPose.y),Location(getRobotPose().x,getRobotPose().y))){
			goToPosition(targetPose,getRobotPose(),false);
		}
		else{
			point1 = true;
		}
	}
	else{
		goToPosition(Pose(object.x,object.y,0),getRobotPose(),false);
	}
}

//Should be called in the move with puck
void tryKick(){
	Pose currentPose = getRobotPose();
	uint8_t dMax = ROBOT_RADIUS + 40;
	uint8_t dXMax = ROBOT_RADIUS + PUCK_RADIUS + 15;
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
			goToPosition(Pose(XMIN + 3*ROBOT_RADIUS, yPos,getPuckHeading()+getRobotPose().o),getRobotPose(),false);
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
			if(puck != UNKNOWN_LOCATION){
				if(puck.x>0){
					scoreLogic();
				}
				else{
					goToPosition(puck.toPose(getPuckHeading()+getRobotPose().o),getRobotPose(),false);
				}
			}
			else goToPosition(Pose(XMIN+4*ROBOT_RADIUS,25,0),getRobotPose(),false);
		}
		else{
			goToPosition(Pose(XMIN+4*ROBOT_RADIUS,25,0),getRobotPose(),false);
			if(getRobotPose().x<XMIN+5*ROBOT_RADIUS&&getRobotPose().x>XMIN+3*ROBOT_RADIUS) faceAngle(0,getRobotPose());
		}
	}
}
void requestHelp(){
	helpRequested = true;
}
int i = 0;
int rando = 0;
void scoreLogic(){
	if(puckVisible()&& getPuckLocation()!=UNKNOWN_LOCATION){
		if(!hasPuck()){
			goToPosition(getPuckLocation().toPose(getPuckHeading()+getRobotPose().o),getRobotPose(),false);
		}
		else{
			
			if (i==0){
				rando = rand() % 3;
				i++;
			} //change to number of strategies
			else if (i==3000){
				i=0;
				point1 = false;
				point2 = false;
				targetSet = false;
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
					goToPositionPuck(Pose(XMAX+5,0,0),getRobotPose());
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
		goToPosition(Pose(XMAX-4*ROBOT_RADIUS,-25,0),getRobotPose(),false);
	}
}

void faceoff(){
	Pose puck = getPuckLocation().toPose(getPuckHeading()+getRobotPose().o);
	if(player==Player::SCORER){
		goToPosition(puck,getRobotPose(),true);
	}
	else if(player==Player::ASSISTER){
		goToPosition(Pose(puck.x-3*ROBOT_RADIUS,0,0),getRobotPose(),false);
	}
	else playerLogic(player);
}

Pose prevPose = getRobotPose();
uint16_t counter = 0;
void pushGoalie(){
	if(!point1){
		if(!atLocationWide(Location(XMAX-2*ROBOT_RADIUS,YMAX/2),Location(getRobotPose().x,getRobotPose().y))){
			goToPosition(Pose(XMAX-2*ROBOT_RADIUS,YMAX/2,0),getRobotPose(),false);
		}
		else point1 = true;
	}
	else if(!point2){
		if(!facingHeading(-PI/2,getRobotPose())){
			faceAngle(-PI/2,getRobotPose());
		}
		else {
			point2 = true;
			prevPose = getRobotPose();
		}
		setLED(LEDColor::PURPLE);
	}
	else{
		setLED(LEDColor::OFF);
		if(counter<1000){
			setMotors(800,800);
		}
		else {
			setMotors(1600,1600);
			setLED(LEDColor::BLUE);
		}
		if(abs(getRobotPose().y - prevPose.y)<6){
			counter++;
			setLED(LEDColor::RED);
		}
		prevPose = getRobotPose();
	}
}

void defenseLogic2(){
	Location* allies = getAllyLocations();
	if(getPuckLocation().x>0){
		if(atLocation(Location(XMIN/2,-25),allies[0])||atLocation(Location(XMIN/2,-25),allies[1])){
			goToPosition(Pose(XMIN/2,25,0),getRobotPose(),false);
		}
		else goToPosition(Pose(XMIN/2,-25,0),getRobotPose(),false);
	}
	else goBehindPuck();
}