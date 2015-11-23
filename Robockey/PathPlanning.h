#include "Localization.h"
#include "BAMSMath.h"
void goTo(Pose target, Pose current);
///Switch to using the Pose class (see Localization.h)
void goToPosition(Pose target, Pose current, bool faceForward);
void goToPositionSpin(Pose target, Pose current);
void goToPositionPuck(Pose target, Pose current);

bool facingPose(Pose target, Pose current);
void facePose(Pose target, Pose current);
void faceAngle(angle o, Pose current);