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
#include "wireless.h"
#include "miscellaneous.h"
#include "avr/io.h"
#include "stdlib.h"

extern "C"{
	#include "m_usb.h"
};


//TODO get rid of this:
int16_t lastDistance = 0;
int16_t lastTheta = 0;
Pose lastPose = getRobotPose();
uint16_t k1 = 2; //distance proportional
uint16_t k2 = 1; //angle proportional
uint16_t k3 = 0; //distance derivative
uint16_t k4 = 55; //angle derivative

void goToBackwards(Pose target, Pose current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	int16_t distance = (uint16_t)abs(deltaX*deltaX + deltaY*deltaY);
	if(distance>16){ //if not within 5 pixels in both x and y
		int16_t targetTheta = atan2b(-deltaY,-deltaX); //find angle towards target
		angle deltaTheta = (current.o - targetTheta)/8;
		int16_t temp1 = k1 * distance - k3 * (distance - lastDistance);
		int16_t temp2 = abs(k2*deltaTheta) - k4*abs((deltaTheta - lastTheta));
		uint16_t x = MIN(1400,MAX(0,temp1));
		uint16_t y = MIN(1200,MAX(0,temp2));
		
		if (deltaTheta < 1375 + PI && deltaTheta > -1375 + PI){ //if within 0.1 radians ~5* of target angle,
			setMotors(-x,-x); //forwards
		}
		else {
			if(abs(deltaTheta)>PI/16+PI){
				faceLocation(Location(target.x,target.y),current);
			}
			else{
				if(deltaTheta >0) {
					setMotors(-x+y,-x); //spin cw, forwards
				}
				else {
					setMotors(-x,-x+y); //spin ccw, forwards
				}
			}
		}
		lastDistance = distance;
		lastTheta = deltaTheta;
	}
	else {
		setMotors(0,0);
		lastDistance = 0;
		lastTheta = 0;
	}
}

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
int k = 0;
void goToPosition(Pose target, Pose current, bool toPuck){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	uint16_t distance = (uint16_t)abs(deltaX*deltaX + deltaY*deltaY);
	int16_t targetTheta = atan2b(-deltaY,-deltaX); //find angle towards target
	angle deltaTheta = (current.o - targetTheta)/8;
	int16_t temp1 = k1 * distance - k3 * (distance - lastDistance);
	int16_t temp2 = abs(k2*deltaTheta) - k4*abs((deltaTheta - lastTheta));
	uint16_t x = MIN(1600,MAX(0,temp1));
	uint16_t y = MIN(1400,MAX(0,temp2));
	uint8_t d1;
	if(toPuck) d1 = 49+ROBOT_RADIUS*ROBOT_RADIUS;
	else d1 = 16;
	if(distance>d1){ //if not within 5 pixels in both x and y
		/*
		uint16_t r = k1*distance;
		uint16_t q = k4*abs((deltaTheta - lastTheta));
		uint8_t packet[10]={0,0,x>>8,x&0xFF,y>>8,y&0xFF,r>>8,r&0xFF,q>>8,q&0xFF};
		sendPacket(Robot::CONTROLLER,0x21,packet);
		*/
		if(abs(deltaTheta)>PI/32){
			faceLocation(Location(target.x,target.y),current);
		}
		else{
			if (deltaTheta < 650 && deltaTheta > -650){ //if within 0.1 radians ~5* of target angle,

				setMotors(x,x); //forwards
			}
			else {
				if(deltaTheta >0) {
					setMotors(x-y,x); //spin cw, forwards
				}
				else {
					setMotors(x,x-y); //spin ccw, forwards
				}
			}
			lastDistance = distance;
			lastTheta = deltaTheta;
		}
		setLED(LEDColor::RED);
	}
	else {
		setLED(LEDColor::OFF);
		if(toPuck){
			
			if (!facingHeading(getPuckHeading()+current.o,getRobotPose())){
				faceAngle(getPuckHeading()+current.o,getRobotPose());
				setLED(LEDColor::BLUE);
			}
			else{
				setMotors(800,800);
				setLED(LEDColor::PURPLE);
			}
		}
		else{
			setMotors(0,0);
			lastDistance = 0;
			lastTheta = 0;
		}
	}
	
}

void goToPositionSpin(Pose target, Pose current){
	if(!facingLocation(target.getLocation(), current)){
		faceLocation(target.getLocation(),current);
	}
	else{
		if(!atLocation(Location(target.x,target.y),Location(current.x,current.y))){
			goToPosition(target,current,false);
		}
		else{//reset PID terms
			setMotors(0,0);
			lastDistance = 0;
			lastTheta = 0;
		}
		
	}
}

void goToPuck(Pose target, Pose current){
	if(!hasPuck()){
		if(target.x > XMIN/2+ROBOT_RADIUS){
			goToPosition(target,current,true);
		}
		else{
			if(target.x>current.x+2*ROBOT_RADIUS){
				goToPosition(target,current,true);
			}
			else{
				if (target.y>0){
					goToPosition(Pose(MAX(target.x-2*ROBOT_RADIUS,XMIN+2*ROBOT_RADIUS),target.y-2*ROBOT_RADIUS,target.o),current,true);
				}
				
				else{
					goToPosition(Pose(MAX(target.x-2*ROBOT_RADIUS,XMIN+2*ROBOT_RADIUS),target.y+2*ROBOT_RADIUS,target.o),current,true);
				}
			}
		}
	}
}

void goToPositionPuck(Pose target, Pose current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	int16_t distance = (uint16_t)abs(deltaX*deltaX + deltaY*deltaY);
	if(distance>16){ //if not within 5 pixels in both x and y
		int16_t targetTheta = atan2b(-deltaY,-deltaX); //find angle towards target
		angle deltaTheta = (current.o - targetTheta)/8;
		int16_t temp1 = k1 * distance - k3 * (distance - lastDistance);
		int16_t temp2 = abs(k2*deltaTheta) - k4*abs((deltaTheta - lastTheta));
		uint16_t x = MIN(1400,MAX(0,temp1));
		uint16_t y = MIN(600,MAX(0,temp2));
		
		/*
		uint16_t r = k1*distance;
		uint16_t q = k4*abs((deltaTheta - lastTheta));
		uint8_t packet[10]={0,0,x>>8,x&0xFF,y>>8,y&0xFF,r>>8,r&0xFF,q>>8,q&0xFF};
		sendPacket(Robot::CONTROLLER,0x21,packet);
		*/
		
		if (deltaTheta < 1375 && deltaTheta > -1375){ //if within 0.1 radians ~5* of target angle,

			setMotors(x,x); //forwards
		}
		else {
			if(deltaTheta >0) {
				setMotors(x-y,x); //spin cw, forwards
			}
			else {
				setMotors(x,x-y); //spin ccw, forwards
			}
		}
		lastDistance = distance;
		lastTheta = deltaTheta;
	}
	
	else{
		setMotors(0,0);
		lastDistance = 0;
		lastTheta = 0;
	}
}

bool facingLocation(Location target, Pose current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	angle o = atan2b(-deltaY,-deltaX);
	return abs(current.o-o) < 6500;
}

bool facingHeading(angle target, Pose current){
	return abs(current.o - target) < 6500;
}

void faceLocation(Location target, Pose current){
	if(!facingLocation(target,current)){
		int16_t deltaX = current.x - target.x;
		int16_t deltaY = current.y - target.y;
		angle o = atan2b(-deltaY,-deltaX);
		angle deltaTheta = (current.o - o)/8;
		//uint8_t buffer[10] = {0,0,(current.o-o)>>8,(current.o-o)&0xFF,(current.o-lastPose.o)>>8,(current.o-lastPose.o)&0xFF,0,0,0,0};
		//sendPacket(Robot::CONTROLLER,0x21,buffer);
		int16_t temp1=k2 * abs(deltaTheta) - k4 * abs(deltaTheta - lastTheta);
		uint16_t x = MAX(0,MIN(1200,temp1));
		/*uint16_t r = k2 * abs(deltaTheta);
		uint16_t q = k4 * abs(deltaTheta - lastTheta);
		uint16_t y = k2 * abs(deltaTheta) - k4 * abs(deltaTheta - lastTheta);
		uint8_t packet[10]={0,0,x>>8,x&0xFF,y>>8,y&0xFF,r>>8,r&0xFF,q>>8,q&0xFF};
		sendPacket(Robot::CONTROLLER,0x21,packet);*/
		if(current.o - o > 0){
			setMotors(-x,x);
		}
		else{
			setMotors(x,-x);
		}
		lastTheta = deltaTheta;
	}
	else {
		setMotors(0,0);
		lastTheta = 0;
	}
}

void faceAngle(angle o,Pose current){
	int16_t temp1 = k2 * abs((current.o - o)/8) - k4 * abs(current.o - lastPose.o);
	uint16_t x = MAX(0,MIN(1200,temp1));
	
	if(!facingHeading(o,current)){
		
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
	//Location* enemyLocations = getEnemyLocations();
	//Location obstacles[5] = {allyLocations[0],allyLocations[1],enemyLocations[0],enemyLocations[1],enemyLocations[2]};
	uint8_t checkRadius = radius + ROBOT_RADIUS;
	for(int x = 0; x < 5; x++){
		if(x == 2){
			checkRadius = checkRadius + ROBOT_RADIUS;
		}
		if(circleIntersectsSegment(p1, p2, allyLocations[x], checkRadius)){
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

bool atLocation(Location target, Location current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	int16_t distance = (uint16_t)abs(deltaX*deltaX + deltaY*deltaY);
	return distance < 16;
}