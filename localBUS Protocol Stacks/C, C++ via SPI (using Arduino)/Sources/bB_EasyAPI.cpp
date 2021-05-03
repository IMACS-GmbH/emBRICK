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
#include "brickbus.h"
#include "bBConfigs.h"
#include "bB_EasyAPI.h"
#include <Arduino.h>

/*************************** DEFINES ****************************/
#define delayMS(x)	delayMicroseconds((x)*1000L)

/******************* EXTERNAL VARIABLES ************************/

/******************* FUNCTION DECLARATIONS *********************/

/********************* GLOBAL VARIABLES ************************/

/**************** BOARDBUS FUNCTIONS ***************************/


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
	// Close brickBUS
	BB_SLAVE_DESELECT(node);
}

/****************************************************************
*FUNCTION NAME:		bB_getWord()								*
*                                                               *
*DESCRIPTION:		The function reads the data(16-bits)		*
* 					from slave on node							*
*				NOTE : This function is not really thread safe! *
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
	unsigned short val = 0;
	
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
*				NOTE : This function is not really thread safe! *
*				To be thread safe, a semaphore to disable       *
*				the brickBUS stack to change the status or a	*
*				critical section is required					*
*                                                               *
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
	unsigned char val = 0;
	
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
*				NOTE : This function is not really thread safe! *
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
*RETURN:			Bit state									*
*					0 in case of wrong parameters or			*
*					uninitialized bus							*
*                                                               *
****************************************************************/	
unsigned char bB_getBit(unsigned short node, unsigned short slaveNo, unsigned short bytePos, unsigned char bitPos)
{
	BRICKBUS *bus;	
	BB_SLAVE *slave;	
	unsigned short bufferIdx;

	unsigned char val = 0;
	
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
*				NOTE : This function is not really thread safe! *
*				To be thread safe, a semaphore to disable       *
*				the brickBUS stack to change the status or a	*
*				critical section is required					*
*                                                               *
*PARAMETER:			node- Node number(1...32)					*
* 					slaveNo- Slave number(1...32)				*
* 					bytePos- Position of MSB in bB message		*
* 					to this slave								*
* 					value- to be written(0...65535)				*
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
*				NOTE : This function is not really thread safe! *
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
	slave = &(bus->bB_Slaves[slaveNo-1]);
	bufferIdx = slave->offsetIn + bytePos;
	bus->bBMOSIBuffer[bufferIdx] = value;

	return 0 ;
}

/****************************************************************
*FUNCTION NAME:		bB_putBit()									*
*                                                               *
*DESCRIPTION:		The function write the bit 					*
* 					in byte into slave on node					*
*				NOTE : This function is not really thread safe! *
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
short bB_putBit(unsigned short node, unsigned short slaveNo, unsigned short bytePos, unsigned char bitPos, unsigned char value) //unsigned char = 8 bit
{
	BRICKBUS *bus;
	BB_SLAVE *slave;
	unsigned char bufferIdx;
	unsigned char mosivalue ;
	
	if ((!node) || (node > MAX_KNOTEN))
		return -3 ;
   
	bus =   &brickBUS[node-1];
	if (!bus->bB_BUS.initialized)
		return -1 ;

	if (slaveNo > bus->bB_BUS.size)
		return -2 ;
   
	slave = &(bus->bB_Slaves[slaveNo -1]);
	bufferIdx = slave->offsetIn + bytePos ;

	mosivalue = bus->bBMOSIBuffer[bufferIdx] & (~(1<<bitPos)) ;
	bus->bBMOSIBuffer[bufferIdx]  = mosivalue | (value<<bitPos);

	return 0 ;
}

/****************************************************************
*FUNCTION NAME:		bB_Start()									*
*                                                               *
*DESCRIPTION:		The function initializes SPI and set 	 	*
*					emBRICK protocol 		      				*
*PARAMETER:			none										*
*RETURN:			none									*
****************************************************************/

void bB_Start()
{
	// Startet Serielle Kommumikation mit PC
	Serial.begin(9600);
	
	// stellt SPI ein
	setupbBData();		

	Serial.println("---------------------------------------");
	Serial.println("emBRICK(R), Arduino Development Version");
	Serial.println("BSP and sample application, V0.01");
	Serial.print("brickBUS stack version: ");
    Serial.println(BRICKBUS_STACK_VERSION);
	Serial.println("(c) 2018 by IMACS GmbH");
	Serial.println("---------------------------------------");
	Serial.println("Initialize bus .... ");
}
