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
/*
void goToBackwards(Pose target, Pose current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	uint16_t distance = (uint16_t)abs(deltaX*deltaX + deltaY*deltaY);
	angle targetTheta = atan2b(-deltaY,-deltaX); //find angle towards target
	angle offsetTheta = (current.o - targetTheta)/8;
	uint16_t temp1 = k1 * distance - k3 * (distance - lastDistance);
	int16_t temp2 = abs(k2*offsetTheta) - k4*abs((offsetTheta - lastTheta));
	uint16_t x = MIN(900,MAX(0,temp1));
	uint16_t y = MIN(700,MAX(0,temp2));
	if(distance>16){ //if not within 5 pixels in both x and y

		if(abs(PI/8-offsetTheta)>PI/32){
				faceAngle(PI+offsetTheta*8,current);
		}
		else{
			if ((uint16_t) abs(PI/8-offsetTheta)<650){ //if within 0.1 radians ~5* of target angle,
				setMotors(-x,-x); //forwards
			}
			else {
				if(PI/8-offsetTheta <0) {
					setMotors(-x+y,-x); //spin cw, forwards
				}
				else {
					setMotors(-x,-x+y); //spin ccw, forwards
				}
			}
			lastDistance = distance;
			lastTheta = offsetTheta;
		}
	}
	else {
		setMotors(0,0);
		lastDistance = 0;
		lastTheta = 0;
	}
}*/

void goTo(Pose target, Pose current, bool backwards){
	if(backwards)
		current.o += PI;
	Pose relativeTarget = Pose(target.x - current.x,target.y - current.y,target.o - current.o);
	angle targetAngle = atan2b(relativeTarget.y, relativeTarget.x) - current.o;
	int16_t d = relativeTarget.x * relativeTarget.x + relativeTarget.y * relativeTarget.y;
	if(d<36){
		setMotors(0,0);
		return;
	}
	bool flipControls = targetAngle >= 0;
	targetAngle = abs(targetAngle);
	int16_t left = MIN(1599,d);
	int16_t right = left*cosb(targetAngle);
	if(backwards){
		if(flipControls)
			setMotors(-right,-left);
		else
			setMotors(-left,-right);
	}
	else{
		if(flipControls)
			setMotors(left,right);
		else
			setMotors(right,left);
	}
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
	updateDestination(target);
}

int k = 0;
void goToPosition(Pose target, Pose current, bool toPuck, bool backwards,uint16_t speed){
	if(backwards)
		current.o += PI;
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	uint16_t distance = (uint16_t)abs(deltaX*deltaX + deltaY*deltaY);
	angle targetTheta = atan2b(-deltaY,-deltaX); //find angle towards target
	angle offsetTheta = (current.o - targetTheta)/8;
	uint16_t temp1 = k1 * distance - k3 * (distance - lastDistance);
	int16_t temp2 = abs(k2*offsetTheta) - k4*abs((offsetTheta - lastTheta));
	uint16_t x = MIN(speed,MAX(0,temp1));
	uint16_t y = MIN(MAX(speed-200,0),MAX(0,temp2));
	uint16_t d1;
	if(toPuck) d1 = (ROBOT_RADIUS+PUCK_RADIUS+10)*(ROBOT_RADIUS+PUCK_RADIUS+10);
	else d1 = 16;
	if(distance>d1){ //if not within 5 pixels in both x and y
		/*
		uint16_t r = k1*distance;
		uint16_t q = k4*abs((offsetTheta - lastTheta));
		uint8_t packet[10]={0,0,x>>8,x&0xFF,y>>8,y&0xFF,r>>8,r&0xFF,q>>8,q&0xFF};
		sendPacket(Robot::CONTROLLER,0x21,packet);
		*/
		if(abs(offsetTheta)>PI/32){
			faceLocation(Location(target.x,target.y),current,targetTheta);
		}
		else{
			uint16_t d2;
			if (toPuck) d2 = 450;
			else d2 = 650;
			if(backwards){
				if ((uint16_t) abs(offsetTheta)<d2){ //if within 0.1 radians ~5* of target angle,
					setMotors(-x,-x); //forwards
				}
				else {
					if(offsetTheta >0) {
						setMotors(-x,y-x); //spin cw, forwards
					}
					else {
						setMotors(y-x,-x); //spin ccw, forwards
					}
				}
			}
			else{
				if ((uint16_t) abs(offsetTheta)<d2){ //if within 0.1 radians ~5* of target angle,
					setMotors(x,x); //forwards
				}
				else {
					if(offsetTheta >0) {
						setMotors(x-y,x); //spin cw, forwards
					}
					else {
						setMotors(x,x-y); //spin ccw, forwards
					}
				}
			}
			lastDistance = distance;
			lastTheta = offsetTheta;
		}
	}
	else {
		if(toPuck){
			
			if (!facingLocation(getPuckLocation(),getRobotPose(),targetTheta)){
				faceLocation(getPuckLocation(),getRobotPose(),targetTheta);
			}
			else{
				setMotors(1200,1200);
			}
		}
		else{
			setMotors(0,0);
			lastDistance = 0;
			lastTheta = 0;
		}
	}
	updateDestination(target);
}

void goToPositionSpin(Pose target, Pose current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	angle o = atan2b(-deltaY,-deltaX);
	if(!facingLocation(target.getLocation(), current,o)){
		faceLocation(target.getLocation(),current,o);
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
	updateDestination(target);
}
/*
void goToPuck(Pose target, Pose current){
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
*/
void goToPositionPuck(Pose target, Pose current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	uint16_t distance = (uint16_t)abs(deltaX*deltaX + deltaY*deltaY);
	if(distance>16){ //if not within 5 pixels in both x and y
		angle targetTheta = atan2b(-deltaY,-deltaX); //find angle towards target
		angle offsetTheta = (current.o - targetTheta)/8;
		uint16_t temp1 = k1 * distance - k3 * (distance - lastDistance);
		int16_t temp2 = abs(k2*offsetTheta) - k4*abs((offsetTheta - lastTheta));
		uint16_t x;
		if(getRobotPose().x<XMAX-5*ROBOT_RADIUS){
		 x= MIN(1200,MAX(0,temp1));
		}
		else{
			x=MIN(1400,MAX(0,temp1));
		}
		uint16_t y = MIN(400,MAX(0,temp2));
		
		/*
		uint16_t r = k1*distance;
		uint16_t q = k4*abs((offsetTheta - lastTheta));
		uint8_t packet[10]={0,0,x>>8,x&0xFF,y>>8,y&0xFF,r>>8,r&0xFF,q>>8,q&0xFF};
		sendPacket(Robot::CONTROLLER,0x21,packet);
		*/
		if (offsetTheta < 650 && offsetTheta > -650){ //if within 0.1 radians ~5* of target angle,

			setMotors(x,x); //forwards
		}
		else {
			if(offsetTheta >0) {
				setMotors(x-y,x); //spin cw, forwards
			}
			else {
				setMotors(x,x-y); //spin ccw, forwards
			}
		}
		lastDistance = distance;
		lastTheta = offsetTheta;
	}
	
	else{
		setMotors(0,0);
		lastDistance = 0;
		lastTheta = 0;
	}
	updateDestination(target);
}
bool facingLocation(Location target, Pose current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	angle o = atan2b(-deltaY,-deltaX);
	return facingLocation(target, current, o);
}
bool facingLocation(Location target, Pose current,angle o){
	return abs(current.o-o) < 4500;
}

bool facingHeading(angle target, Pose current){
	return abs(current.o - target) < 4500;
}

void faceLocation(Location target, Pose current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	angle o = atan2b(-deltaY,-deltaX);
	faceLocation(target,current,o);
}
void faceLocation(Location target, Pose current,angle o){
	if(!facingLocation(target,current,o)){
		angle offsetTheta = (current.o - o)/8;
		//uint8_t buffer[10] = {0,0,(current.o-o)>>8,(current.o-o)&0xFF,(current.o-lastPose.o)>>8,(current.o-lastPose.o)&0xFF,0,0,0,0};
		//sendPacket(Robot::CONTROLLER,0x21,buffer);
		int16_t temp1=k2 * abs(offsetTheta) - k4 * abs(offsetTheta - lastTheta)+200;
		uint16_t x = MAX(0,MIN(800,temp1));
		/*uint16_t r = k2 * abs(offsetTheta);
		uint16_t q = k4 * abs(offsetTheta - lastTheta);
		uint16_t y = k2 * abs(offsetTheta) - k4 * abs(offsetTheta - lastTheta);
		uint8_t packet[10]={0,0,x>>8,x&0xFF,y>>8,y&0xFF,r>>8,r&0xFF,q>>8,q&0xFF};
		sendPacket(Robot::CONTROLLER,0x21,packet);*/
		if(current.o - o > 0){
			setMotors(-x,x);
		}
		else{
			setMotors(x,-x);
		}
		lastTheta = offsetTheta;
	}
	else {
		setMotors(0,0);
		lastTheta = 0;
	}
}

void faceAngle(angle o,Pose current){
	int16_t temp1 = k2 * abs((current.o - o)/2) - k4 * abs(current.o - lastPose.o);
	uint16_t x = MAX(0,MIN(800,temp1));
	
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
	uint16_t distance = (uint16_t)abs(deltaX*deltaX + deltaY*deltaY);
	return distance < 144;
}
bool atLocationWide(Location target, Location current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	uint16_t distance = (uint16_t)abs(deltaX*deltaX + deltaY*deltaY);
	return distance < 500;
}

void goToPosition2(Pose target, Pose current, bool toPuck, bool backwards,uint16_t speed){
	if(backwards)
		current.o += PI;
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	uint16_t distance = (uint16_t)abs(deltaX*deltaX + deltaY*deltaY);
	angle targetTheta = atan2b(-deltaY,-deltaX); //find angle towards target
	angle offsetTheta = (current.o - targetTheta)/8;
	uint16_t temp1 = k1 * distance - k3 * (distance - lastDistance);
	int16_t temp2 = abs(k2*offsetTheta) - k4*abs((offsetTheta - lastTheta));
	uint16_t x = MIN(speed,MAX(0,temp1));
	uint16_t y = MIN(MAX(speed-200,0),MAX(0,temp2));
	uint16_t d1;
	if(toPuck) d1 = (ROBOT_RADIUS+PUCK_RADIUS+10)*(ROBOT_RADIUS+PUCK_RADIUS+10);
	else d1 = 16;
	if(distance>d1){ //if not within 5 pixels in both x and y
		/*
		uint16_t r = k1*distance;
		uint16_t q = k4*abs((offsetTheta - lastTheta));
		uint8_t packet[10]={0,0,x>>8,x&0xFF,y>>8,y&0xFF,r>>8,r&0xFF,q>>8,q&0xFF};
		sendPacket(Robot::CONTROLLER,0x21,packet);
		*/
		
			uint16_t d2;
			if (toPuck) d2 = 450;
			else d2 = 650;
			if(backwards){
				if ((uint16_t) abs(offsetTheta)<d2){ //if within 0.1 radians ~5* of target angle,
					setMotors(-x,-x); //forwards
				}
				else {
					if(offsetTheta >0) {
						setMotors(-x,y-x); //spin cw, forwards
					}
					else {
						setMotors(y-x,-x); //spin ccw, forwards
					}
				}
			}
			else{
				if ((uint16_t) abs(offsetTheta)<d2){ //if within 0.1 radians ~5* of target angle,
					setMotors(x,x); //forwards
				}
				else {
					if(offsetTheta >0) {
						setMotors(x-y,x); //spin cw, forwards
					}
					else {
						setMotors(x,x-y); //spin ccw, forwards
					}
				}
			}
			lastDistance = distance;
			lastTheta = offsetTheta;

	}
	else {
		if(toPuck){
			
			if (!facingLocation(getPuckLocation(),getRobotPose(),targetTheta)){
				faceLocation(getPuckLocation(),getRobotPose(),targetTheta);
			}
			else{
				setMotors(1200,1200);
			}
		}
		else{
			setMotors(0,0);
			lastDistance = 0;
			lastTheta = 0;
		}
	}
	updateDestination(target);
}

