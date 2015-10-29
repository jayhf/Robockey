/*
 * Acrobot.cpp
 *
 * Created: 10/27/2015 9:33:04 PM
 * Author : Jay
 */ 

#define F_CPU 16000000

#include "math.h"
extern "C"{
#include "m_general.h"
#include "m_imu.h"
#include "m_usb.h"
#include "m_bus.h"
}

int requestedCurrent = 0; //Measured in mA

void initADC();
void initTimer1();
void calculateTheta();
void calculateDX();

float calculateCurrent(float o, float w, float x, float dx);
int motorCurrent = 0;
float integralTerm = 0;
int oldDelta = 0;
int imuBuffer[9];
float theta = 0;
float x = 0;
float dx = 0;
float dxPrev = 0;
float ax = 0;
float wheelDiameter = 3.1; //inches
float timeStep = 1/1000.0; //seconds

int main(void)
{
	m_clockdivide(0);
	
	initADC();
	initTimer1();
	
	//initialize motor output pins
	for (int i = 0; i<4; i++){
		set(DDRB,i);
	}
	
	m_imu_init(1,1);
	
	m_usb_init();
	
	sei();
	
	m_green(ON);
	/* Replace with your application code */
	while (1)  {
		if(m_imu_raw(imuBuffer));
		
		float o = atan2(imuBuffer[2],imuBuffer[0]);
		float w = imuBuffer[4]*0.03051757812-3;
		m_usb_tx_int(motorCurrent);
		m_usb_tx_char(',');
		
		m_usb_tx_int(requestedCurrent);
		m_usb_tx_char(',');
		
		m_usb_tx_int(ADC);
		m_usb_tx_char(',');
		//m_usb_tx_int((int)(o*180/3.14159));
		//m_usb_tx_char(',');
		//m_usb_tx_int((int)(w));
		//m_usb_tx_char(',');
		m_usb_tx_int((int)(dx));
		m_usb_tx_char(',');
		m_usb_tx_char('\n');
		requestedCurrent = 1000;//(int)calculateCurrent(theta,imuBuffer[4],x,dx);
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
	
	//Initialize the ADC to read from F0
	set(DIDR0, ADC0D);
	
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
	
	//Enable PWM on pins B6 and B7
	set(DDRB,6);
	set(DDRB,7);
	
	//toggle modes
	set(TCCR1A,COM1B1);
	clear(TCCR1A,COM1B0);
	
	set(TCCR1A,COM1C1);
	clear(TCCR1A,COM1C0);
	
	//Enable with divider 1
	set(TCCR1B,CS10);
	clear(TCCR1B,CS11);
	clear(TCCR1B,CS12);
	
	//Enable Interrupts
	set(TIMSK1,TOIE1);
	
	OCR1A = 16000;
	OCR1B = OCR1C = 1;
}

void calculateDX(){
	float rpm = (motorCurrent - 150) * 130/2250.0;
	if(check(PINB,0)){
		dx = rpm * M_PI * wheelDiameter; //inches / minute
	}
	else if (check(PINB,1)){
		dx = -rpm * M_PI * wheelDiameter; //inches / minute
	}
	
}

void calculateAX(){
	ax = (dx - dxPrev) / timeStep;
}

//PID Current Controller running at 1khz
ISR(TIMER1_OVF_vect,ISR_NOBLOCK){
	motorCurrent = (ADC * 525) / 1000; //525 mV/A
	int delta = motorCurrent - requestedCurrent;
	float cp = 1;
	float ci = 0;//1;
	float cd = 0;//1;
	float p = delta;
	integralTerm += delta;
	float d = (delta - oldDelta) / timeStep;
	if (requestedCurrent >= 0){ //spin clockwise?
		set(PORTB,0);
		clear(PORTB,1);
		clear(PORTB,2);
		set(PORTB,3);
	}
	else{
		clear(PORTB,0);
		set(PORTB,1);
		set(PORTB,2);
		clear(PORTB,3);
	}
	OCR1C = OCR1B = 15999;//OCR1B + (cp*p + ci * integralTerm + cd*d);
	//Check for direction based on pin settings or keep track
	//Add sign to measured current based on this
	//use PID to calculate new OCR1B/C based on difference from old and current difference
	//OCR1B = OCR1C = X
	//Reverse direction if needed
	oldDelta = delta;
}