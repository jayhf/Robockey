#pragma once

/*
* PathPlanning.h
*
* Created: 11/11/2015 7:32:28 PM
*  Author: Daniel Orol
*/

///Please switch all types to those found in stdint.h

#include "Digital.h"
#include "Localization.h"

void goToPosition(Pose target, Pose current, bool faceForward);

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

		uint16_t x = k1 * distance + k2 * deltaTheta;
		if (x > 100) x = 100;
		if (faceForward){
			if (deltaTheta < 1 && deltaTheta > -1){ //if within 1 of target
				movement(k1*distance,k1*distance); //forwards
			}
			else if (deltaTheta < PI + 1 && deltaTheta > PI - 1) {
				movement(-k1*distance,-k1*distance); //backwards
			}
			else {
				if (nearWall(current)) {
					movement(k2*deltaTheta,-k2*deltaTheta);
				}
				else{
					if(deltaTheta < PI/2 && deltaTheta > 0) {
						movement(x,x - k2*deltaTheta); //spin cw, forwards
					}
					else if (deltaTheta < 0 && deltaTheta > -PI/2){
						movement(x+k2*deltaTheta,x); //spin cw, forwards
					}
					else if (deltaTheta < PI && deltaTheta > PI/2){
						movement(-x,-x + k2*deltaTheta); //spin ccw, backwards
					}
					else {
						movement(-x - k2*deltaTheta,-x); //spin ccw, backwards
					}
				}
			}
		}
		else {
			if (deltaTheta < 1 && deltaTheta > -1){ //if within 0.1 radians ~5* of target angle,
				movement(k1*distance,k1*distance); //forwards
			}
			else {
				if (nearWall(current)) {
					movement(k2*deltaTheta,-k2*deltaTheta);
				}
				else
				{
					if(deltaTheta < PI && deltaTheta > 0) {
						movement(x,x - k2*deltaTheta); //spin cw, forwards
					}
					else {
						movement(x + k2*deltaTheta,x); //spin ccw, backwards
					}
				}
			}
		}
	}
}

void goToPositionPuck(Pose target, Pose current){
	//implement motion with puck
}

void facePose(Pose pose){
	Pose robotPose = getRobotPose();
	if (robotPose.o > pose.o + 1){
		//spin cw
		movement(-100,100);
	}
	else if (robotPose.o < pose.o - 1){
		//spin ccw
		movement(100,-100);
	}
}