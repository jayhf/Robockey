#pragma once

#include "GameState.h"

class Strategy{
public:
	Strategy(StrategyType type, uint8_t id);
	StrategyType getStrategyType();
	uint8_t getID();
	uint8_t getSuccessCount();
	uint8_t getFailCount();
	void strategyFailed();
	void strategyWorked();
	virtual void run() = 0;
	
protected:
	StrategyType strategyType;
	uint8_t id;
	uint8_t successCount;
	uint8_t failCount;
};

Strategy *getStrategy(uint8_t strategyID);

class StrategySelector{
public:
	StrategySelector(Strategy **strategies, uint8_t strategyCount) : strategies(strategies), strategyCount(strategyCount){}
	void previousSucceeded();
	void previousFailed();
	Strategy *pickStrategy();
private:
	Strategy **strategies;
	uint8_t strategyCount;
	Strategy *previousStrategy;
};