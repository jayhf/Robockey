/*
* Functions.h
*
* Created: 11/11/2015 7:32:28 PM
*  Author: Daniel Orol
*/

///Please switch all types to those found in stdint.h

#include "math.h"
#include "avr/builtins.h"
#include "string.h"
#include <stdlib.h>
#include "BAMSMath.h"
#include "ADC.h"
#include "Localization.h"


extern "C"{
	#include "m_general.h"
	#include "m_bus.h"
	#include "m_imu.h"
	#include "m_rf.h"
	#include "m_usb.h"
	#include "m_wii.h"
}

void goToPosition(Pose target, Pose current, bool faceForward);
void movement(uint16_t leftSpeed, uint16_t rightSpeed); //pass in as percentage of power
Pose findPuck(Pose current);
bool nearWall(Pose current);

int main(void)
{
	while(1)
	{
		//TODO:: Please write your application code
	}
}


void movement(uint16_t leftSpeed, uint16_t rightSpeed){
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
///Switch to using the Pose class (see Localization.h)
void goToPosition(Pose target, Pose current, bool faceForward){
	if((currentX > targetX + 5 || currentX < targetX - 5) && (currentY > targetY + 5 || currentY < targetY - 5)){ //if not within 5 pixels in both x and y
		uint16_t deltaX = current.x - target.x;
		uint16_t deltaY = current.y - target.y;
		uint16_t distance = sqrt(deltaX*deltaX + deltaY*deltaY);
		uint16_t targetTheta = atan2b(deltaY,deltaX); //find angle towards target
		uint16_t deltaTheta = current.o - target.o;
		
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
				if (nearWall(current.x, current.y)) {
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
			if (deltaTheta < 1 && deltaTheta > -1){ //if within 0.1 radians ~5* of target angle,
				movement(k1*distance,k1*distance); //forwards
			}
			else {
				if (nearWall(current.x, current.y)) {
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

///You can't return arrays. (I'm 95% sure). Return a Pose instead. Also, move this to Localization.h at some point

Pose findPuck(Pose current){
	uint16_t val1 = 0;
	uint16_t val2 = 0;
	uint16_t val3 = 0;
	uint16_t photo1 = 0;
	uint16_t photo2 = 0;
	uint16_t photo3 = 0;

	uint16_t * values = getIRData(); // loop through transistors
	for (uint8_t i = 0; i < 16; i++){
		thisADC = values[i];
		uint8_tint pin = check(PINB,0) + 2 * check(PINB,1) + 4 * check(PINB,2) + 8 * check(PINB,3);
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

	// loop through transistors
	uint16_t pin = check(PINB,0) + 2 * check(PINB,1) + 4 * check(PINB,2) + 8 * check(PINB,3);
	
	///You don't handle the wrap around case where the brightest is the first or last value
	if (ADC > val3) {
		if (ADC > val2) {
			if (ADC > val1){
				photo3 = photo2;
				photo2 = photo1;
				photo1 = pin; //current pin
				val3 = val2;
				val2 = val1;
				val1 = ADC;

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
		///You never rotate by the offset by which phototransistor is selected.
		heading = 2*PI/16 * (photo1 * val1 + photo2 * val2) / (val1+val2); //compute weighted average and multiply by degrees per transistor
	}
	heading = current.o + heading;
	///Don't see the point of multiplying and dividing by 3. Doesn't really matter, because we need a lookup table based system
	///to get a decent distance measurement. You also will need to consider that the resistor changes and you need to check which is used.
	uint16_t distance = 3*(val1 + val2 + val3)/3; //need to scale accordingly
	return Pose(distance*cos(heading) + current.x,distance*sin(heading)+current.y,heading);
}

///We definitely have enough information to implement this. Use the ymax etc. constants 
bool nearWall(Pose current){
	return current.x > XMAX - 10 || current.x < XMIN + 10 || current.y > YMAX - 10 || current.y < YMIN +10;
}