#include "Strategies.h"

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

class EmptyStrategy : public Strategy{
public:
	EmptyStrategy(uint8_t id) : Strategy(StrategyType::OFFENSE, id){
		
	}

	void run() override{}
};


EmptyStrategy emptyStrategy = EmptyStrategy(0);
Strategy *strategies[1] = {&emptyStrategy};

Strategy *getStrategy(uint8_t strategyID){
	switch(static_cast<StrategyType>(strategyID & 0b11000000)){
		case StrategyType::DEFENSE:
			break;
		case StrategyType::GOALIE:
			break;
		case StrategyType::OFFENSE:
			break;
		case StrategyType::SCORER:
			break;
	}
	return &emptyStrategy;
}
