#pragma once

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

uint16_t lastDistance = 0;
uint16_t deltaDistance = 0;
uint16_t lastTheta = 0;
angle integralTheta = 0;
Pose lastPose = getRobotPose();

void goToPosition(Pose target, Pose current, bool faceForward);
void goToPositionSpin(Pose target, Pose current);

///Switch to using the Pose class (see Localization.h)
void goToPosition(Pose target, Pose current, bool faceForward){
	if((current.x > target.x + 5 || current.x < target.x - 5) && (current.y > target.y + 5 || current.y < target.y - 5)){ //if not within 5 pixels in both x and y
		uint16_t deltaX = current.x - target.x;
		uint16_t deltaY = current.y - target.y;
		uint16_t distance = sqrt(deltaX*deltaX + deltaY*deltaY);
		uint16_t targetTheta = atan2b(deltaY,deltaX); //find angle towards target
		angle deltaTheta = current.o - target.o;

		uint16_t k1 = 10; //distance proportional
		uint16_t k2 = 2; //angle proportional
		uint16_t k3 = 1; //distance derivative
		uint16_t k4 = 1; //angle derivative
		uint16_t k5 = 1; //distance integral
		uint16_t k6 = 1; //angle integral
		
		integralTheta += deltaTheta-lastTheta;
		deltaDistance += distance-lastDistance;

		uint16_t x = k1 * distance + k2 * deltaTheta + k3 * (distance - lastDistance) + k4 * deltaDistance;
		uint16_t y = k2*deltaTheta + k5*(targetTheta - lastTheta) + k6*integralTheta;
		if (x > 100) x = 100;
		if (faceForward){
			if (deltaTheta < 1 && deltaTheta > -1){ //if within 1 of target
				movement(x,x); //forwards
			}
			else if (deltaTheta < PI + 1 && deltaTheta > PI - 1) {
				movement(-x,-x); //backwards
			}
			else {
				if (nearWall(current)) {
					movement(y,y);
				}
				else{
					if(deltaTheta < PI/2 && deltaTheta > 0) {
						movement(x,x - y); //spin cw, forwards
					}
					else if (deltaTheta < 0 && deltaTheta > -PI/2){
						movement(x+y,x); //spin cw, forwards
					}
					else if (deltaTheta < PI && deltaTheta > PI/2){
						movement(-x,-x + y); //spin ccw, backwards
					}
					else {
						movement(-x - y,-x); //spin ccw, backwards
					}
				}
			}
		}
		else {
			if (deltaTheta < 1 && deltaTheta > -1){ //if within 0.1 radians ~5* of target angle,
				movement(x,x); //forwards
			}
			else {
				if (nearWall(current)) {
					movement(y,-y);
				}
				else
				{
					if(deltaTheta < PI && deltaTheta > 0) {
						movement(x,x - y); //spin cw, forwards
					}
					else {
						movement(x + y,x); //spin ccw, backwards
					}
				}
			}
		}
	}
	else {
		uint16_t lastDistance = 0;
		uint16_t deltaDistance = 0;
		uint16_t lastTheta = 0;
		angle integralTheta = 0;
	}
}

void goToPositionSpin(Pose target, Pose current){
	if(!facingPose(target, current)){
		facePose(target,current);
	}
	else{
		if((current.x > target.x + 5 || current.x < target.x - 5) && (current.y > target.y + 5 || current.y < target.y - 5)){
			uint16_t deltaX = current.x - target.x;
			uint16_t deltaY = current.y - target.y;
			uint16_t distance = sqrt(deltaX*deltaX + deltaY*deltaY);
			uint16_t x = 5 * distance + 1 * distance - lastDistance;
		}
		else{//reset PID terms
			uint16_t lastDistance = 0;
			uint16_t deltaDistance = 0;
			uint16_t lastTheta = 0;
			angle integralTheta = 0;
		}
	}
}

void goToPositionPuck(Pose target, Pose current){
	//implement motion with puck
}

bool facingPose(Pose target, Pose current){
	uint16_t deltaX = current.x - target.x;
	uint16_t deltaY = current.y - target.y;
	angle o = atan2b(deltaY,deltaX);
	return current.o < o + 1 && current.o > o - 1;
}

void facePose(Pose target, Pose current){
	if(!facingPose(target,current)){
		uint16_t deltaX = current.x - target.x;
		uint16_t deltaY = current.y - target.y;
		angle o = atan2b(deltaY,deltaX);
		uint16_t x = 4 * (current.o - o) + 2 * (current.o - lastPose.o);
		if(current.o > o){
			movement(x,-x);
		}
		else if(current.o < o){
			movement(-x,x);
		}
	}
	lastPose = current;
}