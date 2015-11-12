class ADC{
public:	
	void update();
private:
	uint8_t resistor;
	uint16_t irValues[16];
	uint16_t battery;
	uint16_t boost;
	uint16_t leftMotor;
	uint16_t rightMotor;
};

ADC::update(){
	//start ADC
}

