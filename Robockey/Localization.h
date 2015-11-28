#pragma once

#include <stdint.h>
#include "BAMSMath.h"

#define YMAX 60
#define YMIN -60
#define XMAX 115
#define XMIN -115
#define ROBOT_RADIUS 15
#define PUCK_RADIUS 9

class Location;
class Pose{
public:
	Pose(int16_t x, int16_t y, angle o);
	Pose() : Pose(0,0,0){};
	int16_t x;
	int16_t y;
	angle o;
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
	inline Location getLocation();
};

class Location{
	public:
	Location(int8_t x, int8_t y);
	Location() : Location(0,0){};
	int8_t x;
	int8_t y;
	inline Location operator-(Location b){
		return Location(x-b.x, y-b.y);
	}
	inline Location operator+(Location b){
		return Location(x+b.x, y+b.y);
	}
	
	inline bool operator == (const Location &p){
		return x == p.x && y == p.y;
	}

	inline bool operator != (const Location &p){
		return !(*this == p);
	}
	inline Pose toPose(angle heading){
		return Pose(x,y,heading);
	}
	inline Pose toPose(){
		return toPose(0);
	}
};

inline Location Pose::getLocation(){
	return Location(x,y);
}

#define UNKNOWN_POSE (Pose(1023,1023,0))

Pose* getEnemyLocations();

Pose getPuckLocation();
Pose* getAllyLocations();
Pose getRobotPose();

angle getPuckHeading();

void initLocalization();
void findPuck(Pose current);

Pose predictPuck();
bool nearWall(Pose current);

Pose localizeRobot(uint16_t* irData);

void localizeRobot();

void updateEnemyLocations(int8_t *locations);

bool getStartPositive();
Pose getEnemyGoal();
