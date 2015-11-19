#pragma once

#include <stdint.h>
#include "Localization.h"

void initWireless();
void sendPacket(uint8_t *packet);
void sendRobotLocation();
void sendIR();
