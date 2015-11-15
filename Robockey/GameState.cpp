/*
 * GameState.cpp
 *
 *  Created on: Nov 15, 2015
 *      Author: Jay
 */

#include "GameState.h"

uint8_t redScore;
uint8_t blueScore;
GameState state = GameState::NONE;

uint8_t getRScore(){
	return redScore;
}
uint8_t getBScore(){
	return blueScore;
}

bool allowedToMove(){
	return state == GameState::PLAY;
}
