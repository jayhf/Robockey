#include "time.h"
#include "Digital.h"

uint16_t time;

void initClock(){
	//Count to 16000
	OCR3A = 16000;
	TCCR3B |= << WGM32;
	//Start the timer
	TCCR3B |= 1 << CS30;
}

uint16_t getTime(){
	return time;
}

void sleep(){
	cli();
	startKick(0);
	updateKick();
	setLED(OFF);
	setEnabled(false);
	//Set sleep mode to power down
	SMCR |= 1 << SM1;
	//Enable sleep
	SMCR |= 1 << SE;
}

ISR(TIMER3_OVF_vect){
	time++;
}
