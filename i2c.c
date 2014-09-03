//##################################################
//#Sunday, 03.September.2014 made by Lars C. Schwensen#
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
	DDRD |= (1<<PD2);
	PORTD &= ~(1<<PD2);
}

void revokeInterrupt()
{
	DDRD |= (1<<PD2);
	PORTD |= (1<<PD2);
}

uint32_t calcSCLFrequency(uint32_t bitrate)
{
	//http://www.atmel.com/Images/doc2564.pdf (p.5)
	return (F_CPU / (bitrate * 2)) - (16/2);
}

void initAsSlave(uint8_t deviceAdress)
{
	DDRC &= ~((1 << I2C_SDA) | (1 << I2C_SCL));
	PORTC |= (1 << I2C_SDA) | (1 << I2C_SCL);

	//set the adress of the device
	//shifting is necessary because 7bits (up to MSB) are used to send the adress 
	//and the LSB is used to set the read/write direction
	TWAR = (deviceAdress << 1);

	//TWEN enables i2c
	//TWEA enables response of device
	//TWINT clears the TWI interrupt
	TWCR &= ~(1<<TWSTA) | (1<<TWSTO);
	TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWINT);
}

void initAsMaster(uint32_t bitrate)
{
	DDRC &= ~((1 << I2C_SDA) | (1 << I2C_SCL));
	PORTC |= (1 << I2C_SDA) | (1 << I2C_SCL);
	
	//set bitrate
	TWBR = calcSCLFrequency(bitrate);
}

uint8_t sendAsMaster(uint8_t slaveAdress, struct Message msg)
{
	uint8_t twst = 0;
	//TWEN enables TWI
	//TWIN clears TWI interrupt
	//TWSTA sends START condition
	TWCR=(1<<TWEN) | (1<<TWINT) | (1<<TWSTA);
	
	//wait until TWI bus is free
	while(!(TWCR &(1<<TWINT)));

	//check status register. MASK first three bits
	//0x08 -> bus is free
	twst = TWSR & 0xF8;
	if(twst != 0x08)
	{		

		return FALSE;
	}

	//fill message with adress and read/write condition
	//leftshifting is necessary because the last seven bit up to MSB define the adress
	//and the LSB defines the read/write condition
	TWDR = WRITE_TO_SLAVE(slaveAdress);
	
	//send message
	TWCR=(1<<TWINT) | (1<<TWEN);
	
	//wait until slave received the message
	while(!(TWCR &(1<<TWINT)));

	//check status register. MASK first three bits
	//0x18 -> adress transmitted and ACK received
	twst = TWSR & 0xF8;	
	if(twst != 0x18)
	{	
		return FALSE;
	}

	int i;
	for(i=0; i<BYTES; i++)
	{
		//PORTB = i;
		//fill message with byte
		TWDR = msg.byte[i];
		
		//send message
		
		TWCR=(1<<TWINT) | (1<<TWEN);
	
		
		//wait until slave received the message
		while(!(TWCR &(1<<TWINT)));

		//check status register. MASK first three bits
		//0x28 -> byte transmitted and ACK received
		//0x30 -> byte transmitted and NACK received. Slave is not able to get anymore bytes
		twst = TWSR & 0xF8;	
		if(twst != 0x28 && twst != 0x30)
		{			
			return FALSE;
		}
		if(twst == 0x30)
		{
			break;
		}
	}
	
	//TWEN enables TWI
	//TWIN clears TWI interrupt
	//TWSTA sends STOP condition
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
    

    return TRUE;
}

uint8_t sendAsMasterWithInterrupt(uint8_t slaveAdress, struct Message msg)
{
	uint8_t twst;
	
	setInterrupt();
	
	_delay_us(CLI_TIME_us);
	twst = sendAsMaster(slaveAdress, msg);
    
    revokeInterrupt();
   	
    _delay_ms(10);
    return twst;
}

void receiveAsSlave(struct Message *msg)
{	
	int i;
	for(i=0; i<BYTES; i++){		
		//TWEŃ enables TWI
		//TWEA enables acknowledge bit
		//TWIN clears TWI interrupt
		TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWINT);
		
		//wait until received the message
		while (!(TWCR & (1<<TWINT)));

		//write received byte into the struct
		msg->byte[i] = TWDR;
	}


	//TWCR &= ~(1<<TWSTA) | (1<<TWSTO);
	//TWEŃ enables TWI
	//TWIN clears TWI interrupt
	//Send no more acknowledge bit so the Master get to know slave can't receive any more bytes
	TWCR = (1<<TWEN) | (1<<TWINT);

	while (!(TWCR & (1<<TWINT)));

	//set for the next message
	TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWINT);
}

uint8_t checkRequest(void)
{
	TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWINT);
	while (!(TWCR & (1<<TWINT)));

	//returns the received adress and read/write condition
	return TWDR;
}