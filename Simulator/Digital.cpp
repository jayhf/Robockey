#include "../Robockey/Digital.h"
#include "../Robockey/time.h"
#include <jni.h>
#include "jni_utils.h"

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
	static jmethodID methodID = 0;
	if(methodID == 0)
		methodID = getJNIEnv()->GetMethodID(getClass(), "setLED","(I)V");
	if(methodID == 0)
		fprintf(stdout,"A");
	else{
		fprintf(stdout,"B");
		getJNIEnv()->CallVoidMethod(getRobotObject(), methodID, static_cast<int32_t>(color));
	}
}

bool switchesPressed(){
	return false;
}
