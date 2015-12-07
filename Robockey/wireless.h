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
void sendAllyMessage(Ally ally);
void processTeamMessage(Ally ally, uint8_t *data);
void sendNextMessage();
bool hasPuck(Ally ally);
uint8_t getAllyStrategy(Ally ally);
uint8_t getAllySuggestedStrategy(Ally ally);
bool allyUpToDate(Ally ally);
bool hasPuck(Ally ally);
Ally getHighestPriorityAlly();
bool allyHigherPriorityThanMe(Ally ally);
void handleGameStateMessage(uint8_t id);