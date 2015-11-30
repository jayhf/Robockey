#include "Digital.h"
#include "time.h"
#include "avr/io.h"
#include "stdlib.h"
#include "m_general.h"
int16_t previousLeft = 0;
int16_t previousRight = 0;

///You don't properly handle negative speeds. Floating point math should be avoided.
///At the very least never divide by a constant, multiply by 1/constant.
void movement(int rightSpeed, int leftSpeed){
	if (leftSpeed > 100) {
		leftSpeed = 100;
	}
	if (rightSpeed > 100) {
		rightSpeed = 100;
	}
	if (leftSpeed < -100){
		leftSpeed = -100;
	}
	if (rightSpeed < -100){
		rightSpeed = -100;
	}
	if (leftSpeed >= 0) {
		if(!check(PINC,6)){
			set(PINC,6);
		}
		OCR1B = leftSpeed *0.01 * (OCR1A-0);
	}
	else if (leftSpeed < 0) {
		if(check(PINC,6)){
			clear(PINC,6);
		}
		OCR1B = -leftSpeed *0.01 * (OCR1A-0);
	}
	if (rightSpeed >= 0) {
		if(!check(PINC,7)){
			set(PINC,7);
		}
		OCR1C = rightSpeed *0.01 * (OCR1A-0);
	}
	else if (rightSpeed < 0) {
		if(check(PINC,7)){
			clear(PINC,7);
		}
		OCR1C = -rightSpeed *0.01 * (OCR1A-0);
	}
}

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
	
	OCR1B = abs(0.05*right + 0.95*previousRight)+(right==0?0:400);
	OCR1C = abs(0.05*left + 0.95*previousLeft)+(left==0?0:400);
	PORTC = ((PORTC & ~(0b11 << 6))) | ((bool)(right>0) << 6) | ((bool)(left>0) << 7);
	previousLeft = left;
	previousRight = right;
}

time kickEndTime;
void startKick(uint16_t duration){
	kickEndTime = getTime() + duration;
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
