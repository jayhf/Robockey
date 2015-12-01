#pragma once

#include <stdint.h>
#include "time.h"
#include "BAMSMath.h"

#define YMAX 60
#define YMIN -60
#define XMAX 115
#define XMIN -115
#define ROBOT_RADIUS 8
#define PUCK_RADIUS 4

class Location;
class Pose{
public:
	Pose(int16_t x, int16_t y, angle o) : x(x), y(y), o(o){}
	Pose() : Pose(0,0,0){}
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
	Location(int8_t x, int8_t y) : x(x), y(y){}
	Location() : Location(0,0){}
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

//units: cm/s
class Velocity{
public:
	int8_t x;
	int8_t y;
	Velocity(int8_t vx, int8_t vy) : x(vx), y(vy){}
	Velocity() : Velocity(0,0){}
};

#define UNKNOWN_POSE (Pose(255,255,0))
#define UNKNOWN_LOCATION (Location(255,255))

void initLocalization();
void updateLocalization();


angle getPuckHeading();

Location* getEnemyLocations();
Location getPuckLocation();
Location* getAllyLocations();
Pose getRobotPose();

Velocity getPuckVelocity();
Velocity* getEnemyVelocities();
Velocity* getAllyVelocities();
Velocity getVelocity();

//dt is in 1/256 of a second
Location predictPuck(uint8_t dt);
Location predictEnemy(uint8_t enemyIndex, uint8_t dt);
Location predictAlly(uint8_t allyID, uint8_t dt);
Location predictPose(uint8_t dt);

bool nearWall(Pose pose);
time getPuckUpdateTime();

//Used internally.
void updatePuckPosition();
Location findPuck();
Pose localizeRobot(uint16_t* irData);
void receivedEnemyLocations(int8_t *locations);
void receivedAllyUpdate(Pose pose, Location puckLocation, uint8_t allyID);
void kalmanFilter(Location &location, Velocity &velocity, Location measuredLocation, uint16_t &oldTime, uint16_t newTime);