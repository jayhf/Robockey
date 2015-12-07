

class DoNothingStrategy : public Strategy{
	public:
	DoNothingStrategy(uint8_t id) : Strategy(StrategyType::OFFENSE, id){
		
	}

	uint8_t run(uint8_t *strategyIDs) override{
		setMotors(0,0);
		return PICK_SOMETHING;
	}
	
	uint8_t getPriority(){
		return 0;
	}
	
};