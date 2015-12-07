#pragma once

class Sweep : public Strategy{
	public:
	Sweep(uint8_t id) : Strategy(StrategyType::OFFENSE, id){
		
	}

	uint8_t run(uint8_t *strategyIDs) override{
		setMotors(0,0);
		return PICK_SOMETHING;
	}
	
	uint8_t getPriority(){
		return 0;
	}
	
};

//Scoring
//Bluff Kick -> Drive to one side, kick to other. Need one for both sides
//Goal Corner -> Drive straight to the corner, one for each side
//Goal Middle -> Drive straight to the middle of the goal
//Drive along wall -> Drive with puck along the wall to the goal


//Assist
//Sweep -> push goalies out of the way. Once for each side. Don't go too fast?
//Block -> Drive in front of scorer
//Push -> go behind scorer and push them

//Defense
//Charge -> Go to puck

//Goalie
//Goalie

//Miscellaneous
//5 minute epoxy kicker
//15 cm puck axis to goal
//+= 25cm - puck radius goal corners