#pragma once

#include "Localization.h"
#include "BAMSMath.h"

void goTo(Pose target, Pose current, bool backwards = false);
void goToPosition(Pose target, Pose current, bool toPuck, bool backwards = false,uint16_t speed = 1200);
void goToPositionSpin(Pose target, Pose current);
void goToPositionPuck(Pose target, Pose current);
bool facingHeading(angle target, Pose current);

bool facingLocation(Location target, Pose current);
bool facingLocation(Location target, Pose current,angle o);
void faceLocation(Location target, Pose current);
void faceLocation(Location target, Pose current,angle o);
void faceAngle(angle o, Pose current);
void goToPuck(Pose target, Pose current);
void goToBackwards(Pose target,Pose current);
bool atLocation(Location target, Location current);
bool circleIntersectsSegment(Location p1, Location p2, Location c, uint8_t radius);
bool checkIntersection(Location p1, Location p2, uint8_t radius);
	

//Returns the length of the resulting path.
//Result is filled with the path, starting with the end point and going to the start
//Start is the first vertex and end is the second.
uint8_t findPath(uint8_t *result, Location *vertices, uint8_t vertexCount, Location *enemies, Pose *allies);
bool atLocation(Location target, Location current);
bool atLocationWide(Location target, Location current);