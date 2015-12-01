#include "ADC.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000
#include <util/delay.h>
#include "miscellaneous.h"

Resistor irResistor;
Resistor nextResistor;
uint8_t irIndex;
uint16_t irBuffer1[16];
uint16_t irBuffer2[16];
uint16_t *irReadBuffer = irBuffer1;
uint16_t *irWriteBuffer = irBuffer2;
uint16_t battery;
uint16_t boost;
uint16_t leftMotor;
uint16_t rightMotor;
uint16_t switchValue;

void initADC(){
	//Set the ADC clock prescaler
	ADCSRA |= 0b111 << ADPS0;
	//Set the analog reference
	ADMUX |= 0b01 << REFS0;
	//Disable the digital inputs on the analog pins
	DIDR0 = 0b10110011;
	//Disable pull up resistors
	PORTF &= 0b00001100;
	//Enable the mux pins
	DDRB |= 0b111111;
	//Enable ADC interrupts
	ADCSRA |= 1 << ADIE;
	//Enable the ADC
	ADCSRA |= 1 << ADEN;
	ADMUX |= 0b111 << MUX0;
	_delay_ms(120);
}

void updateResistor(){
	//Change the resistor if necessary
	uint16_t maxValue = 0;
	for(uint8_t i=0;i<16;i++)
		maxValue = maxValue > irReadBuffer[i] ? maxValue : irReadBuffer[i];
	if(maxValue>900)
		nextResistor--;
	else if(maxValue<175)
		nextResistor++;
	PORTB = (PORTB & (~0b11 << 4)) | (static_cast<uint8_t>(nextResistor) << 4);
}

ISR(ADC_vect){
	static uint8_t selectedIR = 0;
	uint8_t currentIndex = (ADMUX >> MUX0) & 0b111;
	ADMUX &= ~(0b111 << MUX0);
	switch(currentIndex){
		case 0:
			irWriteBuffer[selectedIR] = ADC;
			selectedIR = (selectedIR+1) & 0b1111;
			if(selectedIR != 0)
				PORTB = (PORTB & (~0b1111)) | selectedIR;
			else{
				uint16_t *temp = irWriteBuffer;
				irWriteBuffer = irReadBuffer;
				irReadBuffer = temp;
				irResistor = nextResistor;
				ADMUX |= 7 << MUX0;
				updateResistor();
			}
			break;
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
			ADMUX |= 7 << MUX0;
			break;
	}
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

SwitchPosition switchPosition(){
	if(switchValue>768)
			return SwitchPosition::DOWN;
		else if(switchValue>256)
			return SwitchPosition::MIDDLE;
		else
			return SwitchPosition::UP;
};

Resistor getSelectedResistor(){
	return irResistor;
};

uint16_t* getIRData(){
	return irReadBuffer;
}

Resistor& operator++(Resistor &r){
	if(r==Resistor::R330K)
		return r;
	return r = static_cast<Resistor>(static_cast<uint8_t>(r)+1);
}
Resistor operator++(Resistor &r, int){
	Resistor result = r;
	++r;
	return result;
}
Resistor& operator--(Resistor &r){
	if(r==Resistor::R1K)
		return r;
	return r = static_cast<Resistor>(static_cast<uint8_t>(r)-1);
}
Resistor operator--(Resistor &r, int){
	Resistor result = r;
	--r;
	return result;
}

uint16_t getBattery(){
	return battery;
}