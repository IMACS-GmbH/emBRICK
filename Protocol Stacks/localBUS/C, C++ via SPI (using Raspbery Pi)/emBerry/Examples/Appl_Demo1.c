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

/************************************************************************
* Simple Demo how to integrate the emBRICK (R) brickBUS stack	 		*
*																		*
* feel free to ...														*
* - schedule the protocol stack more exactly e.g. using interrupts		*
* - enhance the data exchange											*
* - modify anything else												*
* 																		*
************************************************************************/

/************************* INCLUDES ****************************/
#include "bBConfigs.h"
#include "bB_EasyAPI.h"
#include <pthread.h>

/*************************** DEFINES ****************************/
#define UPDATE_RATE		50			// ms	

/******************* EXTERNAL VARIABLES ************************/

/******************* FUNCTION DECLARATIONS *********************/
void* scheduleStack(void *arg);		// thread function for bB stack
//int threadInit(void);

/********************* GLOBAL VARIABLES ************************/
long BB_Timeout = 0 ;	// ms based Timeout, used by brickBUS Lib functions
pthread_t thread1;
pthread_t thread2;	

/**************** DEMO FUNCTIONS ***************************/

/****************************************************************
*FUNCTION NAME:		bB_Appl()									*
*                                                               *
*DESCRIPTION:		Demo function switching the relais and		*
* 					copying an input bit to an output bit	 	*
* 					This function must be called periodically	*
* 																*
*PARAMETER:			None										*
*                                                               *
*RETURN:			None					                    *
*                                                               *
****************************************************************/
void bB_Appl(void)
{
	static int counter;
	char b;
			
	counter++;		
	bB_putBit(MOD1_REL1, (counter & 0x10) ? 1:0); // Relays switching periodically, set/clear output bit (1,1,0,0) Node 1, Module 1, Byte 0, Bit 0
	
	b = bB_getBit(MOD1_DI1);	// read input bit (1,1,4,2)--> Node 1, Module 1, Byte 4, Bit 2
	bB_putBit(MOD1_REL2, b);	// set output bit (1,1,0,1)--> Node 1, Module 1, Byte 0, Bit 1 
}

/****************************************************************
*FUNCTION NAME:		scheduleTask()								*
*                                                               *
*DESCRIPTION:		Thread function of thread2: is calling the 	*
* 					user function every 50ms					*
*                                                               *
*PARAMETER:			None										*
*                                                               *
*RETURN:			None					                    *
*                                                               *
****************************************************************/
void* scheduleTask(void *arg)
{
	while (1)
	{
		bB_Appl();
		delay(50);		//Delay 50ms
	}
}

/****************************************************************
*FUNCTION NAME:		threadInit()								*
*                                                               *
*DESCRIPTION:		The function initializes and creates threads*
* 					also set the priority of threads 			*	
*                   uses standart linux commands                *
*                                                               *
*PARAMETER:			none										*
*                                                               *
*RETURN:			0-Threads were generated successfully		*
* 					1-Error in generating of threads			*
*                                                               *
****************************************************************/
int threadInit()
{
	pthread_attr_t attr;
	
	pthread_t this_p;
	int err;
	struct sched_param param;
	
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	this_p = pthread_self();
	
	// thread 1 : periodic call of brickBUS stack
	err = pthread_create(&thread1, &attr, &scheduleStack, NULL);
    if (err != 0)
    {
           printf("\n The thread cannot be generated:[%s]", strerror(err));
           return -1;
    }

	// thread 2 : Application Demo
	err = pthread_create(&thread2, &attr, &scheduleTask, NULL);
    if (err != 0)
    {
           printf("\n The thread cannot be generated:[%s]", strerror(err));
           return -1;
    }

	param.sched_priority = 0;
	pthread_setschedparam(this_p, SCHED_RR, &param);
	param.sched_priority = 10; 
	pthread_setschedparam(thread2, SCHED_RR, &param);
	param.sched_priority = 20; 
	pthread_setschedparam(thread1, SCHED_RR, &param);
	
	return 0;
}

/****************************************************************
*FUNCTION NAME:		main()										*
*                                                               *
*DESCRIPTION:		main function								*
*                                                               *
*PARAMETER:			standard params, not used herein			*
*                                                               *
*RETURN:			always 0									*
*                                                               *
****************************************************************/
int main(int argc, char **argv)
{
	short ret, modules, i;

	ret = bB_Init();		// initializes IOs, SPI and emBRICK protocol stack
	
	
	
	printf("---------------------------------------\n");
	printf("emBRICK(R), RaspberryBrick Starterkit-1\n");
	printf("BSP and sample application, V0.06\n");
	printf("brickBUS stack version: %02d \n",BRICKBUS_STACK_VERSION);
	printf("(c) 2016 by IMACS GmbH\n");
	printf("---------------------------------------\n");
	printf("Setting of the DIP switch: DI=%1d%1d%1d%1d \n", bcm2835_gpio_lev(RPI_V2_GPIO_P1_15), bcm2835_gpio_lev(RPI_V2_GPIO_P1_13), bcm2835_gpio_lev(RPI_V2_GPIO_P1_12), bcm2835_gpio_lev(RPI_V2_GPIO_P1_11));
	printf("Initialize bus .... \n");
	
	ret += threadInit();
	if (ret == 0)	// When no error, continue
	{		
		// wait until brickBUS is operating
		while(!brickBUS[0].bB_BUS.initialized)
		{
			delay(20);
		}
		printf("successfull\n");	
		// list found modules with details
		modules = bB_getNumModules(1);	
 		printf("module(s) found ... %d\n", modules);
		for (i=1; i <= modules; i++) 
			printf("   module %02d ...... ID:%1d-%03d, Mod-Vers: %02d, Prot-Vers: %02d\n",
					i, bB_getModulID(1,i)/1000, bB_getModulID(1,i)%1000, bB_getModulSwVers(1,i), bB_getModulbBVers(1,i)); 
		
 		printf("\n--- RUN ---\n");	// bus now running
	
		while(1)
		{		
		// Toggle LED1 "sign of life"
			if(i&0x10) 
				bcm2835_gpio_set(RPI_V2_GPIO_P1_16);
			else
				bcm2835_gpio_clr(RPI_V2_GPIO_P1_16);
				
			delay(UPDATE_RATE);	// delay in  ms
			printf("AI1=%04d  AI2=%04d  DI=%1d%1d%1d%1d\r", // Values are shown on the screen
					bB_getWord(MOD1_AI1),					// these functions just read the buffered data, ...
					bB_getWord(MOD1_AI2),					// .. the data exchange is done in a separate thread asynchronsously
					bB_getBit(MOD1_DI1),
					bB_getBit(MOD1_DI2),
					bB_getBit(MOD1_DI3),
					bB_getBit(MOD1_DI4)
					);
			i++;	
		}
	}	
	else	
		printf("Error %d: init not successfull", ret);

	pthread_cancel(thread1);	// kill thread1
	pthread_cancel(thread2);	// kill thread2	
	bB_terminate();				// terminate brickBUS
	return 0;
}

