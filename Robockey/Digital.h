#pragma once

#include <stdint.h>

void initDigital();
void setEnabled(bool enabled);
//Positive is forward. 0 is off and 1600 is 100% duty cycle
void setMotors(int16_t right, int16_t left);
void startKick();
void updateKick();
bool switchesPressed();

#undef OFF
enum class LEDColor : uint8_t{
	OFF=0, RED=1 ,BLUE=2, PURPLE=3
};

void setLED(LEDColor color);

///You don't properly handle negative speeds. Floating point math should be avoided.
///At the very least never divide by a constant, multiply by 1/constant.
void movement(int leftSpeed, int rightSpeed);
