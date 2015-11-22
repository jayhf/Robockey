#pragma once

/*
* PathPlanning.h
*
* Created: 11/11/2015 7:32:28 PM
*  Author: Daniel Orol
*/

#include "Localization.h"

void goTo(Pose target, Pose current);
void goToPosition(Pose target, Pose current, bool faceForward);
void goToPositionSpin(Pose target, Pose current);
void goToPositionPuck(Pose target, Pose current);


bool facingPose(Pose target, Pose current);
void facePose(Pose target, Pose current);
