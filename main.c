#include "i2c.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>


//comment this to create a slave
//uncomment this to create a master
//#define MASTER


#ifndef MASTER

struct Message msg;

ISR(INT0_vect)       
{
   	cli();
  
  	switch(checkRequest())
  	{
  		case REQUEST_TO_WRITE:
  		
  			break;

  		case REQUEST_TO_READ:
  			receiveAsSlave(&msg);
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
	initINT0();

	while(1)
	{
		//cli();
		//_delay_us(CLI_TIME_us);
		
		PORTB = msg.byte[0];
		_delay_ms(250);		
		PORTB = msg.byte[1];
		_delay_ms(250);		
		PORTB = msg.byte[2];
		_delay_ms(250);	
		PORTB = msg.byte[3];
		_delay_ms(250);		
		PORTB = msg.byte[4];
		_delay_ms(250);
		PORTB = msg.byte[5];
		_delay_ms(250);
		PORTB = msg.byte[6];
		_delay_ms(250);
		PORTB = msg.byte[7];
		_delay_ms(250);		
		//sei();
	}
}

#endif

#ifdef MASTER

int main(void)
{
	DDRB = 0xff;
	PORTB = 0x00;
	

	DDRD = 0xFF;
	PORTD |= (1<<PD2);

	struct Message msg;
	msg.byte[0] = 0b00000001;
	msg.byte[1] = 0b00000011;
	msg.byte[2] = 0b00000111;
	msg.byte[3] = 0b00001111;	
	msg.byte[4] = 0b00011111;
	msg.byte[5] = 0b00111111;
	msg.byte[6] = 0b01111111;
	msg.byte[7] = 0b11111111;
	

	initAsMaster(50000);
	
	_delay_ms(4000);	
	uint8_t twst = 0;
		
	twst = sendAsMasterWithInterrupt(SLAVEADRESS, msg);
	
	_delay_ms(4000);

	msg.byte[0] = 0b00000000;
	msg.byte[1] = 0b10000001;
	msg.byte[2] = 0b11000011;
	msg.byte[3] = 0b11100111;
	msg.byte[4] = 0b00011000;	
	msg.byte[5] = 0b00111100;
	msg.byte[6] = 0b01111110;
	msg.byte[7] = 0b11111111;
	
	twst = sendAsMasterWithInterrupt(SLAVEADRESS, msg);


	while(1)
	{		
	}
}


#endif
