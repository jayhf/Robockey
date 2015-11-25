/*
* Localization.cpp
*
* Created: 11/15/2015 7:04:11 PM
*  Author: Jay
*/

#include "Localization.h"

#ifdef _MSC_VER
#include "../Simulator/m_wii.h"
#else
#include <stdlib.h>
extern "C"{
	#include "m_wii.h"
	#include "m_usb.h"
};
#endif
#include "BAMSMath.h"
#include "ADC.h"
#include "time.h"
#include "stdio.h"
#define robotRadius 10
#define puckRadius 3


Pose::Pose(int16_t x, int16_t y, int16_t o):
x(x), y(y), o(o){
}

Pose enemyPoses[3];
Pose robotPose;
Pose allyPoses[2];
Pose robotPose2;

Pose puckPose[5];
uint16_t puckTime[5];
angle puckHeading = 0;

angle getPuckHeading(){
	return puckHeading;
}

Pose* getEnemyLocations(){
	return enemyPoses;
}

Pose getPuckLocation(){
	return puckPose[0];
}

Pose* getAllyLocations(){
	return allyPoses;
}
Pose getRobotPose(){
	return robotPose;
}
Pose getRobotPose2(){
	return robotPose2;
}


void initLocalization(){
	m_wii_open();
	//m_usb_init();
	localizeRobot();
}

void updateEnemyLocations(int8_t *locations){
	for(int i=0;i<3;i++){
		enemyPoses[i] = Pose(locations[3*i+1], locations[3*i+2], 0);
	}
}

void findPuck(Pose current){
	uint16_t val1 = 0;
	uint16_t val2 = 0;
	uint16_t val3 = 0;
	uint8_t photo1 = 0;
	uint8_t photo2 = 0;
	uint8_t photo3 = 0;

	uint16_t * values = getIRData(); // loop through transistors
	for (uint8_t i = 0; i < 16; i++){
		uint16_t thisADC = values[i];
		if (thisADC > val3) {
			if (thisADC > val2) {
				if (thisADC > val1){
					photo3 = photo2;
					photo2 = photo1;
					photo1 = i; //current pin
					val3 = val2;
					val2 = val1;
					val1 = thisADC;
				}
				else {
					photo3 = photo2;
					photo2 = i;
					val3 = val2;
					val2 = thisADC;
				}
			}
		}
	}
	m_usb_tx_int(photo1);
	m_usb_tx_char(',');
	m_usb_tx_int(photo2);
	m_usb_tx_char(',');
	m_usb_tx_int(photo3);
	m_usb_tx_char('\n');
	
	int16_t heading;
	if (((photo2 == photo1 + 15 && photo3 == photo1 - 15)
	|| (photo2 == photo1 - 15 && photo3 == photo1 + 15))){
		//if largest reading is in betweeen next two and the next two are within +/- 5, assume that middle is pointing directly at it
		heading = -(2*PI/16 * photo1 - 1) + PI;

	}
	else {
		///You never rotate by the offset by which phototransistor is selected.
		heading = -(2*PI/16 * (photo1 * val1 + photo2 * val2) / (val1+val2) - 1) + PI; //compute weighted average and multiply by degrees per transistor
	}
	m_usb_tx_int(heading);
	m_usb_tx_char('\n');
	int avg = 0;
	for (int i = 0; i < 5; i++){
		avg += puckPose[i].o;
	}
	avg = avg/5;
	heading = 0.3*(current.o + heading) + 0.7*avg;
	///Don't see the point of multiplying and dividing by 3. Doesn't really matter, because we need a lookup table based system
	///to get a decent distance measurement. You also will need to consider that the resistor changes and you need to check which is used.
	uint16_t distance = 0.1*(val1 + val2 + val3)/3; //need to scale accordingly
	for(int i = 0; i<4; i++) {
		puckPose[i+1] = puckPose[i];
		puckTime[i+1] = puckTime[i];
	}
	puckPose[0] = Pose(distance*cosb(heading) + current.x,distance*sinb(heading)+current.y,heading);
	puckTime[0] = getTime();
	puckHeading = heading;
}

Pose predictPuck(){
	
	uint16_t deltaT = puckTime[0] - puckTime[1];
	uint16_t deltaX = puckPose[0].x - puckPose[1].x;
	uint16_t deltaY = puckPose[0].y - puckPose[1].y;
	angle deltaO = puckPose[0].o - puckPose[1].o;
	Pose velocity(deltaX/deltaT,deltaY/deltaT,deltaO/deltaT);
	uint16_t timeStep = getTime() - puckTime[0];
	return Pose(puckPose[0].x + velocity.x*timeStep, puckPose[0].y + velocity.y*timeStep,puckPose[0].o + velocity.o*timeStep);
}

bool nearWall(Pose current){
	return current.x > XMAX - robotRadius || current.x < XMIN + robotRadius || current.y > YMAX - robotRadius || current.y < YMIN +robotRadius;
}

void localizeRobot(){
	uint16_t irData[12];
	m_wii_read(irData);
	robotPose = localizeRobot(irData);
}

Pose localizeRobot(uint16_t* irData){
	int16_t possiblePointsX[12];
	int16_t possiblePointsY[12];
	int16_t possiblePointsO[12];
	uint8_t possiblePointCount = 0;
	uint16_t irX[4] = {irData[0], irData[3], irData[6], irData[9]};
	uint16_t irY[4] = {irData[1], irData[4], irData[7], irData[10]};
	if(irY[1] == 1023){
		return Pose(1023,1023,0);
	}
	int8_t validPoints = 0;
	int8_t errorPoints = 0;
	//int8_t pointCount = 2 + ((irY[2] != 1023)?1:0)+((irY[3] != 1023)?1:0);
	for(int8_t i = 0; i<4;i++){
		if(irY[i]==1023)
			continue;
		for(int8_t j = i+1; j<4;j++){
			if(irY[j]==1023)
				continue;
			int16_t dx = irX[i]-irX[j];
			int16_t dy = irY[i]-irY[j];
			int16_t d = dx*dx + dy*dy;

			int8_t id;
			if(d>4500){
				if (d > 7500) {
					if (d > 9000)
						continue;
					else
						id = 5;
				} else {
					if (d > 5850)
						id = 4;
					else
						id = 3;
				}
			} else if (d > 2000) {
				if (d > 3100)
					id = 2;
				else
					id = 1;
			} else if (d > 1100)
				id = 0;
			else
				continue;
			/*if(d>5500){
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
			else continue;*/
			if((validPoints & (1<<id))==0)
			validPoints |= 1<<id;
			else{
				errorPoints |= 1<<id;
				continue;
			}
			int16_t mx = irX[i]+irX[j]-1024;
			int16_t my = irY[i]+irY[j]-768;
			int16_t px = dy;
			int16_t py = -dx;
			float cx;
			float cy;
			int16_t co;
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
			int16_t ox = mx + cy*dx + cx*px;
			int16_t oy = my + cy*dy + cx*py;
			int16_t ox2 = mx - cy*dx - cx*px;
			int16_t oy2 = my - cy*dy - cx*py;
			int16_t o = (int16_t)(32768/3.14159*(atan2((float)dy,(float)dx)))+co;

			possiblePointsX[possiblePointCount] = ox;
			possiblePointsY[possiblePointCount] = oy;
			possiblePointsO[possiblePointCount] = o+32768;
			possiblePointsX[possiblePointCount+1] = ox2;
			possiblePointsY[possiblePointCount+1] = oy2;
			possiblePointsO[possiblePointCount+1] = o;
			possiblePointCount+=2;
			//System.out.printf("(%f,%f,%f) (%f,%f,%f)\n",ox,oy,o,ox2,oy2,(o+Math.PI)%(2*Math.PI));
		}
	}
	/*validPoints &= ~errorPoints;
	short x = 0;
	short y = 0;
	short o = 0;
	for(int i=0;i<6;i++){
		if((validPoints&1<<i)!=0){

		}
	}*/
	//System.out.println(Arrays.toString(possiblePointsX));
	//System.out.println(Arrays.toString(possiblePointsY));
	//System.out.println(Arrays.toString(possiblePointsO));
	/*fprintf(stdout,"\n[");
	for(int i = 0; i < 12;i++)
			fprintf(stdout,"%f, ", possiblePointsX[i]);
	fprintf(stdout,"]\n[");
	for(int i = 0; i < 12;i++)
			fprintf(stdout,"%f, ", possiblePointsY[i]);
	fprintf(stdout,"]\n[");
	for(int i = 0; i < 12;i++)
			fprintf(stdout,"%d, ", possiblePointsO[i]);
	fprintf(stdout,"]\n[");*/
	int16_t ox = 0;
	int16_t oy = 0;
	int16_t oo = 0;
	if(possiblePointCount == 0){
		return Pose(1023,1023,0);
	}
	else if(possiblePointCount == 2){
		int32_t d1 = possiblePointsX[0]*possiblePointsX[0]+possiblePointsY[0]*possiblePointsY[0];
		int32_t d2 = possiblePointsX[1]*possiblePointsX[1]+possiblePointsY[1]*possiblePointsY[1];
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
		bool stop = false;
		uint8_t scores[12];
		for(uint8_t i=0;i<possiblePointCount;i++)
			scores[i] = 0;
		for(uint8_t i=0;i<possiblePointCount && !stop;i++){
			for(uint8_t j=i+1;j<possiblePointCount;j++){
				int16_t dx = possiblePointsX[i]-possiblePointsX[j];
				int16_t dy = possiblePointsY[i]-possiblePointsY[j];
				int16_t dTheta = possiblePointsO[i]-possiblePointsO[j];
				if((abs(dTheta)<16384)&& (dx*dx+dy*dy<400)){
					scores[j]++;
				}
			}
		}
		int maxScore = 0;
		int maxScoreIndex = 0;
		for(uint8_t i=0;i<possiblePointCount;i++)
			if(scores[i]>maxScore){
				maxScore = scores[i];
				maxScoreIndex = i;
			}

		//for(int i = 0; i < 12;i++)
		//	fprintf(stdout,"%d, ", scores[i]);
		//fprintf(stdout,"?] %d\n", possiblePointCount);
		//fprintf(stdout,"%d: (%f,%f,%d)\n",maxScoreIndex, possiblePointsX[maxScoreIndex],possiblePointsY[maxScoreIndex],possiblePointsO[maxScoreIndex]);

		int originCount = 0;
		for(uint8_t i=0;i<possiblePointCount;i++){
			int16_t dx = possiblePointsX[i]-possiblePointsX[maxScoreIndex];
			int16_t dy = possiblePointsY[i]-possiblePointsY[maxScoreIndex];
			int16_t dTheta = possiblePointsO[i]-possiblePointsO[maxScoreIndex];
			if((dTheta>-16384&&dTheta<16384)&&dx*dx+dy*dy<400){
				originCount++;
				ox += possiblePointsX[i];
				oy += possiblePointsY[i];
				if(i!=maxScoreIndex)
					oo += dTheta;
				//System.out.println("O"+possiblePointsO[i]);
			}
		}
		ox/=originCount;
		oy/=originCount;
		oo=(oo/originCount)+possiblePointsO[maxScoreIndex];

	}
	//fprintf(stdout,"(%f,%f,%d)\n",ox,oy,oo);
	
	float coso = cos(toFloatAngle(oo));
	float sino = sin(toFloatAngle(oo));
	int16_t rx = (-ox*coso - oy *sino)*(115.0f/768);
	int16_t ry = (ox*sino - oy *coso)*(115.0f/768);
	oo = PI/2-(oo-PI/2);
	//fprintf(stdout,"(%f,%f,%d)\n",rx,ry,oo);
	return Pose(-rx, ry, -oo);
}

void localizeRobot2(){
	uint16_t center[2] = {1024/2,768/2};
	//constellation center in pixels

	uint16_t data [12];
	m_wii_read(data);
	uint16_t datax[4] = {data[0], data[3], data[6], data[9]};
	uint16_t datay[4] = {data[1], data[4], data[7], data[10]};
	for (int i = 0; i<4; i++){
		if (datax[i] == 1023) return;
	}

	//calculate distance between every pair
	float d [6] = {sqrt((float)(datax[1]-datax[0])*(datax[1]-datax[0])+(datay[1]-datay[0])*(datay[1]-datay[0])),
		sqrt((float)(datax[2]-datax[0])*(datax[2]-datax[0])+(datay[2]-datay[0])*(datay[2]-datay[0])),
		sqrt((float)(datax[3]-datax[0])*(datax[3]-datax[0])+(datay[3]-datay[0])*(datay[3]-datay[0])),
		sqrt((float)(datax[2]-datax[1])*(datax[2]-datax[1])+(datay[2]-datay[1])*(datay[2]-datay[1])),
		sqrt((float)(datax[3]-datax[1])*(datax[3]-datax[1])+(datay[3]-datay[1])*(datay[3]-datay[1])),
	sqrt((float)(datax[3]-datax[2])*(datax[3]-datax[2])+(datay[3]-datay[2])*(datay[3]-datay[2]))};

	//calculate sum of distances from each point (in order 1, 2, 3, 4)
	float sum[4] = {d[0]+d[1]+d[2],d[0]+d[3]+d[4],d[1]+d[3]+d[5],d[2]+d[4]+d[5]};

	//sort sums into ascending order
	uint16_t index [4];
	for(int i = 0; i<3; i++){
		for(int j = 0; j<3; j++){
			if(sum[j] > sum[j+1]){
				uint16_t temp = sum[j+1];
				sum[j+1] = sum[j];
				sum[j] = temp;
				index[j] = j+1;
				index[j+1] = j;
			}
		}
	}

	//from drawing, note that each point has a unique sum and assign indices
	//accordingly
	uint8_t top = index[0];
	uint8_t right = index[2];
	uint8_t bottom = index[3];
	uint8_t left = index[1];

	//reassign matrix so it's in the order, T R B L
	uint16_t datax2[4] = {datax[top],datax[right],datax[bottom],datax[left]};
	uint16_t datay2[4] = {datay[top],datay[right],datay[bottom],datay[left]};

	//find center as midpoint of top and bottom point (relative to constellation)
	uint16_t offsetcenter[2] = {(datax2[0]+datax2[2])/2,(datay2[0]+datay2[2])/2};
	//find theta as offset of top from center
	float offsettheta = -atan2((float)(datax2[0]-datax2[2]),(float)(datay2[0]-datay2[2]));

	//put all points in x,y form
	uint16_t points [4]= {offsetcenter[0],offsetcenter[1], center[0], center[1]};
	//rotation matrix based on theta
	float rotationmatrix[4] = {cos(offsettheta), -sin(offsettheta),sin(offsettheta), cos(offsettheta)};
	//rotate the matrix
	float rotated[4] = {points[0]*rotationmatrix[0] + points[1]*rotationmatrix[2],points[0]*rotationmatrix[1]+points[1]*rotationmatrix[3],
		points[2]*rotationmatrix[0]+points[3]*rotationmatrix[2],points[2]*rotationmatrix[1]+points[3]*rotationmatrix[3]};
	//float uvect[2] = {rotationmatrix[0],rotationmatrix[1]};
	float dvect[2] = {rotated[2]-rotated[0],rotated[3]-rotated[1]};
	robotPose2 = Pose(dvect[0],dvect[1],toBAMS(offsettheta));
}
