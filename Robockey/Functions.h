/*
* Functions.h
*
* Created: 11/11/2015 7:32:28 PM
*  Author: Daniel Orol
*/


#define F_CPU 16000000

#include "math.h"
#include "avr/builtins.h"
#include "string.h"
#include <stdlib.h>
#include "ADC.h"

#define PI 3.14

extern "C"{
	#include "m_general.h"
	#include "m_bus.h"
	#include "m_imu.h"
	#include "m_rf.h"
	#include "m_usb.h"
	#include "m_wii.h"
}

void goToPosition(int targetX, int targetY, int currentX, int currentY, float currentTheta, bool faceForward);
void movement(int leftSpeed, int rightSpeed); //pass in as percentage of power
int[] findPuck();
bool nearWall(int currentX, int currentY);

int main(void)
{
	while(1)
	{
		//TODO:: Please write your application code
	}
}

void movement(int leftSpeed, int rightSpeed){
	if (leftSpeed > 100) {
		leftSpeed = 100;
	}
	if (rightSpeed > 100) {
		rightSpeed = 100;
	}
	if (leftSpeed < -100) {
		leftSpeed = -100;
	}
	if (rightSpeed < -100) {
		rightSpeed = -100;
	}
	if (leftSpeed > 0) {
		set(PINC,6);
		OCR1B = leftSpeed * OCR1A * 0.01;
	}
	else if (leftSpeed < 0) {
		clear(PINC,6);
		OCR1B = leftSpeed * OCR1A * 0.01;
	}
	if (rightSpeed > 0) {
		set(PINC,7);
		OCR1C = rightSpeed *0.01* OCR1A;
	}
	else if (rightSpeed < 0) {
		clear(PINC,7);
		OCR1C = rightSpeed *0.01* OCR1A;
	}
}

void goToPosition(int targetX, int targetY, int currentX, int currentY, float currentTheta, bool faceForward){
	if((currentX > targetX + 5 || currentX < targetX - 5) && (currentY > targetY + 5 || currentY < targetY - 5)){ //if not within 5 pixels in both x and y
		int deltaX = currentX - targetX;
		int deltaY = currentY - targetY;
		int distance = sqrt(deltaX*deltaX + deltaY*deltaY);
		float targetTheta = atan2(deltaY,deltaX); //find angle towards target
		int deltaTheta = currentTheta - targetTheta;
		
		int k1 = 0.1; //distance proportional
		int k2 = 2; //angle proportional
		
		int x = 0.1 * distance + 2 * deltaTheta;
		if (x > 100) x = 100;
		if (faceForward){
			if (deltaTheta < 0.1 && deltaTheta > -0.1){ //if within 0.1 radians ~5* of target angle,
				movement(k1*distance,k1*distance); //forwards
			}
			else if (deltaTheta < PI + 0.1 && deltaTheta > PI - 0.1) {
				movement(-k1*distance,-k1*distance); //backwards
			}
			else {
				if (nearWall(currentX, currentY)) {
					movement(k2*deltaTheta,-k2*deltaTheta);
				}
				else
				{
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
			if (deltaTheta < 0.1 && deltaTheta > -0.1){ //if within 0.1 radians ~5* of target angle,
				movement(k1*distance,k1*distance); //forwards
			}
			else {
				if (nearWall(currentX, currentY)) {
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

int[] findPuck(){
	int val1 = 0;
	int val2 = 0;
	int val3 = 0;
	int photo1 = 0;
	int photo2 = 0;
	int photo3 = 0;
	uint16_t * values = getIRValues(); // loop through transistors
	for (int i = 0; i < 16; i++){
		thisADC = values[i];
		int pin = check(PINB,0) + 2 * check(PINB,1) + 4 * check(PINB,2) + 8 * check(PINB,3);
		if (thisADC > val3) {
			if (thisADC > val2) {
				if (thisADC > val 1){
					photo3 = photo2;
					photo2 = photo1;
					photo1 = pin; //current pin
					val3 = val2;
					val2 = val1;
					val1 = thisADC;
				}
				else {
					photo3 = photo2;
					photo2 = pin;
					val3 = val2;
					val2 = thisADC;
				}
			}
			else{
				photo3 = pin; //current pin
				val3 = thisADC;
			}
		}
	}
	//end loop
	float heading;
	if (((photo2 == photo1 + 1 && photo 3 == photo1 - 1) || (photo2 == photo1 - 1 && photo 3 == photo1 + 1)) && (val2 < val3 + 5 && val2 > val3 - 5)){
		//if largest reading is in betweeen next two and the next two are within +/- 5, assume that middle is pointing directly at it
		heading = 2*PI/16 * photo1;
		
	}
	else {
		heading = 2*PI/16 * (photo1 * val1 + photo2 * val2) / (val1+val2); //compute weighted average and multiply by degrees per transistor
	}
	int distance = 3*(val1 + val2 + val3)/3; //need to scale accordingly
	return [distance*cos(heading),distance*sin(heading)];
}

bool nearWall(int currentX, int currentY){
	return false;
}