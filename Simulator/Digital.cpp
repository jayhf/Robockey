#include "../Robockey/Digital.h"
#include "../Robockey/time.h"

///You don't properly handle negative speeds. Floating point math should be avoided.
///At the very least never divide by a constant, multiply by 1/constant.
void movement(int leftSpeed, int rightSpeed){}

void initDigital(){}

void setEnabled(bool enabled){}

void setMotors(int8_t right, int8_t left){}

uint16_t kickEndTime;
void startKick(uint16_t duration){}

void updateKick(){}

void setLED(LEDColor color){

}

bool switchesPressed(){
	return false;
}
