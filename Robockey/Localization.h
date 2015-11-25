#pragma once

#include <stdint.h>
#include "BAMSMath.h"

#define YMAX 60
#define YMIN -60
#define XMAX 115
#define XMIN -115
#define ROBOT_RADIUS 15
#define PUCK_RADIUS 68

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
	
	inline bool operator == (const Pose &p){
		return x == p.x && y == p.y && o == p.o;
	}

	inline bool operator != (const Pose &p){
		return !(*this == p);
	}
};

#define UNKNOWN_POSE (Pose(1023,1023,0))

Pose* getEnemyLocations();

Pose getPuckLocation();
Pose* getAllyLocations();
Pose getRobotPose();
Pose getRobotPose2();

angle getPuckHeading();

void initLocalization();
void findPuck(Pose current);

Pose predictPuck();
bool nearWall(Pose current);

Pose localizeRobot(uint16_t* irData);

void localizeRobot();
void localizeRobot2();

void updateEnemyLocations(int8_t *locations);
