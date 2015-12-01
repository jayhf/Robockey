#pragma once

#include <stdint.h>
#include "Localization.h"
#include "GameState.h"

void initWireless();
void sendPacket(Robot robot, uint8_t messageID, uint8_t *packet);
void sendRobotLocation();
void sendIR();
void sendBattery();
void sendPuckPose();
void sendRobotMessage(Robot otherRobot, Pose location, uint8_t thisStrategy, uint8_t suggestedStrategy);
void processTeamMessage(uint8_t allyID, uint8_t *data);