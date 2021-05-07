 /*THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * IMACS/emBRICK BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE. */

/****************************************************************
*   file bB_EasyAPI.c	                                        *
*                                                               *
*   Functions support the communication between 				*
* 	Raspberry PI and emBRICK-Bus								*
*                                                               *
*   These functions support only one brickBUS node !            *
*                                                               *
* Copyright [C] 2013 by											*
*		IMACS GmbH												*
*       Mittelfeldstr. 25, D-70806 Kornwestheim					*
*       Tel.: 07154/8083-0, Fax.: 07154/8083-29					*
*       E-Mail: info@imacs-gmbh.de								*
****************************************************************/

/************************* INCLUDES ****************************/
#include <pthread.h>
#include <bcm2835.h>
#include "bBConfigs.h"
#include "bB_EasyAPI.h"
#include <stdio.h>

/*************************** DEFINES ****************************/
#define BB_DELAY 2		// delay in ms for counter of time out
/******************* EXTERNAL VARIABLES ************************/

/******************* FUNCTION DECLARATIONS *********************/

/********************* GLOBAL VARIABLES ************************/

/**************** BOARDBUS FUNCTIONS ***************************/
/****************************************************************
*FUNCTION NAME:		scheduleStack()								*
*                                                               *
*DESCRIPTION:	  	Thread function of thread1: initializes and *
* 					read the Bus. This happens periodically		*
* 					The period is with BB_DELAY(ms) defined		* 
*                                                               *
*PARAMETER:			None										*
*                                                               *
*RETURN:			None					                    *
*                                                               *
****************************************************************/
void* scheduleStack(void *arg)
{
	while (1)
	{
		bB_Processing();

		if (BB_Timeout > 0)		// count Timeout
			BB_Timeout -= BB_DELAY; 	
		else
			BB_Timeout = 0 ;

		delay(BB_DELAY);		//Delay between calls of emBRICK-Bus
	}

}

/****************************************************************
*FUNCTION NAME:		printInHex()								*
*                                                               *
*DESCRIPTION:		The function prints hex value				*
*                                                               *
*PARAMETER:			str- decimal value, len- length 			*
*                                                               *
*RETURN:			None					                    *
*                                                               *
****************************************************************/
void printInHex(unsigned char *str, short len)
{
	short i;
	
	for (i=0; i<len; i++)
	{
		printf("%02X ", str[i]);
	}
	printf("\n");
}

/****************************************************************
*FUNCTION NAME:		bB_Init()									*
*                                                               *
*DESCRIPTION:		The function initializes IOs, SPI 			*
* 					and emBRICK protocol stack       			*
*                                                               *
*PARAMETER:			None										*
*                                                               *
*RETURN:			0- Initialisation is successfully			*
* 					-1- Error(initialisation isn't successfully)*
*                                                               *
****************************************************************/
int bB_Init(void)
{	
	int ret = 0;
	
	setupbBData();
	ret = spiInit();
	
	// Set RPI pin P1-11,12,13,15 to be an input(DIP Switch)    
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_11, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_12, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_13, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_15, BCM2835_GPIO_FSEL_INPT);
	// Set the pin P1-16,18,22(LED1-3) to be an output    
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_16, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_18, BCM2835_GPIO_FSEL_OUTP);	
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_22, BCM2835_GPIO_FSEL_OUTP);
	
	return ret;
}
/****************************************************************
*FUNCTION NAME:		bB_getNumModules()							*
*                                                               *
*DESCRIPTION:		The function retuns the number of modules 	* 
*                                                               *
*PARAMETER:			node- Node number(1...32)					*
*                                                               *
*RETURN:			Number of modules on					    *
*                                                               *
****************************************************************/
unsigned char bB_getNumModules(unsigned short node)
{
	return brickBUS[min((node-1),(MAX_KNOTEN-1))].bB_BUS.sizecount;
}
/****************************************************************
*FUNCTION NAME:		bB_getModulID()								*
*                                                               *
*DESCRIPTION:		The function gets the ID-Number of slave 	* 
* 					on node										*
*                                                               *
*PARAMETER:			node- Node number(1...32)					*
* 					slaveNo- Slave number(1...32)				*
*                                                               *
*RETURN:			ID-Number								    *
*                                                               *
****************************************************************/
short bB_getModulID(unsigned short node, unsigned short slaveNo)
{
	BRICKBUS *bus;		
	BB_SLAVE *slave;
	
	bus =   &brickBUS[min((node-1),(MAX_KNOTEN-1))];
	slave = &(bus->bB_Slaves[min((slaveNo -1),(bus->bB_BUS.size-1))]);
	return(slave->deviceID);
}

/****************************************************************
*FUNCTION NAME:		bB_Close()									*
*                                                               *
*DESCRIPTION:		The function closes spi	interfase			*
*                                                               *
*PARAMETER:			none										*
*                                                               *
*RETURN:			none					                    *
*                                                               *
****************************************************************/
void bB_Close(void)
{
	spiClose();
}
/****************************************************************
*FUNCTION NAME:		bB_getModulSwVers()							*
*                                                               *
*DESCRIPTION:		The function gets the software version		* 
* 					of slave on node							*
*                                                               *
*PARAMETER:			node- Node number(1...32)					*
* 					slaveNo- Slave number(1...32)				*
* 																*
*RETURN:			Software version							*
*                                                               *
****************************************************************/
unsigned char bB_getModulSwVers(unsigned short node, unsigned short slaveNo)
{
	BRICKBUS *bus;		
	BB_SLAVE *slave;	
	
	bus =   &brickBUS[min((node-1), (MAX_KNOTEN-1))];
	slave = &(bus->bB_Slaves[min((slaveNo -1),(bus->bB_BUS.size-1))]);
	return(slave->HardwareVersion);
	
}
/****************************************************************
*FUNCTION NAME:		bB_getModulbBVers()							*
*                                                               *
*DESCRIPTION:		The function gets the protokoll version		* 
* 					of slave on node							*
*                                                               *
*PARAMETER:			node- Node number(1...32)					*
* 					slaveNo- Slave number(1..32)				*
*                                                               *
*RETURN:			Protokoll version							*
*                                                               *
****************************************************************/
unsigned char bB_getModulbBVers(unsigned short node, unsigned short slaveNo)
{
	BRICKBUS *bus;		
	BB_SLAVE *slave;	
	
	bus =   &brickBUS[min((node-1),(MAX_KNOTEN-1))];
	slave = &(bus->bB_Slaves[min((slaveNo -1),(bus->bB_BUS.size-1))]);
	return(slave->ProtokollVersion);	
}
/****************************************************************
*FUNCTION NAME:		bB_getModulStatus()							*
*                                                               *
*DESCRIPTION:		The function gets the State of slave on node*
*                                                               *
*PARAMETER:			node- Node number(1...32)					*
* 					slaveNo- Slave number(1...32)				*
*                                                               *
*RETURN:			0- no slave									*
* 					1- ok.(no errors)							*
* 					200- little errors							*
* 					201- no date reseive from slave				*
* 					202- a lot of errors						*
*                                                               *
****************************************************************/
unsigned char bB_getModulStatus(unsigned short node, unsigned short slaveNo)
{
	BRICKBUS *bus;		
	BB_SLAVE *slave;
	unsigned char val;
	
	bus =   &brickBUS[min((node-1),(MAX_KNOTEN-1))];
	slave = &(bus->bB_Slaves[min((slaveNo -1),(bus->bB_BUS.size-1))]);
	return(slave->status);	
}
/****************************************************************
*FUNCTION NAME:		bB_terminate()								*
*                                                               *
*DESCRIPTION:		The function deinitializes IOs, SPI, 		*
* 					emBRICK Bus.					 			*
*                                                               *
*PARAMETER:			None										*
*                                                               *
*RETURN:			None										*
*                                                               *
****************************************************************/
void bB_terminate(void)
{
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_16);
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_18);
	bcm2835_gpio_clr(RPI_V2_GPIO_P1_22);
	spiClose();	//SPI-Interfase close
	BB_SLAVE_DESELECT(node);	// brickBUS close	
}
/****************************************************************
*FUNCTION NAME:		bB_getWord()								*
*                                                               *
*DESCRIPTION:		The function reads the data(16-bits)		*
* 					from slave on node							*
*				NOTE : This function is not really thread.safe !*
*				To be thread safe, a semaphore to disable       *
*				the brickBUS stack to change the status or a	*
*				critical section is required					*
*                                                               *
*PARAMETER:			node- Node number(1...32)					*
* 					slaveNo- Slave number(1...32)				*
* 					bytePos- Position of MSB in bB message		*
* 					of this slave								*
*                                                               *
*RETURN:			Data(16-bits)								*
*					0 in case of wrong parameters or			*
*					uninitialized bus							*
*                                                               *
****************************************************************/
unsigned short bB_getWord(unsigned short node, unsigned short slaveNo, unsigned short bytePos)
{	
	BRICKBUS *bus;	
	BB_SLAVE *slave;	
	unsigned short bufferIdx;
	unsigned short val = 0 ;
	
	if (node && (node <= MAX_KNOTEN))
	{
		bus =   &brickBUS[node-1];
		if (bus->bB_BUS.initialized && (slaveNo && (slaveNo <= bus->bB_BUS.size)))
		{
			slave = &(bus->bB_Slaves[min((slaveNo -1),(bus->bB_BUS.size-1))]);
			bufferIdx = slave->offsetOut + bytePos ;
			val = (((unsigned short)(bus->bBMISOBuffer[bufferIdx])) << 8)  + (unsigned short)(bus->bBMISOBuffer[bufferIdx+1]);
		}
	}
	return val;
}

/****************************************************************
*FUNCTION NAME:		bB_getByte()								*
*                                                               *
*DESCRIPTION:		The function reads the data(8-bits)			*
* 					from slave on node							*
*				NOTE : This function is not really thread.safe !*
*				To be thread safe, a semaphore to disable       *
*				the brickBUS stack to change the status or a	*
*				critical section is required					*
*                                                               *
*PARAMETER:			node- Node number(1...32)					*
* 					slaveNo- Slave number(1...32)				*
* 					bytePos- Position of byte in bB message		*
* 					of this slave								*
*                                                               *
*RETURN:			Data(8-bits)								*
*					0 in case of wrong parameters or			*
*					uninitialized bus							*
*                                                               *
****************************************************************/	
unsigned char bB_getByte(unsigned short node, unsigned short slaveNo, unsigned short bytePos)
{
	BRICKBUS *bus;
	BB_SLAVE *slave;
	unsigned short bufferIdx;
	unsigned char val = 0 ;
	
	if (node && (node <= MAX_KNOTEN))
	{
		bus =   &brickBUS[node-1];
		if (bus->bB_BUS.initialized && (slaveNo && (slaveNo <= bus->bB_BUS.size)))
		{
			slave = &(bus->bB_Slaves[min((slaveNo -1),(bus->bB_BUS.size-1))]);
			bufferIdx = slave->offsetOut + bytePos ;
	
			val = bus->bBMISOBuffer[bufferIdx];
		}
	}
	return val;
}
/****************************************************************
*FUNCTION NAME:		bB_getBit()									*
*                                                               *
*DESCRIPTION:		The function reads the bit state			*
* 					in byte from slave on node					*
*				NOTE : This function is not really thread.safe !*
*				To be thread safe, a semaphore to disable       *
*				the brickBUS stack to change the status or a	*
*				critical section is required					*
*                                                               *
*PARAMETER:			node- Node number(1...32)					*
* 					slaveNo- Slave number(1...32)				*
* 					bytePos- Position of byte in bB message		*
* 					of this slave								*
* 					bitPos- Position of bit in byte				*
*                                                               *
*RETURN:			Bit state, 									*
*					0 in case of wrong parameters or			*
*					uninitialized bus							*
*                                                               *
****************************************************************/	
unsigned char bB_getBit(unsigned short node, unsigned short slaveNo, unsigned short bytePos, unsigned char bitPos)
{
	BRICKBUS *bus;	
	BB_SLAVE *slave;	
	unsigned short bufferIdx;

	unsigned char val = 0 ;
	
	if (node && (node <= MAX_KNOTEN))
	{
		bus =   &brickBUS[node-1];
		if (bus->bB_BUS.initialized && (slaveNo && (slaveNo <= bus->bB_BUS.size)))
		{
			slave = &(bus->bB_Slaves[min((slaveNo -1),(bus->bB_BUS.size-1))]);
			bufferIdx = slave->offsetOut + bytePos ;
			val = (bus->bBMISOBuffer[bufferIdx] & (1 << bitPos)) ? 1 : 0;	// if true then 1 else 0
		}
	}
	return val;
}

/****************************************************************
*FUNCTION NAME:		bB_putWord()								*
*                                                               *
*DESCRIPTION:		The function writes the data(16-bits)		*
* 					into slave on node							*
*				NOTE : This function is not really thread.safe !*
*				To be thread safe, a semaphore to disable       *
*				the brickBUS stack to change the status or a	*
*				critical section is required					*
*                                                               *
*PARAMETER:			node- Node number(1...32)					*
* 				slaveNo- Slave number(1...32)					*
* 				bytePos- Position of MSB in bB message			*
* 				to this slave									*
* 				value- to be written(0...65535)					*
*                                                               *
*RETURN:		0 = succeeded									*
*				-1 = Bus not initialized                        *
*				-2 = non-existing slave	                        *
*				-3 = non-existing node	                        *
*                                                               *
****************************************************************/
short bB_putWord(unsigned short node, unsigned short slaveNo, unsigned short bytePos, short value)
{
	BRICKBUS *bus;
	BB_SLAVE *slave;
	unsigned short bufferIdx;

	if ((!node) || (node > MAX_KNOTEN))
		return -3 ;
	bus =   &brickBUS[node-1];
	if (!bus->bB_BUS.initialized)
		return -1 ;
	if (slaveNo > bus->bB_BUS.size)
		return -2 ;
	slave = &(bus->bB_Slaves[slaveNo -1]);

	bufferIdx = slave->offsetIn + bytePos;

	bus->bBMOSIBuffer[bufferIdx+1] = value & 255;
	bus->bBMOSIBuffer[bufferIdx] = (value >>  8);

	return 0 ;
}
/****************************************************************
*FUNCTION NAME:		bB_putByte()								*
*                                                               *
*DESCRIPTION:		The function writes the data(8-bits)		*
* 					into slave on node							*
*				NOTE : This function is not really thread.safe !*
*				To be thread safe, a semaphore to disable       *
*				the brickBUS stack to change the status or a	*
*				critical section is required					*
*                                                               *
*PARAMETER:			node- Node number(1...32)					*
* 					slaveNo- Slave number(1...32)				*
* 					bytePos- Position of byte in bB message		*
* 					to this slave								*
* 					value- to be written(0...65535)				*
*                                                               *
*RETURN:		0 = succeeded									*
*				-1 = Bus not initialized                        *
*				-2 = non-existing slave	                        *
*				-3 = non-existing node	                        *
*                                                               *
****************************************************************/	
short bB_putByte(unsigned short node, unsigned short slaveNo, unsigned short bytePos, unsigned char value)
{
	BRICKBUS *bus;	
	BB_SLAVE *slave;	
	unsigned char bufferIdx;
	
	if ((!node) || (node > MAX_KNOTEN))
		return -3 ;
	bus =   &brickBUS[node-1];
	if (!bus->bB_BUS.initialized)
		return -1 ;
	if (slaveNo > bus->bB_BUS.size)
		return -2 ;
	slave = &(bus->bB_Slaves[slaveNo -1]);
	bufferIdx = slave->offsetIn + bytePos ;
	bus->bBMOSIBuffer[bufferIdx] = value;		
	return 0 ;
}

/****************************************************************
*FUNCTION NAME:		bB_putBit()									*
*                                                               *
*DESCRIPTION:		The function write the bit 					*
* 					in byte into slave on node					*
*				NOTE : This function is not really thread.safe !*
*				To be thread safe, a semaphore to disable       *
*				the brickBUS stack to change the status or a	*
*				critical section is required					*
*                                                               *
*PARAMETER:			node- Node number(1...32)					*
* 					slaveNo- Slave number(1...32)				*
* 					bytePos- Position of byte in bB message		*
* 					to this slave								*
* 					bitPos- Position of bit in byte(0...7)		*
* 					value- to be written(0/1)					*
*                                                               *
*RETURN:		0 = succeeded									*
*				-1 = Bus not initialized                        *
*				-2 = non-existing slave	                        *
*				-3 = non-existing node	                        *
*                                                               *
****************************************************************/
short bB_putBit(unsigned short node, unsigned short slaveNo, unsigned short bytePos, unsigned char bitPos, unsigned char value)
{
	BRICKBUS *bus;
	BB_SLAVE *slave;
	unsigned char bufferIdx;
	
	if ((!node) || (node > MAX_KNOTEN))
		return -3 ;
	bus =   &brickBUS[node-1];
	if (!bus->bB_BUS.initialized)
		return -1 ;
	if (slaveNo > bus->bB_BUS.size)
		return -2 ;
	slave = &(bus->bB_Slaves[slaveNo -1]);
	bufferIdx = slave->offsetIn + bytePos ;
		
	if(value)
		(bus->bBMOSIBuffer[bufferIdx]) |= (1 << bitPos);
	else
		(bus->bBMOSIBuffer[bufferIdx]) &= ~(1 << bitPos);	
	return 0 ;
}




