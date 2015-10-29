/*
 * Acrobot.cpp
 *
 * Created: 10/27/2015 9:33:04 PM
 * Author : Jay
 */ 

#define F_CPU 16000000

#include "math.h"

extern "C" {
	#include "m_general.h"
	#include "m_imu.h"
};

int requestedCurrent = 0; //Measured in mA

void initADC();
void initTimer1();

float calculateCurrent(float o, float w, float x, float dx);
int motorCurrent = 0;
int imuBuffer[9];

int main(void)
{
	m_clockdivide(0);
	
	initADC();
	initTimer1();
	
	m_red(!m_imu_init(1,1));
	
	sei();
	
	m_green(ON);
    /* Replace with your application code */
    while (1)  {
		if(m_imu_raw(imuBuffer)){
			m_red(OFF);
			requestedCurrent = (int)calculateCurrent(0,0,0,0);
		}
		else
			m_red(ON);
    }
}

//Calculates the desired current in mA. Negative means reverse.
float calculateCurrent(float o, float w, float x, float dx){
	//Tune these parameters to optimize performance
	float co = 1;
	float cw = 1;
	float cdx = 1;
	float cx = 1;
	return co * sin(o) + cw * w + cx * x + cdx * dx;
}

//Initialize the ADC to read from F0 continuously.
//If necessary, we can control/measure the motors individually later.
void initADC(){
	//set reference to VCC
	clear(ADMUX, REFS1);
	set(ADMUX, REFS0);
	
	
	//Set ADC clock to 1/128 of system clock (125kHz)
	set(ADCSRA,ADPS2);
	set(ADCSRA,ADPS1);
	set(ADCSRA,ADPS0);
	
	//Initialize the ADC to read from F6
	set(DIDR0, ADC6D);
	
	//Continuously read
	set(ADCSRA,ADATE);
	
	//Set analog pin to F0
	clear(ADCSRB,MUX5);
	clear(ADMUX,MUX2);
	clear(ADMUX,MUX1);
	clear(ADMUX,MUX0);
	
	//Enable the ADC
	set(ADCSRA,ADEN);
	
	//Start the ADC
	set(ADCSRA,ADSC);
}

//Initialize timer 1 for PWM
void initTimer1(){
	//Count to OCR1A in PWM mode
	set(TCCR1B,WGM13);
	set(TCCR1B,WGM12);
	set(TCCR1A,WGM11);
	set(TCCR1A,WGM10);
	
	//Enable PWM on pins B and C
	set(TCCR1A,COM1B1);
	set(TCCR1A,COM1B0);
	set(TCCR1A,COM1C1);
	set(TCCR1A,COM1C0);
	
	//Enable Interrupts
	set(TIMSK1,TOIE1);
}

//PID Current Controller running at 1khz
ISR(TIMER1_OVF_vect,ISR_NOBLOCK){
	static int oldCurrent = 0;
	float cp = 1;
	float ci = 1;
	float cd = 1;
	//Check for direction based on pin settings or keep track
	//Add sign to measured current based on this
	//use PID to calculate new OCR1B/C based on difference from old and current difference
	//OCR1B = OCR1C = X
	//Reverse direction if needed
	oldCurrent = ADC;
}
