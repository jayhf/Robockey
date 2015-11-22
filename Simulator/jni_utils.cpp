/*
 * globals.cpp
 *
 *  Created on: Nov 22, 2015
 *      Author: Jay
 */

#include "jni_utils.h"

jobject robot;
jclass robotClass;
JNIEnv* env = NULL;

jobject getRobotObject(){
	return robot;
}
void setRobot(jobject r){
	robot = r;
	robotClass = env->GetObjectClass(robot);
}

JNIEnv* getJNIEnv(){
	return env;
}
void setJNIEnv(JNIEnv *e){
	env = e;
}

jclass getClass(){
	return robotClass;
}
