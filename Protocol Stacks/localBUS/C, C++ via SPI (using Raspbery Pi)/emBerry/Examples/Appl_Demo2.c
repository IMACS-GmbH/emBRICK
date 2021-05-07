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
    /*
	while (1)
	{
		bB_Appl();
		delay(50);		//Delay 50ms
	}
     */
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

	unsigned char toggle_switch, test=0;
	static unsigned char led=0, led_counter=0, relay=0, relay_counter=0, relay_button=0, button_pressed=0, alternate=0, next_led=0;
	unsigned short  poti_value;
	short temp_value, temp_diff;

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
 		    toggle_switch = bB_getBit(1,1,0,0);

 		    temp_value = bB_getWord(1,2,14);

 		    poti_value = bB_getWord(1,2,0);

 		    relay_button = bB_getBit(1,3,2,1);	// Read DI1

 		    //detect button click
 		    if (relay_button)
 		      {
 			if (!button_pressed)
 			  {
 			    button_pressed=1;
 			    alternate = !alternate;
 			  }
 		      }
 		    else
 		      {
 			button_pressed=0;
 		      }

 		    if (alternate)
 		      {
 			//temp_diff = ( (temp_value)-650 < 0 ? 0 : (temp_value)-650 );

 			if (temp_value-700 > 0)
 			  temp_diff = temp_value-700;
 			else
 			  temp_diff = 0;
 			//if ( relay_counter > (((1023-temp_value)/50)*0.5)) //((1023-temp_value+100)*(1023-temp_value+100)/2500)-10000)
 			if ( relay_counter > ((50-temp_diff)/2.5))
 			  {
			    bB_putBit(1,3,0,relay,0);
			    relay++;
			    if (relay > 5)
			      relay=0;
			    bB_putBit(1,3,0,relay,1);
			    relay_counter=0;
 			  }
 		      }

 		    if (toggle_switch)
 		    {
 			if ((1023-poti_value)<100 && led_counter > 1)
 			  {
 			   next_led = 1; test=1;
 			  }
 			else if ((1023-poti_value)>=100 && (1023-poti_value)<200 && led_counter > 2)
 			  {
 			    next_led = 1; test=2;
 			  }
 			else if ((1023-poti_value)>=200 && (1023-poti_value)<300 && led_counter > 3)
 			  {
			    next_led = 1; test=3;
 			  }
 			else if ((1023-poti_value)>=300 && (1023-poti_value)<400 && led_counter > 4)
 			  {
			    next_led = 1; test=4;
 			  }
 			else if ((1023-poti_value)>=400 && (1023-poti_value)<500 && led_counter > 5)
 			  {
			    next_led = 1; test=5;
 			  }
 			else if ((1023-poti_value)>=500 && (1023-poti_value)<600 && led_counter > 6)
 			  {
			    next_led = 1; test=6;
 			  }
 			else if ((1023-poti_value)>=600 && (1023-poti_value)<700 && led_counter > 7)
 			  {
			    next_led = 1; test=7;
 			  }
 			else if ((1023-poti_value)>=700 && (1023-poti_value)<800 && led_counter > 8)
 			  {
			    next_led = 1; test=8;
 			  }
 			else if ((1023-poti_value)>=800 && (1023-poti_value)<900 && led_counter > 9)
 			  {
			    next_led = 1; test=9;
 			  }
 			else if ((1023-poti_value)>=900 && led_counter > 10)
 			  {
			    next_led = 1; test=10;
 			  }

 			if (next_led)
 			//if ( (led_counter) > ((1023-poti_value)/50))
 			{
			    bB_putBit(1,1,0,led,0);
			    led++;
			    if (led > 7)
			      led=0;
			    bB_putBit(1,1,0,led,1);
			    led_counter=0;
			    next_led=0;
 			}
 		    }




 		    printf("toggle switch: %1d, temperature: %04d, poti: %04d, relay button %1d, temp diff %d\r", toggle_switch, temp_value, poti_value, relay_button, temp_diff);
 		    //printf("\r");

 		    led_counter++;
 		    relay_counter++;
 		    delay(UPDATE_RATE);
 		}


 		/*
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
		*/
	}	
	else	
		printf("Error %d: init not successfull", ret);

	pthread_cancel(thread1);	// kill thread1
	pthread_cancel(thread2);	// kill thread2	
	bB_terminate();				// terminate brickBUS
	return 0;
}

