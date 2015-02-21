//##################################################
//#Sunday, 05.September.2014 made by Lars C. Schwensen#
//##################################################

//#################################################################
//#This file handles the I2C functionality                        #
//#DEVICE: atmega8                                                #
//#DEVELOPMENT BOARD: OLIMEX P-28                                 #
//#PROGRAMMER: USBPROG (avrispmkII clone)                         #
//Notice: http://www.mikrocontroller.net/articles/AVR_TWI         #
//#################################################################

//i2c.c

#include "i2c.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#include <util/twi.h>
#include <util/delay.h>

void setInterrupt()
{
	DDRC |= (1<<PC3);
	PORTC &= ~(1<<PC3);
}

void revokeInterrupt()
{
	DDRC |= (1<<PC3);
	PORTC |= (1<<PC3);
}

uint32_t calcSCLFrequency(uint32_t bitrate)
{
	//http://www.atmel.com/Images/doc2564.pdf (p.5)
	return (F_CPU / (bitrate * 2)) - (16/2);
}

void initAsSlave(uint8_t deviceAdress)
{
	//set direction of TWI pins
	DDRC &= ~((1 << I2C_SDA) | (1 << I2C_SCL));
	PORTC |= (1 << I2C_SDA) | (1 << I2C_SCL);

	//set the adress of the device
	//shifting is necessary because 7bits (up to MSB) are used to send the adress 
	//and the LSB is used to set the read/write direction
	TWAR = (deviceAdress << 1);

	//TWEN enables i2c
	//TWEA enables response of device
	//TWINT clears the TWI interrupt
	TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
}

void initAsMaster(uint32_t bitrate)
{
	//set direction of TWI pins
	DDRC &= ~((1 << I2C_SDA) | (1 << I2C_SCL));
	PORTC |= (1 << I2C_SDA) | (1 << I2C_SCL);
	
	//set bitrate
	TWBR = calcSCLFrequency(bitrate);
}

uint8_t startMaster(uint8_t rwCondition)
{
	uint8_t twsr = 0;

	//TWEN enables TWI
	//TWIN clears TWI interrupt
	//TWSTA sends START condition
	TWCR =  (1<<TWINT) |(1<<TWSTA) | (1<<TWEN);

	//wait until TWI bus is free
	while(!(TWCR &(1<<TWINT)));

	//check status register. MASK first three bits with (0xF8)
	//0x08 -> bus is free
	twsr = TWSR & 0xF8;
	if(twsr != 0x08)
	{		
		return FALSE;
	}

	//fill message with adress and read/write condition
	TWDR = rwCondition;

	//send message
	TWCR = (1<<TWINT) | (1<<TWEN);
	
	//wait until slave received the message
	while(!(TWCR &(1<<TWINT)));

	return TWSR & 0xF8;	
}

uint8_t sendAsMaster(uint8_t slaveAdress, struct Message msg)
{
	uint8_t twsr = startMaster(WRITE_TO_SLAVE(slaveAdress));

	if(twsr != 0x18)
	{	
		return FALSE;
	}

	int i;
	for(i=0; i<BYTES; i++)
	{		
		//fill message with byte
		TWDR = msg.byte[i];
		
		//send message	
		TWCR = (1<<TWINT) | (1<<TWEN);
		
		//wait until slave received the message
		while(!(TWCR &(1<<TWINT)));

		//check status register. MASK first three bits
		//0x28 -> byte transmitted and ACK received
		//0x30 -> byte transmitted and NACK received. Slave is not able to get anymore bytes
		twsr = TWSR & 0xF8;	
		if(twsr != 0x28 && twsr != 0x30)
		{			
			return FALSE;
		}
		if(twsr == 0x30)
		{
			break;
		}
	}
	
	//TWEN enables TWI
	//TWIN clears TWI interrupt
	//TWSTA sends STOP condition
    TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);

    return TRUE;
}

uint8_t sendAsMasterWithInterrupt(uint8_t slaveAdress, struct Message msg)
{
	uint8_t twsr;

	setInterrupt();
	
	twsr = sendAsMaster(slaveAdress, msg);
    
    revokeInterrupt();
   	
    _delay_ms(10);

    return twsr;
}

uint8_t sendAsSlave(struct Message msg)
{
	uint8_t twsr;

	int i;
	for(i=0; i<BYTES; i++)
	{				
		TWDR = msg.byte[i];
		
		if(i < BYTES-1)
		{
			//TWEN enables TWI
			//TWINT clears TWI interrupt
			//TWEA send acknowledge bit
			TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
		
			//wait until master received the message
			while(!(TWCR &(1<<TWINT)));		
		}
		else
		{
			//TWEŃ enables TWI
			//TWIN clears TWI interrupt
			//Send no more acknowledge bit so the Master get to know slave can't receive any more bytes
			TWCR =  (1<<TWINT) | (1<<TWEN);

			while (!(TWCR & (1<<TWINT)));
		}
		
	}
	
	//set for the next message
	TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
}

uint8_t receiveAsMaster(uint8_t slaveAdress, struct Message *msg)
{
	uint8_t twsr = startMaster(RECEIVE_FROM_SLAVE(slaveAdress));

	if(twsr != 0x40)
	{	
		return FALSE;
	}

	int i;
	for(i=0; i<BYTES; i++)
	{
		//TWEN enables TWI
		//TWIN clears TWI interrupt
		//TWEA send acknowledge bit
		TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
		
		//wait until master received the message
		while (!(TWCR & (1<<TWINT)));
		
		//save received byte
		msg->byte[i] = TWDR;
			
		//check status register. 
		twsr = TWSR & 0xF8;			
		if(twsr != 0x50 && twsr != 0x58)
		{
			return FALSE;
		}
		if(TWSR == 0x58)
		{
			break;
		}					
	}

	//TWEN enables TWI
	//TWIN clears TWI interrupt
	//TWSTA sends STOP condition
	TWCR = (1<<TWINT)| (1<<TWSTO) | (1<<TWEN) ;

    return TRUE;
}

uint8_t receiveAsMasterWithInterrupt(uint8_t slaveAdress, struct Message *msg)
{
	uint8_t twsr;

	setInterrupt();
	
	_delay_us(CLI_TIME_us);
	twsr = receiveAsMaster(slaveAdress, msg);
    
    revokeInterrupt();
   	
    _delay_ms(10);

    return twsr;
}

void receiveAsSlave(struct Message *msg)
{	
	int i;
	for(i=0; i<BYTES; i++)
	{		
			//TWEŃ enables TWI
			//TWEA enables acknowledge bit
			//TWIN clears TWI interrupt
			TWCR = (1<<TWINT)| (1<<TWEA) | (1<<TWEN) ;
		
			//wait until received the message
			while (!(TWCR & (1<<TWINT)));

			PORTB = i;
			//write received byte into the struct
			msg->byte[i] = TWDR;
	}

	//TWEŃ enables TWI
	//TWIN clears TWI interrupt
	//Send no more acknowledge bit so the Master get to know slave can't receive any more bytes
	TWCR =  (1<<TWINT) | (1<<TWEN);

	while (!(TWCR & (1<<TWINT)));

	//set for the next message
	TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
}

uint8_t checkRequest(void)
{
	//TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWEA);
	while (!(TWCR & (1<<TWINT)));

	//returns the received adress and read/write condition
	return TWSR;
}