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
extern "C"{
	#include "m_wii.h"
};
#endif
#include "BAMSMath.h"
#include "ADC.h"
#include "time.h"

Pose::Pose(int16_t x, int16_t y, int16_t o):
	x(x), y(y), o(o){
}

Pose enemyPoses[3];
Pose robotPose;
Pose allyPoses[2];

Pose puckPose[5];
uint16_t puckTime[5];

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


void initLocalization(){
	m_wii_open();
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

	uint16_t heading;
	if (((photo2 == photo1 + 1 && photo3 == photo1 - 1)
			|| (photo2 == photo1 - 1 && photo3 == photo1 + 1))
			&& (val2 < val3 + 5 && val2 > val3 - 5)){
		//if largest reading is in betweeen next two and the next two are within +/- 5, assume that middle is pointing directly at it
		heading = -(2*PI/16 * photo1);

	}
	else {
		///You never rotate by the offset by which phototransistor is selected.
		heading = -(2*PI/16 * (photo1 * val1 + photo2 * val2) / (val1+val2)); //compute weighted average and multiply by degrees per transistor
	}
	heading = current.o + heading;
	///Don't see the point of multiplying and dividing by 3. Doesn't really matter, because we need a lookup table based system
	///to get a decent distance measurement. You also will need to consider that the resistor changes and you need to check which is used.
	uint16_t distance = 3*(val1 + val2 + val3)/3; //need to scale accordingly
	for(int i = 0; i<4; i++) {
		puckPose[i+1] = puckPose[i];
		puckTime[i+1] = puckTime[i];
	}
	puckPose[0] = Pose(distance*cosb(heading) + current.x,distance*sinb(heading)+current.y,heading);
	puckTime[0] = getTime();
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
	return current.x > XMAX - 10 || current.x < XMIN + 10 || current.y > YMAX - 10 || current.y < YMIN +10;
}

void localizeRobot(){
	uint16_t irData[12];
	m_wii_read(irData);
	robotPose = localizeRobot(irData);
}

Pose localizeRobot(uint16_t* irData){
	float possiblePointsX[12];
	float possiblePointsY[12];
	int16_t possiblePointsO[12];
	int possiblePointCount = 0;
	int16_t irX[4] = {irData[0], irData[3], irData[6], irData[9]};
	int16_t irY[4] = {irData[1], irData[4], irData[7], irData[10]};
	if(irY[1] == 1023){
		return Pose(1023,1023,0);
	}
	int8_t validPoints = 0;
	int8_t errorPoints = 0;
	int8_t pointCount = 2 + ((irY[2] != 1023)?1:0)+((irY[3] != 1023)?1:0);
	for(int8_t i = 0; i<pointCount;i++){
		for(int8_t j = i+1; j<pointCount;j++){
			int16_t dx = irX[i]-irX[j];
			int16_t dy = irY[i]-irY[j];
			int16_t d = dx*dx + dy*dy;
			//System.out.println(d);
			int8_t id;
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
			float ox = mx + cy*dx + cx*px;
			float oy = my + cy*dy + cx*py;
			float ox2 = mx - cy*dx - cx*px;
			float oy2 = my - cy*dy - cx*py;
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
	fprintf(stdout,"]\n");*/
	float ox = 0;
	float oy = 0;
	int16_t oo = 0;
	if(possiblePointCount == 0){
		return Pose(1023,1023,0);
	}
	else if(possiblePointCount == 2){
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
		bool stop = false;
		int scores[12];
		for(int i=0;i<possiblePointCount && !stop;i++){
			for(int j=i+1;j<possiblePointCount;j++){
				float dx = possiblePointsX[i]-possiblePointsX[j];
				float dy = possiblePointsY[i]-possiblePointsY[j];
				int16_t dTheta = possiblePointsO[i]-possiblePointsO[j];
				if((dTheta>-16384&&dTheta<16384)&& dx*dx+dy*dy<200)
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
		//fprintf(stdout,"%d: (%f,%f,%d)\n",maxScoreIndex, possiblePointsX[maxScoreIndex],possiblePointsY[maxScoreIndex],possiblePointsO[maxScoreIndex]);
		int originCount = 0;
		for(int i=0;i<possiblePointCount;i++){
			float dx = possiblePointsX[i]-possiblePointsX[maxScoreIndex];
			float dy = possiblePointsY[i]-possiblePointsY[maxScoreIndex];
			int16_t dTheta = possiblePointsO[i]-possiblePointsO[maxScoreIndex];
			if((dTheta>-16384&&dTheta<16384)&&dx*dx+dy*dy<200){
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
	float coso = cos(3.14159f/32768*oo);
	float sino = sin(3.14159f/32768*oo);
	float rx = -ox*coso - oy *sino;
	float ry = ox*sino - oy *coso;
	//System.out.printf("(%f,%f,%f)\n",rx,ry,oo);
	rx*=10;
	ry*=10;
	return Pose(rx, ry, -oo);
}
