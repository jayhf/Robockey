﻿#include "Strategies.h"
#include "Digital.h"
#include "stdlib.h"
#include "wireless.h"
//TODO remove the following
#include <avr/io.h>
#include "miscellaneous.h"
#include "PlayerLogic.h"

Strategy currentStrategy = Strategy::DO_NOTHING;
Strategy suggestedAllyStrategies[2];

Strategy getCurrentStrategy(){
	return currentStrategy;
}
Strategy getOurSuggestedStrategy(Ally ally){
	return suggestedAllyStrategies[static_cast<uint8_t>(ally)];
}

Strategy pickStrategy(){
	/*Strategy suggestion1 = getAllySuggestedStrategy(Ally::ALLY1);
	Strategy suggestion2 = getAllySuggestedStrategy(Ally::ALLY2);
	
	if(suggestion1 != Strategy::PICK_SOMETHING && allyUpToDate(Ally::ALLY1)){
		if(suggestion2 != Strategy::PICK_SOMETHING && allyUpToDate(Ally::ALLY2)){
			if(suggestion1 == suggestion2)
				return suggestion1;
		}
		else{
			return getAllySuggestedStrategy(Ally::ALLY1);
		}
	}
	else{
		if(getAllySuggestedStrategy(Ally::ALLY2) != Strategy::PICK_SOMETHING && allyUpToDate(Ally::ALLY2)){
			return getAllySuggestedStrategy(Ally::ALLY2);
		}
	}*/
	if(getThisRobot() == Robot::ROBOT1)
		return Strategy::GOALIE;
	else if(getThisRobot() == Robot::ROBOT2)
		return Strategy::SCORE_PUCK;
	else
		return Strategy::SWEEP;
	/*uint8_t ally1X = getAllyLocations()[0].x;
	bool ally1LocationValid = getAllyLocations()[0] != UNKNOWN_LOCATION && allyUpToDate(Ally::ALLY1);
	uint8_t ally2X = getAllyLocations()[1].x;
	bool ally2LocationValid = getAllyLocations()[1] != UNKNOWN_LOCATION && allyUpToDate(Ally::ALLY2);
	uint8_t robotX = getRobotPose().x;
	bool ally1HasPuck = hasPuck(Ally::ALLY1);
	bool ally2HasPuck = hasPuck(Ally::ALLY2);
	Strategy ally1Strategy = ally1LocationValid ? getAllyStrategy(Ally::ALLY1) : Strategy::DO_NOTHING;
	Strategy ally2Strategy = ally2LocationValid ? getAllyStrategy(Ally::ALLY2) : Strategy::DO_NOTHING;
	if(recentlyHadPuck() || ally1HasPuck || ally2HasPuck){
		if(recentlyHadPuck())
			return Strategy::SCORE_PUCK;
		if(ally1HasPuck){
			if(ally2LocationValid && ally2X < robotX)
				return Strategy::DEFENSE;
			else
				return Strategy::GOALIE;
		}
		else{
			if(ally1LocationValid && ally1X < robotX)
				return Strategy::DEFENSE;
			else
				return Strategy::GOALIE;
		}
	}
	else{
		if(currentStrategy == Strategy::GOALIE && ally1Strategy != Strategy::GOALIE && ally2Strategy != Strategy::GOALIE)
			return Strategy::GOALIE;
		if(ally1Strategy == Strategy::GOALIE || ally2Strategy == Strategy::GOALIE){
			return Strategy::DEFENSE;
		}
		if((!ally1LocationValid || robotX < ally1X + 3) && (!ally2LocationValid || robotX < ally2X + 3))
			return Strategy::GOALIE;
		return Strategy::DEFENSE;
	}*/
	/*if(hasPuck())
		return Strategy::SCORE_PUCK;
	else
		return Strategy::DEFENSE;*/
}

void updateStrategies(){
	Strategy newStrategy = pickStrategy();
	if(newStrategy != currentStrategy){
		resetPoints();
		currentStrategy = newStrategy;
	}
	suggestedAllyStrategies[0] = Strategy::PICK_SOMETHING;
	suggestedAllyStrategies[1] = Strategy::PICK_SOMETHING;
	switch(currentStrategy){
		case Strategy::DO_NOTHING:
		case Strategy::PICK_SOMETHING:
			setMotors(0,0);
			break;
		case Strategy::GOALIE:
			goalieLogic2();
			break;
		case Strategy::DEFENSE:
		setMotors(0,0);
			break;
		case Strategy::SWEEP:
			pushGoalie();
			break;
		case Strategy::PUSH_ALLY:
			pushAlly();
			break;
		case Strategy::SCORE_PUCK:
			/*if(!recentlyMoved()){
				if(getAllyStrategy(Ally::ALLY1)!=Strategy::GOALIE)
					suggestedAllyStrategies[0] = Strategy::PUSH_ALLY;
				else if(getAllyStrategy(Ally::ALLY2)!=Strategy::GOALIE)
					suggestedAllyStrategies[1] = Strategy::PUSH_ALLY;
			}*/
			scoreLogic();
			break;
	}
}

/*Strategy::Strategy(StrategyType strategyType, uint8_t strategyId) : 
	strategyType(strategyType),
	id(strategyId | static_cast<uint8_t>(strategyType)){
}

StrategyType Strategy::getStrategyType(){
	return strategyType;
}

uint8_t Strategy::getID(){
	return id;
}

class DoNothingStrategy : public Strategy{
public:
	DoNothingStrategy(StrategyType strategyType, uint8_t id) : Strategy(strategyType, id){
		
	}

	uint8_t run(uint8_t *strategyIDs) override{
		setMotors(0,0);
		return PICK_SOMETHING;
	}
	
	uint8_t getPriority(){
		return 0;
	}
	
};

class LEDStrategy : public Strategy{
public:
	LEDStrategy(StrategyType strategyType, uint8_t id) : Strategy(strategyType, id){
		
	}
	uint8_t run(uint8_t *strategyIDs) override{
		switch(strategyType){
			case StrategyType::DEFENSE:
				setLED(LEDColor::BLUE);
				break;
			case StrategyType::OFFENSE:
				setLED(LEDColor::RED);
				break;
			case StrategyType::SCORER:
				setLED(LEDColor::PURPLE);
				break;
			case StrategyType::GOALIE:
				setLED(LEDColor::OFF);
				break;
		}
		strategyIDs[0] = PICK_DEFENSE;
		strategyIDs[1] = PICK_OFFENSE;
		return 1 | static_cast<uint8_t>(StrategyType::SCORER);
	}
	
	uint8_t getPriority(){
		return 10;
	}
};

DoNothingStrategy doNothingOffense(StrategyType::OFFENSE, 0);
DoNothingStrategy doNothingDefense(StrategyType::DEFENSE, 0);
DoNothingStrategy doNothingScorer(StrategyType::SCORER, 0);
DoNothingStrategy doNothingGoalie(StrategyType::GOALIE, 0);

LEDStrategy ledOffense(StrategyType::OFFENSE, 1);
LEDStrategy ledDefense(StrategyType::DEFENSE, 1);
LEDStrategy ledScorer(StrategyType::SCORER, 1);
LEDStrategy ledGoalie(StrategyType::GOALIE, 1);

Strategy *offenseStrategies[] = {&doNothingOffense, &ledOffense};
Strategy *defenseStrategies[] = {&doNothingDefense, &ledDefense};
Strategy *scorerStrategies[] = {&doNothingScorer, &ledScorer};
Strategy *goalieStrategies[] = {&doNothingGoalie, &ledGoalie};

StrategySelector offenseSelector(offenseStrategies,sizeof(offenseStrategies)/sizeof(Strategy*), &doNothingOffense);
StrategySelector defenseSelector(defenseStrategies,sizeof(defenseStrategies)/sizeof(Strategy*), &doNothingDefense);
StrategySelector scorerSelector(scorerStrategies,sizeof(scorerStrategies)/sizeof(Strategy*), &doNothingScorer);
StrategySelector goalieSelector(goalieStrategies,sizeof(goalieStrategies)/sizeof(Strategy*), &doNothingGoalie);

uint8_t suggestedAllyStrategies[2];
Strategy *currentStrategy = &doNothingDefense;

uint8_t getOurSuggestedStrategy(Ally ally){
	return suggestedAllyStrategies[static_cast<uint8_t>(ally)];
}

Strategy *getCurrentStrategy(){
	return currentStrategy;
}

Strategy *getStrategy(uint8_t strategyID){
	if(strategyID == PICK_SOMETHING || strategyID == UNKNOWN_STRATEGY){
		if(hasPuck())
			return scorerSelector.pickStrategy();
		if(hasPuck(Ally::ALLY1) || hasPuck(Ally::ALLY2))
			return offenseSelector.pickStrategy();
		if(defenseSelector.pickStrategy()->getID() != ledDefense.getID())
			while(1);
		return defenseSelector.pickStrategy();
	}
	if((strategyID & PICK_STRATEGY_MASK) == PICK_STRATEGY_MASK){
		switch(strategyID){
			case PICK_OFFENSE:
				return offenseSelector.pickStrategy();
			case PICK_DEFENSE:
				return defenseSelector.pickStrategy();
			case PICK_SCORER:
				return scorerSelector.pickStrategy();
			case PICK_GOALIE:
				return goalieSelector.pickStrategy();
		}
	}
	switch(static_cast<StrategyType>(strategyID & 0b11000000)){
		case StrategyType::OFFENSE:
			for(uint8_t i = 0; i < sizeof(offenseStrategies)/sizeof(Strategy*); i++)
				if(offenseStrategies[i]->getID() == strategyID)
					return offenseStrategies[i];
			return &doNothingOffense;
			
		case StrategyType::DEFENSE:
			for(uint8_t i = 0; i < sizeof(defenseStrategies)/sizeof(Strategy*); i++)
				if(defenseStrategies[i]->getID() == strategyID)
					return defenseStrategies[i];
			return &doNothingDefense;
			
		case StrategyType::SCORER:
			for(uint8_t i = 0; i < sizeof(scorerStrategies)/sizeof(Strategy*); i++)
				if(scorerStrategies[i]->getID() == strategyID)
					return scorerStrategies[i];
			return &doNothingScorer;
			
		case StrategyType::GOALIE:
			for(uint8_t i = 0; i < sizeof(goalieStrategies)/sizeof(Strategy*); i++)
				if(goalieStrategies[i]->getID() == strategyID)
					return goalieStrategies[i];
			return &doNothingScorer;
	}
	return &doNothingDefense;
}

Strategy *StrategySelector::pickStrategy(){
	uint8_t priorities[strategyCount];
	uint8_t max = 0;
	for(uint8_t i=0; i<strategyCount; i++){
		priorities[i] = strategies[i]->getPriority();
		max = MAX(max,priorities[i]);
	}
	uint8_t cutoff = max >> 2;
	uint16_t total = 0;
	for(uint8_t i=0;i<strategyCount;i++){
		if(priorities[i] < cutoff)
			priorities[i]=0;
		total += priorities[i];
	}
	if(total == 0)
		return defaultStrategy;
	
	uint16_t totalMultiple = total;
	uint8_t shiftCount = 0;
	while(totalMultiple<RAND_MAX){
		shiftCount++;
		totalMultiple <<= 1;
	}
	totalMultiple >>= 1;
	shiftCount--;
	
	int16_t r;
	do{
		r = rand();
	} while((uint16_t)r >= totalMultiple);
	r >>= shiftCount;
	
	for(uint8_t i=0;i<strategyCount;i++){
		if(priorities[i]!=0){
			r -= priorities[i];			
			if(r<=0)
				return strategies[i];
		}
	}
	return defaultStrategy;
}

bool isOffense(uint8_t strategyID){
	return (strategyID & 0b11000000) == static_cast<uint8_t>(StrategyType::OFFENSE);
}

bool isDefense(uint8_t strategyID){
	return (strategyID & 0b11000000) == static_cast<uint8_t>(StrategyType::DEFENSE);
}

bool isScorer(uint8_t strategyID){
	return (strategyID & 0b11000000) == static_cast<uint8_t>(StrategyType::SCORER);
}

bool isGoalie(uint8_t strategyID){
	return (strategyID & 0b11000000) == static_cast<uint8_t>(StrategyType::GOALIE);
}

void updateStrategies(){
	suggestedAllyStrategies[0] = UNKNOWN_STRATEGY;
	suggestedAllyStrategies[1] = UNKNOWN_STRATEGY;
	Ally highestPriorityAlly = getHighestPriorityAlly();
	Strategy *newStrategy = currentStrategy;
	if(allyHigherPriorityThanMe(highestPriorityAlly)){
		m_red(1);
		uint8_t suggestion = getAllySuggestedStrategy(highestPriorityAlly);
		if(currentStrategy->getID() != suggestion && suggestion != PICK_SOMETHING && suggestion != UNKNOWN_STRATEGY){
			newStrategy = getStrategy(suggestion);
		}
		else{
			uint8_t newID = currentStrategy->run(suggestedAllyStrategies);
			if(newID != currentStrategy->getID())
				newStrategy = getStrategy(newID);
		}
	}
	else{
		m_red(0);
		uint8_t newID = currentStrategy->run(suggestedAllyStrategies);
		if(newID != currentStrategy->getID())
			newStrategy = getStrategy(newID);
	}
	
	if(newStrategy != currentStrategy){
		currentStrategy = newStrategy;
		currentStrategy->prepare();
		currentStrategy->run(suggestedAllyStrategies);
	}
}*/