/*
 * Robot.cpp
 *
 *  Created on: Nov 6, 2015
 *      Author: Jay
 */
#define SIMULATION
#include "Robot.h"
#include <stdint.h>
#include "../Robockey/Localization.h"

/*void findLocation(int16_t* position, int16_t* cameraData){

}*/

JNIEXPORT void JNICALL Java_Robot_mWiiUpdate (JNIEnv *env, jobject robot, jshortArray data, jshortArray location){
 	jshort* irData = env->GetShortArrayElements(data, NULL);
	uint16_t intData[12];
	for(int i=0;i<12;i++)
		intData[i] = irData[i];
	env->ReleaseShortArrayElements(data,irData,NULL);
	Pose pose = localize(intData);
	jshort* result = env->GetShortArrayElements(location, NULL);
	result[0] = pose.x;
	result[1] = pose.y;
	result[2] = pose.o;
	env->ReleaseShortArrayElements(location,result,NULL);
}
