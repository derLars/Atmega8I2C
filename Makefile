##################################################
#Sunday, 31.August.2014 made by Lars C. Schwensen#
##################################################

#################################################################
#This is a standard makefile to compile and flash an ATmega8    #
#on an OLIMEX p-28 development board.                           #
#The programmer is an USBPROG which works as an avrispmkII clone#
#################################################################

#the µC device
MCU=atmega8

#the name of the finally output file
OUTPUTNAME=pr0gramm

#the format of the finally output file
OUTPUTFORMAT=ihex

#all the project files
#REPLACE THIS FILES WITH YOUR PROJECT FILES!!!!
PRJSRC=main.c i2c.c 

#project files as .o files
OBJECTS= $(PRJSRC:.c=.o)

#additional includes
INCLUDES=
LIBRARIES=
#optimization level between (s0 (zero) and s3 (maximum))
OPTIMIZE=s

#the Programmer
PROGRAMMER=avrispmkII
CONNECTION=usb

#compilerflags
CFLAGS= -mmcu=$(MCU) -O$(OPTIMIZE)

#linkerflags
LDFLAGS= 

#executables
CC=/opt/AVR/avr-gnu-toolchain/bin/avr-gcc
OBJCOPY=/opt/AVR/avr-gnu-toolchain/bin/avr-objcopy
AVRDUDE=avrdude

all: $(OUTPUTNAME).hex clean flash

#create the hex file
$(OUTPUTNAME).hex : $(OUTPUTNAME).elf
	$(OBJCOPY)  -j .text -j .data -O $(OUTPUTFORMAT) $(OUTPUTNAME).elf $(OUTPUTNAME).hex

#link the object files 
$(OUTPUTNAME).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUTPUTNAME).elf $(LDFLAGS) $(OBJECTS)

#create object files
.c.o: 
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

#flash the µC
flash:
	sudo $(AVRDUDE) -F -V -c $(PROGRAMMER) -p ATmega8 -P $(CONNECTION) -U flash:w:$(OUTPUTNAME).hex

clean:
	rm -f *.o
	rm -f *.elf
