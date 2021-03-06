#pragma once

#include <stdint.h>

typedef uint16_t time;

#define ONE_SECOND 256
#define ONE_MINUTE (60*ONE_SECOND)

void initClock();
//Returns the current time in milliseconds. Only useful for relative timing.
time getTime();
//Puts the M2 to sleep until it is power cycled.
//Should be used when the battery gets low to save power and avoid damaging it.
void sleep();

bool timePassed(time t);