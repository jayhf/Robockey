#include "time.h"
#include "Digital.h"
#include "avr/io.h"
#include "avr/interrupt.h"

time currentTime;

void initClock(){
	//Count to 62500
	OCR3A = 62500;
	TCCR3B |= 1 << WGM32;
	//Start the timer
	TCCR3B |= 1 << CS30;
}

time getTime(){
	return currentTime;
}

bool timePassed(uint16_t t){
	return (int16_t)(currentTime - t) >= 0;
}

void sleep(){
	cli();
	startKick(0);
	updateKick();
	setLED(LEDColor::OFF);
	setEnabled(false);
	//Set sleep mode to power down
	SMCR |= 1 << SM1;
	//Enable sleep
	SMCR |= 1 << SE;
}

ISR(TIMER3_OVF_vect){
	currentTime++;
}
