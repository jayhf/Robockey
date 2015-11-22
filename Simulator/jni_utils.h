/*
 * globals.h
 *
 *  Created on: Nov 22, 2015
 *      Author: Jay
 */

#pragma once

#include <jni.h>

jobject getRobotObject();
void setRobot(jobject robot);

JNIEnv* getJNIEnv();
void setJNIEnv(JNIEnv *env);

jclass getClass();
