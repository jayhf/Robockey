/*
 * Acrobot.cpp
 *
 * Created: 10/27/2015 9:33:04 PM
 * Author : Jay
 */ 

#define F_CPU 16000000

#include "math.h"
#include "avr/builtins.h"
#include "string.h"
#include <stdlib.h>

extern "C"{
	#include "m_general.h"
	#include "m_bus.h"
	#include "m_imu.h"
	//#include "m_rf.h"
	//#include "m_usb.h"
}


int requestedCurrent = 0; //Measured in mA

void initADC();
void initTimer1();
void calculateTheta();
void calculateDX();
void approximateDX();
void approximateTheta();

int calculateCurrent(float o, float w, float x, float dx);
int motorCurrent = 0;
float integralTerm = 0;
int oldDelta = 0;
int imuBuffer[9];
//float theta = 0;
//float x = 0;
float dx = 0;
float ax = 0;
float wheelDiameter = 3.1; //inches
float timeStep = 1/1000.0; //seconds
volatile float w = 0;
volatile int accelXLowPass = 0;
volatile int wLowPass = 0;
volatile long wIntegral = 0;
volatile long oIntegral = 0;

struct sendData{
	 uint8_t id;
	 int16_t ocr1b;
	 int16_t theta;
	 int16_t omega;
	 int16_t axLowPass;
	 int16_t omegaLowPass;
	 char padding[15];
};
struct sendData sendBuffer;
char receiveBuffer[32];
volatile bool newMessage = false;


int frequency = 16000;

int main(void){
	m_clockdivide(0);
	
	initADC();
	initTimer1();
	//m_usb_init();
	//initialize motor output pins
	for (int i = 0; i<4; i++){
		set(DDRB,i);
	}
	
	m_imu_init(1,1);
	
	//m_usb_init();
	//m_rf_open(12,52,32);
	sei();
	
	m_green(ON);
	
	//int x = 0;
	while (1)  {
		m_imu_raw(imuBuffer);
		//approximateDX();
		//float o = atan2(imuBuffer[2],imuBuffer[0]);
		
		//m_usb_tx_int(imuBuffer[2]);
		//m_usb_tx_char(',');
		
		//m_usb_tx_int(x);
		//m_usb_tx_char(',');
		//m_usb_tx_int((int)dx);
		//m_usb_tx_char(',');
		//m_usb_tx_int((int)(o*180/3.14159));
		//m_usb_tx_char(',');
		//m_usb_tx_int((int)(theta*1000));
		//m_usb_tx_char(',');
		//m_usb_tx_int((int)(w));
		//m_usb_tx_char(',');
		//m_usb_tx_int(requestedCurrent);
		//m_usb_tx_char('\n');
		//int i=0;
		//int q=1000;
		/*sendBuffer[i++]=0;
		memcpy(&sendBuffer[i+=4],&ocr1b,4);
		memcpy(&sendBuffer[i+=4],&co,4);//theta;
		memcpy(&sendBuffer[i+=4],&w,4);
		memcpy(&sendBuffer[i+=4],&q,4);//accelXLowPass;
		memcpy(&sendBuffer[i+=4],&wLowPass,4);*/
		/*if((x++ % 100)==0){
			m_red(TOGGLE);
			sendBuffer.id = 0;
			sendBuffer.ocr1b = OCR1B;
			sendBuffer.theta = 0;//accelXLowPass;
			sendBuffer.omega = w;
			sendBuffer.axLowPass = accelXLowPass;
			sendBuffer.omegaLowPass = wLowPass;
			m_rf_send(86,(char*)&sendBuffer,32);
		}*/
		/*if(newMessage){
			m_rf_read(receiveBuffer,32);
			newMessage = false;
			m_green(TOGGLE);
			char variable = receiveBuffer[0];
			float number = 0;
			float decimalProduct = 1;
			for(int i=1;((receiveBuffer[i]=='.')||(receiveBuffer[i]>='0'&&receiveBuffer[i]<='9'))&&i<32;i++){
				if(receiveBuffer[i] == '.')
					decimalProduct = .1;
				else{
					number = number*10+receiveBuffer[i]-'0';
					if(decimalProduct<1)
						decimalProduct*=.1;
				}
			}
			//m_usb_tx_long((long)number);
			if(decimalProduct<1)
				decimalProduct *= 10;
			number *= decimalProduct;
			//m_usb_tx_char('\t');
			//m_usb_tx_long((long)number);
			//m_usb_tx_char('\n');
			switch(variable){
				case 'o':
					co = number;
					break;
				case 'w':
					cw = number;
					break;
				case 'x':
					cx = number;
					break;
				case 'v':
					cv = number;
					break;
			}
		}*/
		w = wLowPass*0.03051757812;
		
		accelXLowPass = (imuBuffer[2] + 40 + 0*wLowPass)*.02+accelXLowPass*(.98);
		wLowPass = (imuBuffer[4]-3)*.05+wLowPass*(.95);
		wIntegral=((wIntegral*15)>>4) + 1*wLowPass;
		oIntegral=((oIntegral*15)>>4) + accelXLowPass;
		
		requestedCurrent = calculateCurrent(accelXLowPass,w,wIntegral,oIntegral);
	}
}
float f(float x){
	return copysign(x,pow(copysign(1.0,x),2));
}
//float ce = 1/16000.0;
//Calculates the desired current in mA. Negative means reverse.
int calculateCurrent(float o, float w, float x, float dx){
	volatile float co = 22;
	volatile float cw = 3;
	volatile float cv = .1;
	volatile float cx = 1;
	//Tune these parameters to optimize performance
	//if(w<5&&w>-5)
	//	w=0;
	//if(o<100&&o>-100)
	//	o=0;
	static float ocrpass =0;
	ocrpass = OCR1B*0.01 + ocrpass*0.99;
	float result =  co * o + cw * w + cx * x + cv * dx+1*ocrpass;
	if(result > frequency)
		return frequency;
	else if(result<-frequency)
		return -frequency;
	return (int) result;
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
	
	OCR1A = frequency;
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
/*
void calculateAX(){
	ax = (dx - dxPrev) / timeStep;
}


void approximateDX(){
	dx = imuBuffer[2] * timeStep + dxPrev;
}*/

//PID Current Controller running at 1khz
ISR(TIMER1_OVF_vect){
	

	//theta = atan2(accelXLowPass,imuBuffer[0])+.43;//+.135
	
	//motorCurrent = (ADC * 525) / 1000; //525 mV/A
	//int delta = imuBuffer[4];
	//float cp = 150;
	//float ci = 10;//1;
	//float cd = 30;//1;
	//float p = delta;
	//integralTerm += delta;
	//float d = (delta - oldDelta) / timeStep;*/
	
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
	OCR1B = (15*(long)OCR1B+abs(requestedCurrent))>>4;
	OCR1C = 0.89*(float)OCR1B;
	if(OCR1B>15250){
		OCR1B = OCR1C = 15999;
	}
	//OCR1B + (cp*p + ci * integralTerm + cd*d);
	//Check for direction based on pin settings or keep track
	//Add sign to measured current based on this
	//use PID to calculate new OCR1B/C based on difference from old and current difference
	//OCR1B = OCR1C = X
	//Reverse direction if needed
	//oldDelta = delta;
}

ISR(INT2_vect){
	newMessage = true;
}