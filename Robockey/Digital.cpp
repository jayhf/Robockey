#include "Digital.h"
#include "time.h"

void initDigital(){
	DDRB |= 0b11 << 6;
	DDRC |= 0b11 << 6;
	DDRD |= 0b11 << 4;
	DDRD |= 1 << 7;
	DDRE |= 1 << 6;
	
	//Set B6,7 to PWM mode (set on rollover)
	TCCR1A |= 1 << COM1B1;
	TCCR1A |= 1 << COM1C1;
	OCR1A = 1600;
	//Set timer 1 to mode 15 (OCR1A PWM mode)
	TCCR1A |= 0b11 << WGM10;
	TCCR1B |= 0b11 << WGM12;
	//Enable timer 1 and set the clock divider to 1
	TCCR1B |= 1 << CS10;
	
	setEnabled(true);
}

void setEnabled(bool enabled){
	if(enabled)
		PORTE |= 1 << 6;
	else
		PORTE &= ~(1 << 6);
}

void setMotors(int8_t right, int8_t left){
	TCCR1B = abs(right);
	TCCR1C = abs(left);
}

uint16_t kickEndTime;
void startKick(uint16_t duration){
	kickEndTime = getTime() + duration;
	PORTD |= 1 << 7;
}

void updateKick(){
	if(((int16_t)(getTime() - kickEndTime)) >= 0)
		PORTD &= ~(1 << 7);
}

void setLED(enum LEDColor color){
	PORTD = (PORTD & (~0b11 << 4)) | color << 4;
}

bool switchesPressed(){
	return PIND & (1 << 6);
}