#include "i2c.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

//comment this to create a slave
//uncomment this to create a master
//#define MASTER

#ifndef MASTER

struct Message msgToSend;
struct Message msgToReceive;

ISR(INT0_vect)       
{   	
   	cli();
  	switch(checkRequest())
  	{
  		case REQUEST_TO_WRITE:
  			sendAsSlave(msgToSend);
  			break;

  		case REQUEST_TO_READ:

  			receiveAsSlave(&msgToReceive);
  			break;
  		
  		default:

  			break;
  	}    	
   	sei();
}

void initINT0()
{
	MCUCR &= ~(1<<ISC01) | ~(1<<ISC00);
	GIMSK |= (1<<INT0);

	sei();
}

int main(void)
{			
	DDRB = 0xff;
	PORTB = 0x00;

	initAsSlave(SLAVEADRESS);

	msgToSend.byte[0] = 0b10000000;
	msgToSend.byte[1] = 0b11000000;
	msgToSend.byte[2] = 0b01100000;
	msgToSend.byte[3] = 0b00110000;
	msgToSend.byte[4] = 0b00011000;
	msgToSend.byte[5] = 0b00001100;
	msgToSend.byte[6] = 0b00000110;
	msgToSend.byte[7] = 0b00000011;

	msgToReceive.byte[0] = 0b00000000;
	msgToReceive.byte[1] = 0b00000000;
	msgToReceive.byte[2] = 0b00000000;
	msgToReceive.byte[3] = 0b00000000;
	msgToReceive.byte[4] = 0b00000000;
	msgToReceive.byte[5] = 0b00000000;
	msgToReceive.byte[6] = 0b00000000;
	msgToReceive.byte[7] = 0b00000000;
	
	initINT0();

	while(1)
	{	
		cli();
		int i;
		for(i=0; i<BYTES; i++)
		{
			PORTB = msgToReceive.byte[i];
			_delay_ms(100);
		}
		sei();
	}
}

#endif

#ifdef MASTER

struct Message msgToSend;
struct Message msgToReceive;

int main(void)
{
	DDRB = 0xff;
	PORTB = 0x00;
	
	DDRD = 0xFF;
	PORTD |= (1<<PD2);

	initAsMaster(SLC_FREQUENCY);
	
	msgToSend.byte[0] = 0b00000000;
	msgToSend.byte[1] = 0b10000001;
	msgToSend.byte[2] = 0b11000011;
	msgToSend.byte[3] = 0b11100111;
	msgToSend.byte[4] = 0b11111111;
	msgToSend.byte[5] = 0b01111110;
	msgToSend.byte[6] = 0b00111100;
	msgToSend.byte[7] = 0b00011000;

	msgToReceive.byte[0] = 0b00000000;
	msgToReceive.byte[1] = 0b00000000;
	msgToReceive.byte[2] = 0b00000000;
	msgToReceive.byte[3] = 0b00000000;
	msgToReceive.byte[4] = 0b00000000;
	msgToReceive.byte[5] = 0b00000000;
	msgToReceive.byte[6] = 0b00000000;
	msgToReceive.byte[7] = 0b00000000;

	_delay_ms(2300);
	receiveAsMasterWithInterrupt(SLAVEADRESS, &msgToReceive);


	_delay_ms(2000);
	sendAsMasterWithInterrupt(SLAVEADRESS, msgToSend);

	int z = 0;
	while(z < 10)
	{
		int i;
		for(i=0; i<BYTES; i++)
		{
			PORTB = msgToReceive.byte[i];
			_delay_ms(100);
		}
		z++;
	}

	msgToSend.byte[0] = 0b00011000;
	msgToSend.byte[1] = 0b00111100;
	msgToSend.byte[2] = 0b01111110;
	msgToSend.byte[3] = 0b11111111;
	msgToSend.byte[4] = 0b11100111;
	msgToSend.byte[5] = 0b11000011;
	msgToSend.byte[6] = 0b10000001;
	msgToSend.byte[7] = 0b00000000;
	sendAsMasterWithInterrupt(SLAVEADRESS, msgToSend);
	
	while(1)
	{
		int i;
		for(i=0; i<BYTES; i++)
		{
			PORTB = msgToReceive.byte[i];
			_delay_ms(100);
		}
		
	}
}

#endif