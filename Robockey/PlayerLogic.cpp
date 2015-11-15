/*
* PlayerLogic.cpp
*
* Created: 11/15/2015 11:14:10 AM
*  Author: Daniel Orol
*/

#include <stdint.h>
#include "Localization.h"

enum class Player : uint8_t{
	GOALIE = 0, DEFENSE = 1, SCORER = 2, ASSISTER = 3
};

void playerLogic(Player player){
	switch(player){
		case GOALIE:
		Pose puckPredict = predictPuck();
		if (puckPredict.x < XMAX / 2) {
			if (puckPredict.y >= 0) {
				uint16_t yPos = min(YMAX/2,puckPredict.y);
			}
			else{
				uint16_t yPos = max(YMIN/2,puckPredict.y);
			}
			goToPosition(Pose(XMIN + 10, yPos,puckPredict.o),getRobotPose(), true);
			facePose(puckPredict);
		}
		else if(puckPredict.x < 3*XMIN/4){
			facePose(puckPredict);
			movement(100,100);
			//communicate to other robot to fill in
		}
		else {
			if (puckPredict.y >= 0) {
				uint16_t yPos = min(YMAX/2,puckPredict.y);
			}
			else{
				uint16_t yPos = max(YMIN/2,puckPredict.y);
			}
			goToPosition(Pose(7*XMIN/8, yPos, puckPredict.o), getRobotPose(), true);
			facePose(puckPredict);
		}
		break;
		default:
		break;
	}
}