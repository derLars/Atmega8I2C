//##################################################
//#Sunday, 05.September.2014 made by Lars C. Schwensen#
//##################################################

//#################################################################
//#This file handles the I2C functionality                        #
//#DEVICE: atmega8                                                #
//#DEVELOPMENT BOARD: OLIMEX P-28                                 #
//#PROGRAMMER: USBPROG (avrispmkII clone)                         #
//#################################################################

//i2c.h

#include <stdint.h>

#ifndef F_CPU
#define F_CPU 				1000000UL
#endif

#define SLAVEADRESS 		0x24

//leftshifting is necessary because the last seven bit up to MSB define the adress
//and the LSB defines the read/write condition
#define WRITE_TO_SLAVE(ADRESS)		((ADRESS << 1) + 0)
#define RECEIVE_FROM_SLAVE(ADRESS) 	((ADRESS << 1) + 1)

#define REQUEST_TO_READ 	0x60
#define REQUEST_TO_WRITE 	0xA8

#define I2C_SDA             PC4
#define I2C_SCL             PC5

#define BYTES 				8

#define TRUE 				1
#define FALSE 				0

#define SLC_FREQUENCY		50000

#define CLI_TIME_us 80000

struct Message
{
	volatile uint8_t adress;
	volatile uint8_t byte[BYTES];
};

void initAsSlave(uint8_t deviceAdress);
void initAsMaster(uint32_t bitrate);

uint8_t sendAsMaster(uint8_t slaveAdress, struct Message msg);
uint8_t sendAsMasterWithInterrupt(uint8_t slaveAdress, struct Message msg);

uint8_t sendAsSlave(struct Message msg);

uint8_t receiveAsMaster(uint8_t slaveAdress, struct Message *msg);
uint8_t receiveAsMasterWithInterrupt(uint8_t slaveAdress, struct Message *msg);

void receiveAsSlave(struct Message *msg);

uint8_t checkRequest(void);