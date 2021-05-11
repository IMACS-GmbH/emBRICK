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
#include <math.h>

#define delayMS(x)	usleep((x)*1000L)

/*************************** DEFINES ****************************/
#define UPDATE_RATE		50			// ms

// module 1 - 2Rel4Di2Ai (basic module)
#define MOD1_REL1		1,3,0,0		// Node 1, Module 1, Byte 0, Bit 0
#define MOD1_REL2		1,3,0,1		// Node 1, Module 1, Byte 0, Bit 1
#define MOD1_DI1		1,3,4,0		// Node 1, Module 1, Byte 4, Bit 0
#define MOD1_DI2		1,3,4,1		// Node 1, Module 1, Byte 4, Bit 1
#define MOD1_DI3		1,3,4,2		// Node 1, Module 1, Byte 4, Bit 2
#define MOD1_DI4		1,3,4,3		// Node 1, Module 1, Byte 4, Bit 3
#define MOD1_AI1		1,3,0		// Node 1, Module 1, Byte 0
#define MOD1_AI2		1,3,2		// Node 1, Module 1, Byte 2

// module 2 - 4DimLedI
#define MOD2_LED1		1,1,0		//
#define MOD2_LED2		1,1,6		//

// module 3 - 6Ai2Temp
#define MOD3_LDR		1,2,14		//
#define MOTION			1,2,10

// module 4
#define LED_LINE		1,4,0

/******************* EXTERNAL VARIABLES ************************/

/******************* FUNCTION DECLARATIONS *********************/

/********************* GLOBAL VARIABLES ************************/

/**************** DEMO FUNCTIONS ***************************/

/****************************************************************
*FUNCTION NAME:		bB_Appl()																			*
*                                                               *
*DESCRIPTION:		Demo function switching the relais and					*
*								copying an input bit to an output bit	 					*
*								This function is being called periodically.			*
*								Adjust UPDATE_RATE to your needs								*
*PARAMETER:			None																						*
*RETURN:				None					                    							*
****************************************************************/
void bB_Appl()
{
	static int counter=0, rel2_counter=0;
	static unsigned char relay_switch=0, button_pressed=0, alternate=0, relay_onoff=0, rel2_active=0;
	unsigned short poti_resistance=700, motion_value;
	static unsigned short ldr_value;

	static unsigned short ldr_values[10] = {0,0,0,0,0,0,0,0,0,0};
	static char ldr_counter=0;
	static unsigned char led_line=0;

	int i;

	counter++;
	
	poti_resistance = bB_getWord(MOD1_AI2); //nicht getestet

	motion_value = bB_getWord(MOTION);

	if (counter >= poti_resistance/128)
	{
	    relay_onoff = !relay_onoff;
	    counter = 0;
	}

	relay_switch = bB_getBit(MOD1_DI1);	// Read DI1
	
	//detect button click
	if (relay_switch)
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
	    bB_putBit(MOD1_REL1, relay_onoff); // Relays switching periodically
	}


	//activate Relay 2 for 0.5 sec for poti values below 300 or above 500
	rel2_counter++;
	if (!rel2_active)
	  {
	    if ((motion_value < 300)||(motion_value > 500))
	      {
		bB_putBit(MOD1_REL2, 1);
		rel2_active = 1;
		rel2_counter = 0;
	      }
	  }
	else if (rel2_active)
	  {
	    if (rel2_counter > 10)
	      {
		bB_putBit(MOD1_REL2, 0);
		if ((motion_value > 300)&&(motion_value < 500))
		  {
		    rel2_active = 0;
		  }
	      }
	  }


	//LED2 as function of the poti resistance. Brightness ~ resistance^2
	bB_putWord(MOD2_LED2,(((734-poti_resistance)*(734-poti_resistance))*0.005>3000) ? 3000 : ((734-poti_resistance)*(734-poti_resistance))*0.005);


	//LED1 dependent on LDR
	ldr_values[(++ldr_counter > 9) ? ldr_counter = 0 : ldr_counter] = bB_getWord(MOD3_LDR);
	//ldr_value = bB_getWord(MOD3_LDR);


	for(i=0; i<10; i++)
	  {
	    ldr_value += ldr_values[i];
	  }
	ldr_value /= 10;

	bB_putWord(MOD2_LED1, (ldr_value*ldr_value)*0.003>3000 ? 3000 : (ldr_value*ldr_value)*0.003); //flackert weil kein MotiLDR-Modul


	//moving leds
	bB_putByte(LED_LINE,led_line++);

	printf("button=%1d poti=%04d LDR=%04d motion=%04d\r", // Values are shown on the screen
				bB_getBit(MOD1_DI1),
				poti_resistance,
				ldr_value,
				motion_value
				);


}

/****************************************************************
*FUNCTION NAME:		main()																				*
*                                                               *
*DESCRIPTION:		main function, initializes the Bus,							*
*								then starts mainloop														*
*PARAMETER:			standard params, not used herein								*
*RETURN:			always 0																					*
****************************************************************/
int main(int argc, char **argv)
{
	// start brickBUS
	while(bB_Start()!=0);
	
	// Enter mainloop
	while(1)
	{			
		// make sure bus is initialized, otherwise reinitialize
		if(bB_getNumModules(1))
		{
			delayMS(UPDATE_RATE);	// delay in  ms
			
			// call Appplication periodically
			bB_Appl();
			
		} else
		{
			// reinitialize
			while(bB_Start()!=0);
		}
	}
	return 0;
}



