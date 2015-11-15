#pragma once

#include <stdint.h>
#include "BAMSMath.h"
#include "ADC.h"

#ifndef SIMULATION
extern "C"{
	#include "m_wii.h"
};
#else
	#include "../Simulator/m_wii.h"
#endif

#define YMAX 768
#define YMIN -768
#define XMAX 1024
#define XMIN -1024

class Pose{
public:
	Pose(int16_t x, int16_t y, int16_t o);
	Pose() : Pose(0,0,0){};
	int16_t x;
	int16_t y;
	int16_t o;
	inline Pose operator-(Pose b){
		return Pose(x-b.x, y-b.y, o-b.o);
	}
	inline Pose operator+(Pose b){
		return Pose(x+b.x, y+b.y, o+b.o);
	}
};

Pose::Pose(int16_t x, int16_t y, int16_t o):
	x(x), y(y), o(o){
}
Pose enemyPoses[3];
Pose puck;
Pose allyPoses[2];


Pose* getEnemyLocations(){
	return enemyPoses;
}

Pose getPuckLocation(){
	return puck;
}

Pose* getAllyLocations(){
	return allyPoses;
}
typedef int8_t byte;
typedef int8_t boolean;

void localizePuck(){
	uint16_t *irData = getIRData();
	localizePuck(irData);
}

void initLocalization(){
	m_wii_open();
}

Pose localizeRobot(uint16_t* irData);

void localizeRobot(){
	uint16_t data[16];
	m_wii_read(data);
	localizeRobot(data);
}

Pose localizeRobot(uint16_t* irData){
	float possiblePointsX[12];
	float possiblePointsY[12];
	short possiblePointsO[12];
	int possiblePointCount = 0;
	short irX[] = {(short)(irData[0]),(short)(irData[3]),(short)(irData[6]),(short)(irData[9])};
	short irY[] = {(short)(irData[1]),(short)(irData[4]),(short)(irData[7]),(short)(irData[10])};
	if(irY[1] == 1023){
		return Pose(1023,1023,0);
	}
	byte validPoints = 0;
	byte errorPoints = 0;
	for(byte i = 0; i<4;i++){
		if(irY[i]==1023)
			continue;
		for(byte j = (byte) (i+1); j<4;j++){
			if(irY[j]==1023)
				continue;
			short dx = (short) (irX[i]-irX[j]);
			short dy = (short) (irY[i]-irY[j]);
			short d = (short) (dx*dx + dy*dy);
			//System.out.println(d);
			byte id;
			if(d>5500){
				if(d>8950){
					if(d>11000) continue;
					else id = 5;
				}
				else{
					if(d>7100) id = 4;
					else id = 3;
				}
			}
			else if(d>2550){
				if(d>3800) id = 2;
				else id = 1;
			}
			else if(d>1500) id = 0;
			else continue;
			if((validPoints & (1<<id))==0)
				validPoints |= 1<<id;
			else{
				errorPoints |= 1<<id;
				continue;
			}
			short mx = (short) (irX[i]+irX[j]-1024);
			short my = (short) (irY[i]+irY[j]-768);
			short px = dy;
			short py = (short) -dx;
			float cx;
			float cy;
			short co;
			switch(id){
				case 0:
					cx = 2.014857231f;
					cy = 0.011916738f;
					co = 27951;
					break;
				case 1:
					cx = 1.189835575f;
					cy = 0.358868959f;
					co = -23947;
					break;
				case 2:
					cx = 0.761287104f;
					cy = -0.230281866f;
					co = 22225;
					break;
				case 3:
					cx = 0.45522679f;
					cy = 0.177359962f;
					co = -29904;
					break;
				case 4:
					cx = 0.503755633f;
					cy = -0.002927468f;
					co = 11567;
					break;
				case 5:
					cx = 0;
					cy = 0;
					co = -16384;
					break;
				default:
					continue;
			}
			float ox = mx + cy*dx + cx*px;
			float oy = my + cy*dy + cx*py;
			float ox2 = mx - cy*dx - cx*px;
			float oy2 = my - cy*dy - cx*py;
			short o = (short) (toBAMS(atan2((float)dy,(float)dx))+co);

			possiblePointsX[possiblePointCount] = ox;
			possiblePointsY[possiblePointCount] = oy;
			possiblePointsO[possiblePointCount] = (short)(o+PI);
			possiblePointsX[possiblePointCount+1] = ox2;
			possiblePointsY[possiblePointCount+1] = oy2;
			possiblePointsO[possiblePointCount+1] = o;
			possiblePointCount+=2;
			//System.out.printf("(%f,%f,%f) (%f,%f,%f)\n",ox,oy,o,ox2,oy2,(o+M_PI)%(2*M_PI));
		}
	}
	float ox = 0;
	float oy = 0;
	short oo = 0;
	if(possiblePointCount == 2){
		float d1 = possiblePointsX[0]*possiblePointsX[0]+possiblePointsY[0]*possiblePointsY[0];
		float d2 = possiblePointsX[1]*possiblePointsX[1]+possiblePointsY[1]*possiblePointsY[1];
		if(d1<d2){
			ox = possiblePointsX[1];
			oy = possiblePointsY[1];
			oo = possiblePointsO[1];
		}
		else{
			ox = possiblePointsX[0];
			oy = possiblePointsY[0];
			oo = possiblePointsO[0];
		}
	}
	else{
		boolean stop = false;
		int scores[12];
		for(int i=0;i<possiblePointCount && !stop;i++){
			for(int j=i+1;j<possiblePointCount;j++){
				float dx = possiblePointsX[i]-possiblePointsX[j];
				float dy = possiblePointsY[i]-possiblePointsY[j];

				short dTheta = (short) (possiblePointsO[i]-possiblePointsO[j]);
				if((dTheta>-PI/2&&dTheta<PI/2)&&dx*dx+dy*dy<200)
					scores[j]++;
			}
		}
		int maxScore = 0;
		int maxScoreIndex = 0;
		for(int i=0;i<possiblePointCount;i++)
			if(scores[i]>maxScore){
				maxScore = scores[i];
				maxScoreIndex = i;
			}
		int originCount = 0;
		for(int i=0;i<possiblePointCount;i++){
			float dx = possiblePointsX[i]-possiblePointsX[maxScoreIndex];
			float dy = possiblePointsY[i]-possiblePointsY[maxScoreIndex];
			short dTheta = (short) ((possiblePointsO[i]-possiblePointsO[maxScoreIndex]));

			if((dTheta>-16768&&dTheta<16768)&&dx*dx+dy*dy<200){
				originCount++;
				ox += possiblePointsX[i];
				oy += possiblePointsY[i];
				oo += dTheta;
			}
		}
		ox/=originCount;
		oy/=originCount;
		oo=(short) (oo/originCount+possiblePointsO[maxScoreIndex]);

	}
	float coso = (float) cos(toFloatAngle(oo));
	float sino = (float) sin(toFloatAngle(oo));
	float rx = -ox*coso - oy *sino;
	float ry = ox*sino - oy *coso;
	rx*=10;
	ry*=10;
	short ro = (short) -oo;
	return Pose((short)rx,(short)ry,(short)ro);

	/*int8_t xOffsets[6];
	int8_t yOffsets[6];
	int16_t estimatedPoints[18];
	uint16_t irX[4] = {irData[0],irData[3],irData[6],irData[9]};
	uint16_t irY[4] = {irData[1],irData[4],irData[7],irData[10]};
	if(irY[1] == 1023)
		return Pose(1023,1023,0);
	uint8_t validPoints = 0;
	uint8_t errorPoints = 0;
	uint8_t pointCount = 2 + (irY[2] != 1023)+(irY[3] != 1023);
	for(uint8_t i = 0; i<pointCount;i++){
		for(uint8_t j = i+1; j<pointCount;j++){
			int16_t dx = irX[i]-irX[j];
			int16_t dy = irY[i]-irY[j];
			uint16_t d = dx*dx + dy*dy;
			uint8_t id;
			if(d>5500){
				if(d>8950){
					if(d>11000) continue;
					else id = 5;
				}
				else{
					if(d>7100) id = 4;
					else id = 3;
				}
			}
			else if(d>2550){
				if(d>3800) id = 2;
				else id = 1;
			}
			else if(d>1500) id = 0;
			else continue;
			if(!validPoints & 1<<id)
				validPoints |= id;
			else{
				errorPoints |= id;
				continue;
			}
			int16_t mx = irX[i]+irX[j]-1024;
			int16_t my = irY[i]+irY[j]-768;
		}
	}
	validPoints &= ~errorPoints;
	int16_t x = 0;
	int16_t y = 0;
	int16_t o = 0;
	for(int i=0;i<6;i++){
		if(validPoints&1<<i){

		}
	}
	return Pose(23,validPoints,errorPoints);*/
}

