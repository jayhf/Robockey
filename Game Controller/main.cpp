/*
 * Game Controller.cpp
 *
 * Created: 11/12/2015 12:52:39 PM
 * Author : Jay
 */ 

#define F_CPU 16000000
#define CONTROLLER_ADDR 84
#define R1_ADDR 85
#define R2_ADDR 86
#define R3_ADDR 87

#include <util/delay.h>
extern "C"{
	#include "m_general.h"
	#include "m_rf.h"
	#include "m_usb.h"
};

void sendAllByte(uint8_t value);
void goal(uint8_t teamID, uint8_t redScore, uint8_t blueScore);

#define commTest() sendAllByte(0xA0)
#define play() sendAllByte(0xA1)
#define goalR()	sendAllByte(0xA2)
#define goalB()	sendAllByte(0xA3)
#define pause() sendAllByte(0xA4)
#define halftime() sendAllByte(0xA6)
#define gameover() sendAllByte(0xA7)
#define enemyPositions(packet) sendAll(packet)

volatile uint8_t newPacket = 0;
int main(void)
{
    m_rf_open(1,CONTROLLER_ADDR, 10);
	m_usb_init();
	
	m_green(ON);
	
	uint8_t usbIndex = 0;
	uint8_t usbBuffer[100];
	while (1){
		if(m_usb_rx_available()){
			usbBuffer[usbIndex++] = m_usb_rx_char();
			if(usbIndex>=100)
				usbIndex=0;
		}
		else if(newPacket){
			newPacket = 0;
			uint8_t rfBuffer[10];
			m_rf_read((char*)rfBuffer,10);
			m_usb_tx_char(0xFF);
			m_usb_tx_char(0x00);
			for(int i=0;i<10;i++)
				m_usb_tx_char(rfBuffer[i]);
			m_usb_tx_push();
		}
	}
}

void sendAll(uint8_t *packet){
	for(int i=0;i<3;i++)
		m_rf_send(R1_ADDR+i, (char*)packet, 10);
}
void sendAllByte(uint8_t value){
	uint8_t packet[10] = {value, value, value, value, value, value, value, value, value, value};
	sendAll(packet);
}

void goal(uint8_t teamID, uint8_t redScore, uint8_t blueScore){
	uint8_t packet[10] = {teamID, redScore, blueScore};
	sendAll(packet);
}

ISR(INT2_vect){
	m_red(TOGGLE);
	newPacket = 1;
}