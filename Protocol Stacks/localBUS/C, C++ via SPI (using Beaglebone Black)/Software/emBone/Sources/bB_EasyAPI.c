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
#include "brickbus.h"
#include "bBConfigs.h"
#include "bB_EasyAPI.h"
#include <stdio.h>

/*************************** DEFINES ****************************/
#define BB_DELAY 2		// delay in ms for counter of time out
#define delayMS(x)	usleep((x)*1000L);

/******************* EXTERNAL VARIABLES ************************/
//extern pthread_t threadApplication;

/******************* FUNCTION DECLARATIONS *********************/
extern void* scheduleStack(void *arg);		// thread function for bB stack
//extern void* bB_Appl(void* arg);

/********************* GLOBAL VARIABLES ************************/
pthread_t threadScheduler;								// Scheduler Task Handle
pthread_t threadApplication;							// Application task Handle


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
*FUNCTION NAME:	bB_startScheduler()															*
*                                                               *
*DESCRIPTION:		This function will start the scheduler					*                                                        *
*PARAMETER:			priority level																	*                                                               *
*RETURN:				-1 = failure																		*
								0 = success																			*
****************************************************************/
char bB_startScheduler(unsigned short priority)
{
	pthread_attr_t attr;
	int err;
	struct sched_param param;
	
	bB_stopScheduler(); 	// kill thread if already existing
	
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	// thread 1 : periodic call of brickBUS stack
	err = pthread_create(&threadScheduler, &attr, &scheduleStack, NULL);
  if (err != 0)
  {
         printf("\n The thread cannot be generated:[%s]", strerror(err));
         return -1;
  }
  param.sched_priority = priority;
	pthread_setschedparam(threadScheduler, SCHED_RR, &param);
	
	return 0;
}

/****************************************************************
*FUNCTION NAME:	bB_stopScheduler()															*
*                                                               *
*DESCRIPTION:		This function will stop the scheduler						*                                                            *
*PARAMETER:			none																						*                                                             *
*RETURN:				none																						*
****************************************************************/

void bB_stopScheduler()
{
	if(threadScheduler)
		pthread_cancel(threadScheduler);
}

/****************************************************************
*FUNCTION NAME:		bB_Init()																			*
*                                                               *
*DESCRIPTION:		The function initializes IOs, SPI 							*
*								and emBRICK protocol stack       								*
*                                                               *
*PARAMETER:			none																						*
*RETURN:				0: Initialisation is successfull								*
****************************************************************/
int bB_Init(void)
{	
	extern void initBrickBus(void);
	int ret = 0;
	
	setupbBData();
	initBrickBus();
	
	return ret;
}

/****************************************************************
*FUNCTION NAME:		bB_Start()																			*
*                                                               *
*DESCRIPTION:		The function initializes IOs, SPI and emBRICK 	*
*								protocol stack       														*
*PARAMETER:			none																						*
*RETURN:				none																						*
****************************************************************/

int bB_Start()
{
	short ret, modules, i;
	
	// terminate brickBUS if already initialized
	bB_terminate();				
	
	// initializes IOs, SPI and emBRICK protocol stack
	ret = bB_Init();		

	printf("---------------------------------------\n");
	printf("emBRICK(R), BeagleBONE Development Version\n");
	printf("BSP and sample application, V0.07\n");
	printf("brickBUS stack version: %02d \n",BRICKBUS_STACK_VERSION);
	printf("(c) 2016 by IMACS GmbH\n");
	printf("---------------------------------------\n");
	printf("Initialize bus .... \n");
	
	// start schedulerTask
	ret += bB_startScheduler(0);
	
	if (ret == 0)	// When no error, continue
	{		
		// wait until brickBUS is operating
		while(!brickBUS[0].bB_BUS.initialized)
		{
			delayMS(20);
		}
		printf("successfull\n");
		
		// list found modules with details
		modules = bB_getNumModules(1);	
 		printf("module(s) found ... %d\n", modules);
		for (i=1; i <= modules; i++) 
			printf("   module %02d ...... ID:%1d-%03d, Mod-Vers: %02d, Prot-Vers: %02d\n",
					i, bB_getModulID(1,i)/1000, bB_getModulID(1,i)%1000, bB_getModulSwVers(1,i), bB_getModulbBVers(1,i)); 
 		printf("\n--- RUN ---\n");	// bus now running
 		return 0;
 	}else	
 	{
		printf("Error %d: init not successfull", ret);
		bB_stopScheduler();
		bB_terminate();				// terminate brickBUS
		
		return -1;
	}
}
