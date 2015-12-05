#pragma once

#include <stdint.h>
#include "Localization.h"
#include "GameState.h"

void initWireless();
void sendPacket(Robot robot, uint8_t *packet);
void sendPacket(Robot robot, uint8_t messageID, uint8_t *packet);
void sendRobotLocation();
void sendIR();
void sendBattery();
void sendPuckPose();
void updateWireless();
void sendAllyMessage(Robot ally);
void processTeamMessage(Ally ally, uint8_t *data);
void sendNextMessage();