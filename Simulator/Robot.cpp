/*
 * Robot.cpp
 *
 *  Created on: Nov 6, 2015
 *      Author: Jay
 */
#include "Robot.h"
#include <stdint.h>
//#include "../Robockey/main.cpp"
#include "../Robockey/Localization.h"
#include <math.h>
#include <stdio.h>
#include "../Robockey/Digital.h"
#include "jni_utils.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

JNIEXPORT void JNICALL Java_Robot_mWiiUpdate
  (JNIEnv *env, jclass, jshortArray data, jshortArray location){
	jshort* irData = env->GetShortArrayElements(data, NULL);
	uint16_t intData[12];
	for(int i=0;i<12;i++)
		intData[i] = irData[i];
	env->ReleaseShortArrayElements(data,irData,NULL);
	Pose pose = localizeRobot(intData);
	jshort* result = env->GetShortArrayElements(location, NULL);
	result[0] = pose.x;
	result[1] = pose.y;
	result[2] = pose.o;
	env->ReleaseShortArrayElements(location,result,NULL);
 	/*jshort* irData = env->GetShortArrayElements(data, NULL);
	uint16_t intData[12];
	for(int i=0;i<12;i++)
		intData[i] = irData[i];
	env->ReleaseShortArrayElements(data,irData,NULL);
	Pose pose = localize(intData);
	jshort* result = env->GetShortArrayElements(location, NULL);
	result[0] = pose.x;
	result[1] = pose.y;
	result[2] = pose.o;
	env->ReleaseShortArrayElements(location,result,NULL);*/
}

JNIEXPORT void JNICALL Java_Robot_run
  (JNIEnv *env, jobject robot){
	setJNIEnv(env);
	setRobot(robot);
	LEDColor color = LEDColor::BLUE;
	while(1){
		setLED(color);
		Sleep(1000);
		//color = static_cast<LEDColor>((1+static_cast<uint8_t>(color))%4);
	}
	//main();
}
