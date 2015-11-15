#pragma once

#include <math.h>

typedef int16_t angle;
typedef uint16_t uangle;

#define PI 32768
#define toBAMS(x) ((x)*10430.37835047045272f)
#define toFloatAngle(x) ((x*0.0000958737992428525f))

#define cosb(x) (cos(toFloatAngle(x)))
#define sinb(x) (sin(toFloatAngle(x)))
#define atan2b(y,x)  (toBAMS(atan2(y,x)))

#define angleGreaterb(x,y) ((x)-(y)>=0)
#define angleGreaterbu(x,y) ((x)-(y)<PI)
