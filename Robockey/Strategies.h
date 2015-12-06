#pragma once

#include "GameState.h"

#define PICK_STRATEGY 0x3F
#define PICK_OFFENSE (PICK_STRATEGY | static_cast<uint8_t>(StrategyType::OFFENSE))
#define PICK_DEFENSE (PICK_STRATEGY | static_cast<uint8_t>(StrategyType::DEFENSE))
#define PICK_SCORER (PICK_STRATEGY | static_cast<uint8_t>(StrategyType::SCORER))
#define PICK_GOALIE (PICK_STRATEGY | static_cast<uint8_t>(StrategyType::GOALIE))
#define PICK_SOMETHING 0xFE
#define UNKNOWN_STRATEGY 0xFD

class Strategy{
public:
	Strategy(StrategyType type, uint8_t id);
	StrategyType getStrategyType();
	uint8_t getID();
	//Called once to update any internal variables before starting to use the strategy
	virtual void prepare(){};
	//Gets called every update cycle to control the robot with this strategy
	//Returns the next strategy. Free to use one of the above constants or this.
	virtual uint8_t run() = 0;
	//Fills the pointer with the suggested ally strategies for ally1 and ally2
	virtual void getSuggestedAllyStrategies(uint8_t *strategyIDs) = 0;
	//Returns how good a time it is for this strategy. Don't make it too slow (ie. no trig if possible)
	//Approximate values:
	//0 for do not choose this one
	//1 for there must be something better, but you can try this
	//16 for good strategy
	//etc. can go all the way to 255
	virtual uint8_t getPriority() = 0;
	
protected:
	StrategyType strategyType;
	uint8_t id;
};

Strategy *getStrategy(uint8_t strategyID);
bool isOffense(uint8_t strategyID);
bool isDefense(uint8_t strategyID);
bool isScorer(uint8_t strategyID);
bool isGoalie(uint8_t strategyID);
Strategy *getCurrentStrategy();
uint8_t getOurSuggestedStrategy(Ally ally);
void updateStrategies();

class StrategySelector{
public:
	StrategySelector(Strategy **strategies, uint8_t strategyCount, Strategy* defaultStrategy) :
	 strategies(strategies), strategyCount(strategyCount), defaultStrategy(defaultStrategy){}
	Strategy *pickStrategy();
private:
	Strategy **strategies;
	uint8_t strategyCount;
	Strategy *defaultStrategy;
};