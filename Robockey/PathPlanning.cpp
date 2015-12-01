/*
* PathPlanning.h
*
* Created: 11/11/2015 7:32:28 PM
*  Author: Daniel Orol
*/

#include "PathPlanning.h"
#include "PlayerLogic.h"
#include "Digital.h"
#include "BAMSMath.h"
#include "time.h"
#include "FastMath.h"
#include "Localization.h"
#include <stdlib.h>

extern "C"{
	#include "m_usb.h"
};


//TODO get rid of this:
int16_t lastDistance = 0;
int16_t lastTheta = 0;
Pose lastPose = getRobotPose();
uint16_t time2=0;

void goToPosition(Pose target, Pose current, bool faceForward);
void goToPositionSpin(Pose target, Pose current);
void goTo(Pose target, Pose current){
	Pose relativeTarget = Pose(target.x - current.x,target.y - current.y,target.o - current.o);
	angle targetAngle = atan2b(relativeTarget.y, relativeTarget.x) - current.o;
	int16_t d = relativeTarget.x * relativeTarget.x + relativeTarget.y * relativeTarget.y;
	if(d<36){
		setMotors(0,0);
		return;
	}
	bool flipControls = targetAngle >= 0;
	targetAngle = abs(targetAngle);
	int16_t left = MIN(800,d);
	int16_t right = left*cosb(targetAngle);
	if(flipControls)
	setMotors(left,right);
	else
	setMotors(right,left);
	/*
	uint8_t packet[10];
	packet[2] = targetAngle >> 8;
	packet[3] = targetAngle &0xFF;
	packet[4] = d >> 8;
	packet[5] = d &0xFF;
	packet[6] = right>>3;//(flipControls?right:left) >> 3;
	packet[7] = left>>3;//(flipControls?left:right) >> 3;
	packet[8] = relativeTarget.x&0xFF;
	packet[9] = relativeTarget.y&0xFF;
	sendPacket(Robot::CONTROLLER, 0x20, packet);
	*/
}
///Switch to using the Pose class (see Localization.h)
void goToPosition(Pose target, Pose current, bool faceForward){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	int16_t distance = sqrt((uint16_t)abs(deltaX*deltaX + deltaY*deltaY));
	if(distance>5){ //if not within 5 pixels in both x and y
		int16_t targetTheta = atan2b(-deltaY,-deltaX); //find angle towards target
		angle deltaTheta = current.o - targetTheta;

		uint16_t k1 = 10; //distance proportional
		uint16_t k2 = 1; //angle proportional
		uint16_t k3 = 2; //distance derivative
		uint16_t k4 = 20; //angle derivative
		

		uint16_t x = MIN(900,k1 * distance - k3 * (distance - lastDistance));
		uint16_t y = MIN(300,MAX(0,abs(k2*deltaTheta) - k4*abs((targetTheta - lastTheta))));
		if (!faceForward){
			if (deltaTheta < 3500 && deltaTheta > -3500){
				setMotors(x,x); //forwards
			}
			else if (-deltaTheta < 3500 && -deltaTheta > 3500) {
				setMotors(-x,-x); //backwards
			}
			else {
				if (nearWall(current)) {
					goToPositionSpin(Pose(current.x, 0.8*current.y,current.o),current);
				}
				else{
					if(deltaTheta < PI/2 && deltaTheta > 0) {
						setMotors(x-y,x); //spin cw, forwards
					}
					else if (deltaTheta < 0 && deltaTheta > -PI/2){
						setMotors(x,x+y); //spin cw, forwards
					}
					else if (deltaTheta < PI && deltaTheta > PI/2){
						setMotors(-x+y,-x); //spin ccw, backwards
					}
					else {
						setMotors(-x,-x-y); //spin ccw, backwards
					}
				}
			}
		}
		else {
			if (deltaTheta < 3500 && deltaTheta > -3500){ //if within 0.1 radians ~5* of target angle,
				setMotors(x,x); //forwards
			}
			else {
				if (nearWall(current)) {
					goToPositionSpin(Pose(current.x, 0.8*current.y,current.o),current);
				}
				else
				{
					if(deltaTheta >0) {
						setMotors(x-y,x); //spin cw, forwards
					}
					else {
						setMotors(x,x-y); //spin ccw, forwards
					}
				}
			}
		}
	}
	else {
		setMotors(0,0);
		lastDistance = 0;
		lastTheta = 0;
	}
}

void goToPositionSpin(Pose target, Pose current){
	if(!facingLocation(target.getLocation(), current)){
		faceLocation(target.getLocation(),current);
	}
	else{
		int16_t deltaX = current.x - target.x;
		int16_t deltaY = current.y - target.y;
		if((deltaX > 5 || deltaX < -5) || (deltaY > 5 || deltaY < -5)){
			int16_t distance = sqrt((uint16_t)abs(deltaX*deltaX + deltaY*deltaY));
			int16_t x = MAX(0,MIN(900,7 * distance - 1 * (distance - lastDistance)));
			setMotors(x,x);
		}
		else{//reset PID terms
			setMotors(0,0);
			lastDistance = 0;
			lastTheta = 0;
		}
		
	}
}

bool goToPuck(Pose target, Pose current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	int16_t distance =  sqrt((uint16_t)abs(deltaX*deltaX + deltaY*deltaY));
	if(distance<5){
	//if (getStartPositive()) {
		if (target.x > current.x){
			if(target.y>0){
				goToPosition(Pose(target.x + ROBOT_RADIUS, target.y + ROBOT_RADIUS, target.o),current,true);
			}
			else {
				goToPosition(Pose(target.x + ROBOT_RADIUS, target.y - ROBOT_RADIUS, target.o),current,true);
			}
		}
		else {
			goToPosition(target, current,true);
		}
	/*}
	else {
		if (target.x < current.x){
			if(target.y>0){
				goToPosition(Pose(target.x - ROBOT_RADIUS, target.y + ROBOT_RADIUS, target.o),current,true);
			}
			else {
				goToPosition(Pose(target.x - ROBOT_RADIUS, target.y - ROBOT_RADIUS, target.o),current,true);
			}
		}
		else {
			goToPosition(target, current,true);
		}
	}*/
	return false;
	}
	else 
	{
		setMotors(0,0);
		return true;
		}
}

void goToPositionPuck(Pose target, Pose current){
	tryKick();
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	int16_t distance = sqrt((uint16_t)abs(deltaX*deltaX + deltaY*deltaY));
	if(distance>5){ //if not within 5 pixels in both x and y
		int16_t targetTheta = atan2b(-deltaY,-deltaX); //find angle towards target
		angle deltaTheta = current.o - targetTheta;

		uint16_t k1 = 40; //distance proportional
		uint16_t k2 = 0.5; //angle proportional
		uint16_t k3 = 25; //distance derivative
		uint16_t k4 = 15; //angle derivative
		

		uint16_t x = MIN(900,k1 * distance - k3 * (distance - lastDistance));
		uint16_t y = MIN(300,MAX(0,abs(k2*deltaTheta) - k4*abs((targetTheta - lastTheta))));
		
		if (deltaTheta < 3500 && deltaTheta > -3500){ //if within 0.1 radians ~5* of target angle,
			setMotors(x,x); //forwards
		}
		else {
			//if (nearWall(current)) {
				//goToPositionSpin(Pose(current.x, 0.8*current.y,current.o),current);
			//}
			//else
			//{
				if(deltaTheta >0) {
					setMotors(x-y,x); //spin cw, forwards
				}
				else {
					setMotors(x,x-y); //spin ccw, forwards
				}
			//}
		}
	}
	else {
		setMotors(0,0);
		lastDistance = 0;
		lastTheta = 0;
	}
}

bool facingLocation(Location target, Pose current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	angle o = atan2b(-deltaY,-deltaX);
	return current.o < o + 3500 && current.o > o - 3500;
}

void faceLocation(Location target, Pose current){
	uint16_t time1 = getTime();
	if(!facingLocation(target,current)){
		int16_t deltaX = current.x - target.x;
		int16_t deltaY = current.y - target.y;
		angle o = atan2b(-deltaY,-deltaX);
		float deltaTime = 1/(time1-time2);
		//uint8_t buffer[10] = {0,0,(current.o-o)>>8,(current.o-o)&0xFF,(current.o-lastPose.o)>>8,(current.o-lastPose.o)&0xFF,0,0,0,0};
		//sendPacket(Robot::CONTROLLER,0x21,buffer);
		uint16_t x = MAX(0,MIN(1199,1 * abs((current.o - o)) - 20 * abs((current.o - lastPose.o)*deltaTime)));
		if(current.o - o > 0){
			setMotors(-x,x);
		}
		else if(current.o - o < 0){
			setMotors(x,-x);
		}
	}
	lastPose = current;
	time2=time1;
}

void faceAngle(angle o,Pose current){
	uint16_t time1 = getTime();
	float deltaTime = 1/(time1-time2);
	uint16_t x = MAX(0,MIN(1199,1 * abs((current.o - o)) - 20 * abs((current.o - lastPose.o)*deltaTime)));
	
	if(current.o - o < -3500 || current.o - o > 3500){
		
		if(current.o - o > 0){
			setMotors(-x,x);
		}
		else if(current.o - o < 0){
			setMotors(x,-x);
		}
	}
	else{
	
		setMotors(0,0);
	}
	lastPose = current;
	time2=time1;
}

bool circleIntersectsSegment(Location p1, Location p2, Location c, uint8_t radius){
	if(c == UNKNOWN_LOCATION){
		return false;
	}
	p1.x-=c.x;
	p1.y-=c.y;
	p2.x-=c.x;
	p2.y-=c.y;
	if(radius>MIN(p1.x,p2.x)&&-radius<MAX(p1.x,p2.x)&&radius>MIN(p1.y,p2.y)&&-radius<MAX(p1.y,p2.y)){
		uint8_t dx = abs((int16_t)p1.x-p2.x);
		uint8_t dy = abs((int16_t)p1.y-p2.y);
		int16_t n = (int16_t)p2.x*p1.y-(int16_t)p2.y*p1.x;
		uint8_t d = sqrt((uint16_t)dx*dx+(uint16_t)dy*dy);
		return (int16_t)((uint16_t)radius*d) > n;
	}
	else
		return false;
}

bool checkIntersection(Location p1, Location p2, uint8_t radius){
	Location* allyLocations = getAllyLocations();
	Location* enemyLocations = getEnemyLocations();
	Location obstacles[5] = {allyLocations[0],allyLocations[1],enemyLocations[0],enemyLocations[1],enemyLocations[2]};
		uint8_t checkRadius = radius + ROBOT_RADIUS;
	for(int x = 0; x < 5; x++){
		if(x == 2){
			checkRadius = checkRadius + ROBOT_RADIUS;
		}
		if(circleIntersectsSegment(p1, p2, obstacles[x], checkRadius)){
			return true;
		}
	}
	return false;
}

uint8_t findPath(uint8_t *result, Location *vertices, uint8_t vertexCount, Location *enemies, Pose *allies){
	Location allyLocations[2] = {allies[0].getLocation(),allies[1].getLocation()};
	uint16_t scores[vertexCount];
	uint8_t distances[vertexCount][vertexCount];
	uint8_t previousLocations[vertexCount];
	bool checked[vertexCount];
	for(uint8_t i = 0; i<vertexCount;i++){
		checked[i] = 0;
		scores[i] = 0xffff;
		Location v1 = vertices[i];
		for(uint8_t j=i+1; j<vertexCount; j++){
			Location v2 = vertices[j];	
			uint8_t dx = abs((int16_t)v1.x-v2.x);
			uint8_t dy = abs((int16_t)v1.y-v2.y);
			distances[i][j] = distances[j][i] = sqrt((uint16_t)dx*dx+(uint16_t)dy*dy);
		}
	}
	scores[0] = 0;
	bool foundGoal = false;
	while(!foundGoal){
		uint16_t minScore = 0xFFFF;
		uint8_t minScoreIndex = 0xFF;
		for(uint8_t i = 0; i<vertexCount;i++){
			if(!checked[i]){
				uint16_t score = scores[i];
				if(score<minScore){
					score += distances[1][i];
					if(score<minScore){
						minScore = score;
						minScoreIndex = i;
					}
				}
			}
		}
		if(minScoreIndex == -1){
			return 0xFF;
		}
		checked[minScoreIndex] = true;
		for(uint8_t i = 0; i<vertexCount; i++){
			if(checked[i] || i==minScoreIndex)
				continue;
			uint16_t score = minScore + distances[minScoreIndex][i];
			if(score < scores[i]){
				bool connected = true;
				for(int j=0;j<3;j++){
					if(circleIntersectsSegment(vertices[minScoreIndex],vertices[i],enemies[j],30))
						connected = false;
				}
				if(circleIntersectsSegment(vertices[minScoreIndex],vertices[i],allyLocations[0],17))
					connected = false;
				if(circleIntersectsSegment(vertices[minScoreIndex],vertices[i],allyLocations[1],17))
					connected = false;
				if(connected){
					scores[i] = score;
					previousLocations[i] = minScoreIndex;
					if(i == 1){
						foundGoal = true;
						break;
					}
				}
			}
		}
	}
	uint8_t resultLength = 1;
	result[0] = 1;
	while(result[resultLength] != 0){
		result[resultLength] = previousLocations[result[resultLength-1]];
		resultLength++;
	}
	return resultLength;
}