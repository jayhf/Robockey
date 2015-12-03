#include "Digital.h"
#include "time.h"
#include "avr/io.h"
#include "stdlib.h"
#include "miscellaneous.h"

int16_t previousLeft = 0;
int16_t previousRight = 0;

void initDigital(){
	DDRB |= 0b11 << 6;
	DDRC |= 0b11 << 6;
	DDRD |= 0b11 << 4;
	DDRD |= 1 << 7;
	DDRE |= 1 << 6;
	PORTD |= 1<<7;
	
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

void setMotors(int16_t right, int16_t left){
	
	OCR1B = abs(right)+(right==0?0:400);
	OCR1C = abs(left)+(left==0?0:400);
	PORTC = ((PORTC & ~(0b11 << 6))) | ((bool)(right>0) << 6) | ((bool)(left>0) << 7);
	//previousLeft = left;
	//previousRight = right;
}


time kickEndTime;
time kickResetTime;
bool canKick;
void startKick(){
	kickEndTime = getTime() + 50;
	PORTD &= ~(1 << 7);
}

void updateKick(){
	if(((int16_t)(getTime() - kickEndTime)) >= 0){
		PORTD |= 1 << 7;
		kickEndTime = getTime();
	}
}

void setLED(enum LEDColor color){
	PORTD = (PORTD & (~0b11 << 4)) | static_cast<uint8_t>(color) << 4;
}

bool switchesPressed(){
	return PIND & (1 << 6);
}
