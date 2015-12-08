#pragma once

#include <stdint.h>
#include "time.h"
#include "BAMSMath.h"
#include "GameState.h"

#define YMAX 60
#define YMIN -60
#define XMAX 115
#define XMIN -115
#define ROBOT_RADIUS 8
#define PUCK_RADIUS 4

class Location;
class Pose{
public:
	Pose(int8_t x, int8_t y, angle o) : x(x), y(y), o(o){}
	Pose() : Pose(127,127,0){}
	int8_t x;
	int8_t y;
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
	Location() : Location(127,127){}
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

#define UNKNOWN_POSE (Pose(127,127,0))
#define UNKNOWN_LOCATION (Location(127,127))

void initLocalization();
void updateLocalization();
void coordinatesFlipped();

bool puckVisible();
angle getPuckHeading();
uint8_t getPuckDistance();

//Location* getEnemyLocations();
Location getPuckLocation();
Location* getAllyLocations();
Pose getRobotPose();
Pose getLastKnownRobotPose();

Velocity getPuckVelocity();
//Velocity* getEnemyVelocities();
Velocity* getAllyVelocities();
Velocity getVelocity();

//dt is in 1/256 of a second
Location predictLocation(Location l, Velocity v, uint8_t dt);
Location predictPuck(uint8_t dt);
//Location predictEnemy(uint8_t enemyIndex, uint8_t dt);
Location predictAlly(uint8_t allyID, uint8_t dt);
Location predictPose(uint8_t dt);

bool nearWall(Pose pose);
time getPuckUpdateTime();
bool hasPuck();
bool recentlyHadPuck(time maxTime = ONE_SECOND);
bool stuck();

//Used internally.
void updatePuckPosition();
Location findPuck();
Pose localizeRobot(uint16_t* irData);
void receivedEnemyLocations(int8_t *locations);
void receivedAllyUpdate(Location location, Location puckLocation, Ally allyID);
void locationFilter(Location &location, Velocity &velocity, Location measuredLocation, uint16_t &oldTime, uint16_t newTime, uint8_t &certainty, uint8_t radius);