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
int16_t deltaDistance = 0;
int16_t lastTheta = 0;
angle integralTheta = 0;
Pose lastPose = getRobotPose();
uint16_t time2=0;

void goToPosition(Pose target, Pose current, bool faceForward);
void goToPositionSpin(Pose target, Pose current);
void goTo(Pose target, Pose current){
	Pose relativeTarget = target - current;
	angle targetAngle = atan2b(relativeTarget.y, relativeTarget.x) - current.o;
	int16_t d = relativeTarget.x * relativeTarget.x + relativeTarget.y * relativeTarget.y;
	//if(d<36){
		//setMotors(0,0);
	//	return;
	//}
	bool flipControls = targetAngle >= 0;
	targetAngle = abs(targetAngle);
	int16_t left = MIN(800,d);
	int16_t right = left*cosb(targetAngle);
	//if(flipControls)
		//setMotors(left,right);
	//else
		//setMotors(right,left);
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
}
///Switch to using the Pose class (see Localization.h)
void goToPosition(Pose target, Pose current, bool faceForward){
	if((current.x > target.x + 5 || current.x < target.x - 5) && (current.y > target.y + 5 || current.y < target.y - 5)){ //if not within 5 pixels in both x and y
		int16_t deltaX = current.x - target.x;
		int16_t deltaY = current.y - target.y;
		int16_t distance = sqrt(deltaX*deltaX + deltaY*deltaY);
		int16_t targetTheta = atan2b(deltaY,deltaX); //find angle towards target
		angle deltaTheta = current.o - target.o;

		uint16_t k1 = 10; //distance proportional
		uint16_t k2 = 2; //angle proportional
		uint16_t k3 = 1; //distance derivative
		uint16_t k4 = 1; //angle derivative
		uint16_t k5 = 1; //distance integral
		uint16_t k6 = 1; //angle integral
		
		integralTheta += deltaTheta;
		deltaDistance += distance;

		uint16_t x = k1 * distance + k2 * deltaTheta + k3 * (distance - lastDistance) + k4 * deltaDistance;
		uint16_t y = k2*deltaTheta + k5*(targetTheta - lastTheta) + k6*integralTheta;
		if (x > 100) x = 100;
		if (faceForward){
			if (deltaTheta < 1 && deltaTheta > -1){ //if within 1 of target
				setMotors(x,x); //forwards
			}
			else if (deltaTheta < PI + 1 && deltaTheta > PI - 1) {
				setMotors(-x,-x); //backwards
			}
			else {
				if (nearWall(current)) {
					setMotors(y,y);
				}
				else{
					if(deltaTheta < PI/2 && deltaTheta > 0) {
						setMotors(x,x - y); //spin cw, forwards
					}
					else if (deltaTheta < 0 && deltaTheta > -PI/2){
						setMotors(x+y,x); //spin cw, forwards
					}
					else if (deltaTheta < PI && deltaTheta > PI/2){
						setMotors(-x,-x + y); //spin ccw, backwards
					}
					else {
						setMotors(-x - y,-x); //spin ccw, backwards
					}
				}
			}
		}
		else {
			if (deltaTheta < 1 && deltaTheta > -1){ //if within 0.1 radians ~5* of target angle,
				setMotors(x,x); //forwards
			}
			else {
				if (nearWall(current)) {
					setMotors(y,-y);
				}
				else
				{
					if(deltaTheta < PI && deltaTheta > 0) {
						setMotors(x,x - y); //spin cw, forwards
					}
					else {
						setMotors(x + y,x); //spin ccw, backwards
					}
				}
			}
		}
	}
	else {
		lastDistance = 0;
		deltaDistance = 0;
		lastTheta = 0;
		integralTheta = 0;
	}
}

void goToPositionSpin(Pose target, Pose current){
	if(!facingPose(target, current)){
		m_green(0);
		facePose(target,current);
	}
	else{
		m_green(1);
		//setMotors(800,800);
	/*
		if((current.x > target.x + 5 || current.x < target.x - 5) && (current.y > target.y + 5 || current.y < target.y - 5)){
			int16_t deltaX = current.x - target.x;
			int16_t deltaY = current.y - target.y;
			int16_t distance = sqrt(deltaX*deltaX + deltaY*deltaY);
			int16_t x = MIN(800,3 * distance - 1 * distance - lastDistance);
			setMotors(x,x);
		}
		else{//reset PID terms
			lastDistance = 0;
			deltaDistance = 0;
			lastTheta = 0;
			integralTheta = 0;
		}
		*/
	}
}

void goToPositionPuck(Pose target, Pose current){
	//implement motion with puck
}

bool facingPose(Pose target, Pose current){
	int16_t deltaX = current.x - target.x;
	int16_t deltaY = current.y - target.y;
	angle o = atan2b(-deltaY,-deltaX);
	return current.o < o + 250 && current.o > o - 250;
}

void facePose(Pose target, Pose current){
	uint16_t time1 = getTime();
	if(!facingPose(target,current)){
		int16_t deltaX = current.x - target.x;
		int16_t deltaY = current.y - target.y;
		angle o = atan2b(-deltaY,-deltaX);
		uint16_t deltaTime = time1-time2;
		uint16_t x = MIN(800,0.5 * abs((current.o - o)) - 0.5 * abs((current.o - lastPose.o)/deltaTime));
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