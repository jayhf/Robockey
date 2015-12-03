#pragma once

#include "Localization.h"
#include "BAMSMath.h"

void goTo(Pose target, Pose current);
///Switch to using the Pose class (see Localization.h)
void goToPosition(Pose target, Pose current);
void goToPositionSpin(Pose target, Pose current);
void goToPositionPuck(Pose target, Pose current);

bool facingLocation(Location target, Pose current);
void faceLocation(Location target, Pose current);
void faceAngle(angle o, Pose current);
void goToPuck(Pose target, Pose current);
bool stuck();

bool circleIntersectsSegment(Location p1, Location p2, Location c, uint8_t radius);
bool checkIntersection(Location p1, Location p2, uint8_t radius);
	

//Returns the length of the resulting path.
//Result is filled with the path, starting with the end point and going to the start
//Start is the first vertex and end is the second.
uint8_t findPath(uint8_t *result, Location *vertices, uint8_t vertexCount, Location *enemies, Pose *allies);