#include "Strategies.h"
#include "Digital.h"

Strategy::Strategy(StrategyType strategyType, uint8_t strategyId) : 
	strategyType(strategyType),
	id(strategyId | static_cast<uint8_t>(strategyType)){
}

StrategyType Strategy::getStrategyType(){
	return strategyType;
}

uint8_t Strategy::getSuccessCount(){
	return successCount;
}
uint8_t Strategy::getFailCount(){
	return failCount;
}

void Strategy::strategyFailed(){
	failCount++;
}
void Strategy::strategyWorked(){
	successCount++;
}

uint8_t Strategy::getID(){
	return id;
}

class DoNothingStrategy : public Strategy{
public:
	DoNothingStrategy(StrategyType strategyType, uint8_t id) : Strategy(strategyType, id){
		
	}

	void run() override{
		setMotors(0,0);
	}
		
	bool possible() override{
		return true;
	}
};


DoNothingStrategy doNothingOffense = DoNothingStrategy(StrategyType::OFFENSE, 0);
DoNothingStrategy doNothingDefense = DoNothingStrategy(StrategyType::DEFENSE, 0);
DoNothingStrategy doNothingScorer = DoNothingStrategy(StrategyType::SCORER, 0);
DoNothingStrategy doNothingGoalie = DoNothingStrategy(StrategyType::GOALIE, 0);


Strategy *offenseStrategies[] = {&doNothingOffense};
Strategy *defenseStrategies[] = {&doNothingDefense};
Strategy *scorerStrategies[] = {&doNothingScorer};
Strategy *goalieStrategies[] = {&doNothingGoalie};

Strategy *getStrategy(uint8_t strategyID){
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

void StrategySelector::previousSucceeded(){
	
}
void StrategySelector::previousFailed(){
	
}
Strategy *StrategySelector::pickStrategy(StrategyType strategyType){
	
}