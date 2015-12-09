/*
* Localization.cpp
*
* Created: 11/15/2015 7:04:11 PM
*  Author: Jay
*/

#include "Localization.h"

#define WU_AND_CHEN
//#define GM_LAB

#if defined(GM_LAB)
	#if defined(WU_AND_CHEN)
		#error "GM_LAB and WU_AND_CHEN cannot be defined together!"
	#else
		#pragma message "GM_LAB Selected"
	#endif
#else
	#if defined(WU_AND_CHEN)
		#pragma message "WU_AND_CHEN Selected"
	#else
		#error "Either GM_LAB or WU_AND_CHEN must be defined!"
	#endif
#endif

#ifdef _MSC_VER
#include "../Simulator/m_wii.h"
#else
#include <stdlib.h>
extern "C"{
	#include "m_wii.h"
};
#endif

#include "BAMSMath.h"
#include "ADC.h"
#include "stdio.h"
#include "GameState.h"
#include "Digital.h"
#include "wireless.h"

//Location newEnemyLocations[3];
//time newEnemyUpdateTime;

Location allyLocations[2];
Location allyPuckLocations[2];
//Location enemyLocations[3] = {Location(110,5),Location(-110,5), Location(-110,-10)};
//Velocity enemyVelocities[3];
//time enemyUpdateTime;
uint8_t robotPoseCertainty = 0;
Pose robotPose;
Pose lastKnownRobotPose;
Velocity robotVelocity;
time robotUpdateTime;
uint8_t puckCertainty = 0;
bool seePuck = false;
uint8_t photo = 0;
uint8_t puckDistance;
angle puckHeading;
Location puckLocationToSend;
Location puckLocation;
Velocity puckVelocity;
time puckUpdateTime;
time lastHadPuckTime = -ONE_MINUTE;
time lastMoveTime = -ONE_MINUTE;
Location lastMoveLocation = UNKNOWN_LOCATION;


void initLocalization(){
	m_wii_open();
	updateLocalization();
	//m_wait(100);
	//startPositive = getRobotPose().x >= 0;
}

void updateLocalization(){
	for(uint8_t i = 0; i<2; i++)
		if(!allyUpToDate(static_cast<Ally>(i))){
			allyPuckLocations[i] = UNKNOWN_LOCATION;
			allyLocations[i] = UNKNOWN_LOCATION;
		}
	uint16_t buffer[12];
	m_wii_read(buffer);
	Pose newRobotPose = localizeRobot(buffer);
	if(newRobotPose != UNKNOWN_POSE){
		static uint8_t wrongSide = 0;
		bool oldPoseUnknown = robotPose == UNKNOWN_POSE;
		if(!oldPoseUnknown){
			if(abs(robotPose.x)>10 && ((robotPose.x > 0) ^ (newRobotPose.x > 0))){
				wrongSide++;
				newRobotPose.x = -newRobotPose.x;
				newRobotPose.y = -newRobotPose.y;
				newRobotPose.o = PI + newRobotPose.o;
			}
			else{
				if(wrongSide > 0)
					wrongSide--;
			}
			if(wrongSide >= 16){
				robotPose.x = -robotPose.x;
				robotPose.y = -robotPose.y;
				robotPose.o = robotPose.o + PI;
				wrongSide = 0;
			}
		}
		Location robotLocation = robotPose.getLocation();
		locationFilter(robotLocation, robotVelocity, newRobotPose.getLocation(), robotUpdateTime, getTime(),robotPoseCertainty,15);
		robotPose.x = robotLocation.x;
		robotPose.y = robotLocation.y;
		static uint8_t headingWrong = 0;
		if(oldPoseUnknown){
			robotPose.o = newRobotPose.o;	
		}
		else{
			if(abs(robotPose.o - newRobotPose.o) > PI/2 ){
				headingWrong++;
				robotPose.o = PI + newRobotPose.o;
			}
			else{
				if(headingWrong > 0)
					headingWrong--;
				robotPose.o = newRobotPose.o;
			}
			if(headingWrong >= 8){
				robotPose.o += PI;
				headingWrong = 0;
			}
		}
		if(robotPoseCertainty >= 4)
			lastKnownRobotPose = robotPose;
		if(irDataFresh())
			updatePuckPosition();
		determineTeam();
		
		if(lastMoveLocation == UNKNOWN_LOCATION)
			lastMoveLocation = robotLocation;
			
		if(distanceSquared(lastMoveLocation,robotLocation) > 15){
			lastMoveTime = getTime();
			lastMoveLocation = robotLocation;
		}
		else if(timePassed(lastMoveTime + ONE_MINUTE))
			lastMoveTime = getTime() - ONE_MINUTE;
		
	}
	//for(int i=0;i<2;i++)
	//	locationFilter(allyLocations[i],allyVelocities[i],newAllyLocations[i], allyUpdateTimes[i], newAllyUpdateTimes[i],allyLocationCertainty[i],15);
	//for(int i=0;i<3;i++)
	//	locationFilter(enemyLocations[i],enemyVelocities[i],newEnemyLocations[i], enemyUpdateTime, newEnemyUpdateTime);
}


void coordinatesFlipped(){
	robotPose.x = -robotPose.x;
	robotPose.y = -robotPose.y;
	robotPose.o = PI + robotPose.o;
}

void updatePuckPosition(){
	puckLocationToSend = findPuck();
	
	if(hasPuck())
		lastHadPuckTime = getTime();
	else if(timePassed(lastHadPuckTime + ONE_MINUTE))
		lastHadPuckTime = getTime() - ONE_MINUTE - 1;

	int16_t totalPuckX = 0;
	int16_t totalPuckY = 0;
	uint8_t totalWeight = 0;
	
	if(allyUpToDate(Ally::ALLY1) && allyPuckLocations[0] != UNKNOWN_LOCATION){
		uint16_t allyPuckLocationWeight1 = 1;
		totalPuckX += allyPuckLocations[0].x * allyPuckLocationWeight1;
		totalPuckY += allyPuckLocations[0].y * allyPuckLocationWeight1;
		totalWeight += allyPuckLocationWeight1;
	}
	if(allyUpToDate(Ally::ALLY2) && allyPuckLocations[1] != UNKNOWN_LOCATION){
		uint16_t allyPuckLocationWeight2 = 1;
		totalPuckX += allyPuckLocations[1].x * allyPuckLocationWeight2;
		totalPuckY += allyPuckLocations[1].y * allyPuckLocationWeight2;
		totalWeight += allyPuckLocationWeight2;
	}
	if(puckLocationToSend != UNKNOWN_LOCATION){
		uint16_t puckLocationWeight = 2;
		totalPuckX += puckLocationToSend.x * puckLocationWeight;
		totalPuckY += puckLocationToSend.y * puckLocationWeight;
		totalWeight += puckLocationWeight;
	}
	
	Location averagePuckLocation;
	
	if(totalWeight == 0)
		averagePuckLocation = UNKNOWN_LOCATION;
	else{
		averagePuckLocation.x = totalPuckX/totalWeight;
		averagePuckLocation.y = totalPuckY/totalWeight;
	}
	
	time currentTime = getTime();
	locationFilter(puckLocation, puckVelocity, averagePuckLocation, puckUpdateTime, currentTime/*-(dt>>1)*/, puckCertainty,30);
}

bool hasPuck(){
	
	if(puckVisible() && (puckHeading >= -2048) && (puckHeading <= 2048)&&(puckDistance<13)){
		//m_green(1);
		return true;
	}
	//m_green(0);
	return false;
}

bool recentlyHadPuck(time maxTime){
	return timePassed(lastHadPuckTime + maxTime);
}


bool recentlyMoved(time maxTime){
	return timePassed(lastMoveTime + maxTime);
}

void locationFilter(Location &location, Velocity &velocity, Location measuredLocation, time &oldTime, time newTime, uint8_t &certainty,uint8_t radius){
	time dt = newTime - oldTime;
	if(dt > certainty * (ONE_SECOND/8)){
		location = UNKNOWN_LOCATION;
		velocity = Velocity(0,0);
		oldTime = newTime;
		certainty = 0;
	}
	if(measuredLocation == UNKNOWN_LOCATION){
		return;
	}
	else if(location == UNKNOWN_LOCATION || certainty == 0){
		location = measuredLocation;
		oldTime = newTime;
		certainty = 2;
		return;
	}
	else{
		Location predictedLocation = predictLocation(location,velocity,newTime - oldTime);
		
		if(predictedLocation.x > (int16_t)measuredLocation.x + radius || predictedLocation.x < (int16_t)measuredLocation.x - radius ||
			predictedLocation.y > (int16_t)measuredLocation.y + radius || predictedLocation.y < (int16_t)measuredLocation.y - radius){
			if(certainty < 8)
				certainty--;
		}
		else{
			int8_t nextX = ((int16_t)predictedLocation.x+(int16_t)measuredLocation.x)>>1;
			int8_t nextY = ((int16_t)predictedLocation.y+(int16_t)measuredLocation.y)>>1;
			Location nextLocation(nextX,nextY);
			Velocity estimatedVelocity((((int16_t)nextX-location.x)<<8)/dt,(((int16_t)nextY-location.y)<<8)/dt);
			velocity = Velocity((3*velocity.x+estimatedVelocity.x)>>2,(3*velocity.y+estimatedVelocity.y)>>2);
			location = nextLocation;
			oldTime = newTime;
			if(certainty < 8)
				certainty++;
		}
	}
}


//Location* getEnemyLocations(){
//	return enemyLocations;
//}
Location getPuckLocation(){
	if(puckCertainty > 4)
		return puckLocation;
	else
		return UNKNOWN_LOCATION;
}
Location* getAllyLocations(){
	return allyLocations;
}

Pose getRobotPose(){
	return robotPose;
}

Velocity getPuckVelocity(){
	return puckVelocity;
}

Pose getLastKnownRobotPose(){
	return lastKnownRobotPose;
}

//Velocity* getEnemyVelocities(){
//	return enemyVelocities;
//}

Velocity getVelocity(){
	return robotVelocity;
}

Location predictLocation(Location l, Velocity v, uint8_t dt){
	if(l == UNKNOWN_LOCATION)
		return UNKNOWN_LOCATION;
	return Location(l.x+(int8_t)(((uint16_t)dt*v.x)>>8),l.y+(uint8_t)(((uint16_t)dt*v.y)>>8));
}

Location predictPuck(uint16_t dt){
	return predictLocation(puckLocation, puckVelocity, dt);
}

/*Location predictEnemy(uint8_t enemyIndex, uint16_t dt){
	Location location = enemyLocations[enemyIndex];
	Velocity velocity = enemyVelocities[enemyIndex];
	return Location(location.x+(uint8_t)((((uint16_t)dt*velocity.x)>>8)),location.y+(uint8_t)((((uint16_t)dt*velocity.y)>>8)));
}*/


Location predictPose(uint16_t dt){
	return predictLocation(robotPose.getLocation(), robotVelocity, dt);
}

void receivedAllyUpdate(Location location, Location puckLocation, Ally ally){
	uint8_t allyID = static_cast<uint8_t>(ally);
	allyLocations[allyID] = location;
	allyPuckLocations[allyID] = puckLocation;
}

/*void receivedEnemyLocations(int8_t *locations){
	newEnemyUpdateTime = getTime();
	for(int i=0;i<3;i++){
		newEnemyLocations[i] = flipCoordinates() ? Location(-locations[3*i+1], -locations[3*i+2]) : Location(locations[3*i+1], locations[3*i+2]);
	}
}*/

#ifdef WU_AND_CHEN
#define PUCK_VISIBLE_BOUND 290
uint16_t intensities1K[] = {55,65,150,350,1023};
uint8_t distances1K[] = {12,11,10,9,8};
uint16_t intensities6K8[] = {0,45,80,190,330,650,1023};
uint8_t distances6K8[] = {40,25,20,16,12,10,9};
uint16_t intensities47K[] = {0,67,105,125,180,300,510,860,980,1023};
uint8_t distances47K[] = {150,50,40,35,30,25,20,16,14,13};
uint16_t intensities330K[] = {280,315,330,355,380,450,565,750,875,985,1023};
uint8_t distances330K[] = {210,160,130,100,80,60,50,40,35,30,25};
#endif
#ifdef GM_LAB
#define PUCK_VISIBLE_BOUND 320
uint16_t intensities1K[] = {0,160,260,1023};
uint8_t distances1K[] = {11,10,9,8};
uint16_t intensities6K8[] = {0,150,300,650,1023};
uint8_t distances6K8[] = {18,16,12,10,9};
uint16_t intensities47K[] = {0,150,290,400,500,650,850,1023};
uint8_t distances47K[] = {34,30,26,22,20,18,16,14};
uint16_t intensities330K[] = {0,315,320,340,360,410,450,525,600,700,775,850,1023};
uint8_t distances330K[] = {255,200,150,120,100,80,70,60,50,45,40,35,31};
#endif

Location findPuck(){
	uint16_t val = 0;
	uint16_t * values = getIRData(false); // loop through transistors
	for (uint8_t i = 0; i < 16; i++){
		uint16_t thisADC = values[i];
		if (thisADC > val) {
			val=thisADC;
			photo=i;
		}
	}
	uint8_t photol = (photo - 1) & 0xF;
	uint8_t photor = (photo + 1) & 0xF;
	int16_t leftValue = values[photo] - values[photol];
	int16_t rightValue = values[photo] - values[photor];
	angle heading = PI + (PI/8) * photo + PI/16;
	int16_t max = MAX(abs(leftValue),abs(rightValue));
	if(max!=0)
		heading += (PI/8)*((leftValue-rightValue)/(float)max); //compute weighted average and multiply by degrees per transistor

	uint8_t intensityCount;
	uint16_t* intensities;
	uint8_t* distances;
	switch(getSelectedResistor()){
		case Resistor::R1K:
			intensities = intensities1K;
			distances = distances1K;
			intensityCount = sizeof(intensities1K)/sizeof(uint16_t);
			break;
		case Resistor::R6K8:
			intensities = intensities6K8;
			distances = distances6K8;
			intensityCount = sizeof(intensities6K8)/sizeof(uint16_t);
			break;
		case Resistor::R47K:
			intensities = intensities47K;
			distances = distances47K;
			intensityCount = sizeof(intensities47K)/sizeof(uint16_t);
			break;
		case Resistor::R330K:
			intensities = intensities330K;
			distances = distances330K;
			intensityCount = sizeof(intensities330K)/sizeof(uint16_t);
			break;
		default://never gets here, but need to keep the compiler happy
			return UNKNOWN_LOCATION;
			seePuck = false;
	}
	uint16_t intensity = values[photo];
	uint8_t distance = 0xFF;
	for(unsigned int i=1;i<intensityCount;i++){
		if(intensities[i]>intensity){
			float ratio = (intensity-intensities[i-1])/(float)(intensities[i]-intensities[i-1]);
			distance = (1-ratio) * distances[i-1]+ratio*distances[i];
			break;
		}
	}
	if(distance == 0xFF || (getSelectedResistor() == Resistor::R330K && values[photo] < PUCK_VISIBLE_BOUND)){
		seePuck = false;
		return UNKNOWN_LOCATION;
	}
	seePuck = true;
	Pose robot = getRobotPose();
	float puckX = distance * cosb(heading + robot.o) + robot.x;
	float puckY = distance * sinb(heading + robot.o) + robot.y;
	if(puckX > XMAX + 5 || puckX < XMIN - 5 || puckY > YMAX + 5 || puckY < YMIN - 5){
		seePuck = false;
		return UNKNOWN_LOCATION;
	}
	puckDistance = distance;
	puckHeading = heading;
	return Location((int8_t)puckX,(int8_t)puckY);
}

bool puckVisible(){
	return seePuck;
}

angle getPuckHeading(){
	return puckHeading;
}

uint8_t getPuckDistance(){
	return puckDistance;
}

time getPuckUpdateTime(){
	return puckUpdateTime;
}

/*Pose predictPuck(){
	
	uint16_t deltaT = puckTime[0] - puckTime[1];
	uint16_t deltaX = puckPose[0].x - puckPose[1].x;
	uint16_t deltaY = puckPose[0].y - puckPose[1].y;
	angle deltaO = puckPose[0].o - puckPose[1].o;
	Pose velocity(deltaX/deltaT,deltaY/deltaT,deltaO/deltaT);
	uint16_t timeStep = getTime() - puckTime[0];
	return Pose(puckPose[0].x + velocity.x*timeStep, puckPose[0].y + velocity.y*timeStep,puckPose[0].o + velocity.o*timeStep);
}*/

bool nearWall(Pose current){
	return current.x > XMAX - 2*ROBOT_RADIUS || current.x < XMIN + 2*ROBOT_RADIUS || current.y > YMAX - 2*ROBOT_RADIUS || current.y < YMIN +2*ROBOT_RADIUS;
}

void localizeRobot(){
	uint16_t irData[12];
	m_wii_read(irData);
	//robotPose = localizeRobot(irData);
	robotPose = Pose(0,0,0);
}

Pose localizeRobot(uint16_t* irData){
	int16_t possiblePointsX[12];
	int16_t possiblePointsY[12];
	int16_t possiblePointsO[12];
	uint8_t possiblePointCount = 0;
	uint16_t irX[4] = {irData[0], irData[3], irData[6], irData[9]};
	uint16_t irY[4] = {irData[1], irData[4], irData[7], irData[10]};
	//if(irY[1] == 1023){
	//	return UNKNOWN_POSE;
	//}
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
			#ifdef GM_LAB
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
			#endif
			#ifdef WU_AND_CHEN
			if(d>5050){
				if (d > 8300) {
					if (d > 10000)
						continue;
					else
						id = 5;
					}
				else {
					if (d > 6500)
						id = 4;
					else
						id = 3;
				}
			} else if (d > 2300) {
				if (d > 3600)
					id = 2;
				else
					id = 1;
			} else if (d > 1350)
				id = 0;
			else
			continue;
			#endif
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
		return UNKNOWN_POSE;
	}
	else if(possiblePointCount == 2){
		int32_t d1 = (int32_t)possiblePointsX[0]*possiblePointsX[0]+(int32_t)possiblePointsY[0]*possiblePointsY[0];
		int32_t d2 = (int32_t)possiblePointsX[1]*possiblePointsX[1]+(int32_t)possiblePointsY[1]*possiblePointsY[1];
		if(d1>d2){
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
	#ifdef WU_AND_CHEN
		ry+=5;
	#endif
	if(rx > XMAX + 5 || rx < XMIN - 5 || ry > YMAX + 5 || ry < YMIN - 5){
		return UNKNOWN_POSE;
	}
	//fprintf(stdout,"(%f,%f,%d)\n",rx,ry,oo);
	if(flipCoordinates())
		return Pose(rx,-ry,PI-oo);
	else
		return Pose(-rx, ry, -oo);
}

bool stuck(){
	bool frontWall = robotPose.x>XMAX-2.5*ROBOT_RADIUS && robotPose.o <3500 && robotPose.o >-3500;
	bool rightWall = robotPose.y<YMIN+2.5*ROBOT_RADIUS && robotPose.o <3500 - PI/2 && robotPose.o >-3500 - PI/2;
	bool backWall = robotPose.x<XMIN+2.5*ROBOT_RADIUS  && robotPose.o <3500 + PI && robotPose.o >-3500 + PI;
	bool leftWall = robotPose.x>YMAX-2.5*ROBOT_RADIUS && robotPose.o <3500 + PI/2 && robotPose.o >-3500 + PI/2;
	return backWall||frontWall||leftWall||rightWall;
}

inline uint16_t distanceSquared(Location l1, Location l2){
	int16_t dx = (int16_t)(l1.x)-l2.x;
	int16_t dy = (int16_t)(l1.y)-l2.y;
	return dx*dx+dy*dy;
}

uint16_t distanceSquared(Pose p, Location l){
	return distanceSquared(p.getLocation(),l);
}

uint16_t distanceSquared(Pose p1, Pose p2){
	return distanceSquared(p1.getLocation(),p2.getLocation());
}

uint8_t distance(Location l1, Location l2){
	return (uint8_t)sqrt(distanceSquared(l1,l2));
}

uint8_t distance(Pose p, Location l){
	return (uint8_t)sqrt(distanceSquared(p,l));
}

uint8_t distance(Pose p1, Pose p2){
	return (uint8_t)sqrt(distanceSquared(p1,p2));
}

uint8_t Location::distanceTo(Location location){
	return distance(*this,location);
}
uint16_t Location::distanceToSquared(Location location){
	return distance(*this,location);
}
uint8_t Location::distanceTo(Pose pose){
	return distance(pose,*this);
}
uint16_t Location::distanceToSquared(Pose pose){
	return distanceSquared(pose,*this);
}

uint8_t Pose::distanceTo(Location location){
	return distance(*this,location);
}
uint16_t Pose::distanceToSquared(Location location){
	return distance(*this,location);
}
uint8_t Pose::distanceTo(Pose pose){
	return distance(*this,pose);
}
uint16_t Pose::distanceToSquared(Pose pose){
	return distanceSquared(*this,pose);
}