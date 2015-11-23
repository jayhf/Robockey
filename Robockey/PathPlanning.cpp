/*
* PathPlanning.h
*
* Created: 11/11/2015 7:32:28 PM
*  Author: Daniel Orol
*/

///Please switch all types to those found in stdint.h

#include "PathPlanning.h"
#include "Digital.h"
#include "BAMSMath.h"
#include "time.h"
extern "C"{
	#include "m_usb.h"
};


//TODO get rid of this:
#include "GameState.h"
#include "wireless.h"
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
	if((current.x - target.x > 5 || current.x - target.x < - 5) && (current.y - target.y > 5 || current.y- target.y < - 5)){ //if not within 5 pixels in both x and y
		int16_t deltaX = current.x - target.x;
		int16_t deltaY = current.y - target.y;
		int16_t distance = sqrt(deltaX*deltaX + deltaY*deltaY);
		int16_t targetTheta = atan2b(deltaY,deltaX); //find angle towards target
		angle deltaTheta = current.o - targetTheta;

		uint16_t k1 = 10; //distance proportional
		uint16_t k2 = 1; //angle proportional
		uint16_t k3 = 1; //distance derivative
		uint16_t k4 = 20; //angle derivative
		

		uint16_t x = MIN(800,k1 * distance - k3 * (distance - lastDistance));
		uint16_t y = MIN(800,MAX(0,abs(k2*deltaTheta) - k4*abs((targetTheta - lastTheta))));
		if (!faceForward){
			if (deltaTheta < 3500 && deltaTheta > -3500){
				setMotors(x,x); //forwards
			}
			else if (deltaTheta < - 3500 && deltaTheta > - 3500) {
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
					if(deltaTheta < PI && deltaTheta > 0) {
						setMotors(x-y,x); //spin cw, forwards
					}
					else {
						setMotors(x,x+y); //spin ccw, forwards
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
	if(!facingPose(target, current)){
		facePose(target,current);
	}
	else{
		
		if((current.x - target.x > 5 || current.x - target.x < -5) && (current.y - target.y > 5 || current.y - target.y < 5)){
			int16_t deltaX = current.x - target.x;
			int16_t deltaY = current.y - target.y;
			int16_t distance = sqrt(deltaX*deltaX + deltaY*deltaY);
			int16_t x = MAX(0,MIN(800,10 * distance - 1 * (distance - lastDistance)));
			setMotors(x,x);
		}
		else{//reset PID terms
			setMotors(0,0);
			lastDistance = 0;
			lastTheta = 0;
		}
		
	}
}

void goToPositionPuck(Pose target, Pose current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	int16_t distance = sqrt(deltaX*deltaX + deltaY*deltaY);
	int16_t targetTheta = atan2b(deltaY,deltaX); //find angle towards target
	angle deltaTheta = current.o - targetTheta;

	uint16_t k1 = 10; //distance proportional
	uint16_t k2 = 1; //angle proportional
	uint16_t k3 = 1; //distance derivative
	uint16_t k4 = 20; //angle derivative
	

	uint16_t x = MIN(800,k1 * distance - k3 * (distance - lastDistance));
	uint16_t y = MIN(800,MAX(0,abs(k2*deltaTheta) - k4*abs((targetTheta - lastTheta))));
	
	if (distance < 40){
		setMotors(0,0);
	}
	else {
		if (deltaTheta >= 0){
			setMotors(y,y-200);
		}
		else {
			setMotors(y-200,y);
		}
	}
}

bool facingPose(Pose target, Pose current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	angle o = atan2b(-deltaY,-deltaX);
	return current.o < o + 3500 && current.o > o - 3500;
}

void facePose(Pose target, Pose current){
	uint16_t time1 = getTime();
	if(!facingPose(target,current)){
		int16_t deltaX = current.x - target.x;
		int16_t deltaY = current.y - target.y;
		angle o = atan2b(-deltaY,-deltaX);
		float deltaTime = 1/(time1-time2);
		//uint8_t buffer[10] = {0,0,(current.o-o)>>8,(current.o-o)&0xFF,(current.o-lastPose.o)>>8,(current.o-lastPose.o)&0xFF,0,0,0,0};
		//sendPacket(Robot::CONTROLLER,0x21,buffer);
		uint16_t x = MAX(0,MIN(800,1 * abs((current.o - o)) - 20 * abs((current.o - lastPose.o)*deltaTime)));
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