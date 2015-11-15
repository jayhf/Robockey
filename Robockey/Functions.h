/*
* Functions.h
*
* Created: 11/11/2015 7:32:28 PM
*  Author: Daniel Orol
*/

///Please switch all types to those found in stdint.h
///F_CPU is handled in the main class, so this is unnecessary
#define F_CPU 16000000

#include "math.h"
#include "avr/builtins.h"
#include "string.h"
#include <stdlib.h>
///Update to using BAMSMath.h. It should be really easy to switch over. Just use toFloatAngle and toBAMS as needed.
///PI is defined already in BAMSMath.h and use the type angle ex.  angle o = PI;
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

///You don't properly handle negative speeds. Floating point math should be avoided.
///At the very least never divide by a constant, multiply by 1/constant.
void movement(int leftSpeed, int rightSpeed){
	if (leftSpeed > 100) {
		leftSpeed = 100;
	}
	if (rightSpeed > 100) {
		rightSpeed = 100;
	}
	if (leftSpeed > 0) {
		set(PINC,6);
		OCR1B = leftSpeed / 100.0 * OCR1A;
	}
	else if (leftSpeed < 0) {
		clear(PINC,6);
		OCR1B = leftSpeed / 100.0 * OCR1A;
	}
	if (rightSpeed > 0) {
		set(PINC,7);
		OCR1C = rightSpeed / 100.0 * OCR1A;
	}
	else if (rightSpeed < 0) {
		clear(PINC,7);
		OCR1C = rightSpeed / 100.0 * OCR1A;
	}
}
///Switch to using the Pose class (see Localization.h)
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

///You can't return arrays. (I'm 95% sure). Return a Pose instead. Also, move this to Localization.h at some point
///You're assuming we can get readings from both adjacent phototransistors, which is probably a bad assumption, based on the datasheet.
int[] findPuck(){
	int val1 = 0;
	int val2 = 0;
	int val3 = 0;
	int photo1 = 0;
	int photo2 = 0;
	int photo3 = 0;
	// loop through transistors
	int pin = check(PINB,0) + 2 * check(PINB,1) + 4 * check(PINB,2) + 8 * check(PINB,3);
	///Keeping track of photo1, 2 and 3 is not necessary. You can just do pin+1 or irValue[pin+1]
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
			else {
				photo3 = photo2;
				photo2 = pin;
				val3 = val2;
				val2 = ADC;
			}
		}
		else{
			photo3 = pin; //current pin
			val3 = ADC;
		}
	}
	//end loop
	///This seems unnecessary. If they're that close, whatever averaging you do will keep it nearly centered
	float heading;
	if (((photo2 == photo1 + 1 && photo 3 == photo1 - 1) || (photo2 == photo1 - 1 && photo 3 == photo1 + 1)) && (val2 < val3 + 5 && val2 > val3 - 5)){
		//if largest reading is in betweeen next two and the next two are within +/- 5, assume that middle is pointing directly at it
		heading = 2*PI/16 * photo1;
		
	}
	else {
		///You never rotate by the offset by which phototransistor is selected.
		heading = 2*PI/16 * (photo1 * val1 + photo2 * val2) / (val1+val2); //compute weighted average and multiply by degrees per transistor
	}
	///Don't see the point of multiplying and dividing by 3. Doesn't really matter, because we need a lookup table based system
	///to get a decent distance measurement. You also will need to consider that the resistor changes and you need to check which is used.
	int distance = 3*(val1 + val2 + val3)/3; //need to scale accordingly
	return [distance*cos(heading),distance*sin(heading)];
}

///We definitely have enough information to implement this. Use the ymax etc. constants 
bool nearWall(int currentX, int currentY){
	return false;
}