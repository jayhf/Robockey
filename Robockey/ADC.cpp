#include "ADC.h"

bool updateCompleted;

uint8_t irResistor;
uint8_t irIndex;
uint16_t irValues[16];
uint16_t battery;
uint16_t boost;
uint16_t leftMotor;
uint16_t rightMotor;
uint16_t switchValue;

adcUpdateCompleted(){
	return updateCompleted;
}

initADC(){
	//Set the ADC clock prescaler
	ADCSRA |= 0b111 << ADPS0;
	//Disable the digital inputs on the analog pins
	DIDR0 = 0b10110011;
	//Enable the mux pins
	DDRB |= 0b111111;
	//Enable ADC interrupts
	ADCSRA |= 1 << ADIE;
	
	beginADC();
	while(!updateCompleted);
}

beginADC(){
	updateCompleted = 0;
	ADMUX |= 0b111 << MUX0;
	//Start the conversion
	ADCSRA |= 1 << ADSC;
	//Change the resistor if necessary
	uint16_t maxValue = 0;
	for(uint8_t i=0;i<16;i++)
		maxValue = maxValue > irValues[i] ? maxValue : irValues[i];
	if(maxValue>800 && irResistor != R1K)
		irResistor--;
	else if(maxValue<200 && irResistor != R330K)
		irResistor++;
	PORTB = (PORTB & (~0b11 << 4)) | (irResistor << 4);
}

ISR(ADC_vect){
	static uint8_t selectedIR = 0;
	uint8_t currentIndex = (ADMUX >> MUX0) & 0b111;
	ADMUX &= ~(0b111 << MUX0);
	switch(currentIndex){
		case 0:
			irValues[selectedIR++] = ADC;
			if(selectedIR != 0){
				PORTB = (PORTB & (~0b1111)) | selectedIR;
				break;
			}
			else goto default;
		case 1:
			switchValue = ADC;
			ADMUX |= 0 << MUX0;
			break;
		case 4:
			rightMotor = ADC;
			ADMUX |= 1 << MUX0;
			break;
		case 5:
			leftMotor = ADC;
			ADMUX |= 4 << MUX0;
			break;
		case 6:
			battery = ADC;
			ADMUX |= 5 << MUX0;
			break;
		case 7:
			boost = ADC;
			ADMUX |= 6 << MUX0;
			break;
		default:
			updateCompleted = true;
			ADMUX |= 0b111 << MUX0;
			return;
	}
	//Start next conversion
	ADCSRA |= 1 << ADSC;
}

bool adcUpdateCompleted(){
	return updateCompleted;
}

bool batteryLow(){
	return battery < 450;
}

bool boostReady(){
	return boost > 800;
}

uint16_t leftMotorCurrent(){
	return (leftMotor * 147) >> 3;
}

uint16_t rightMotorCurrent(){
	return (rightMotor * 147) >> 3;
}

enum SwitchPosition switchPosition(){
	if(switchPosition>768)
		return DOWN;
	else if(switchPosition>256)
		return MIDDLE;
	else
		return UP;
};

enum resistor getSelectedResistor(){
	return resistor;
};

uint16_t* getIRData(){
	return irValues;
}