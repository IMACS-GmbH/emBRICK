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

#define delayMS(x)	usleep((x)*1000L)

/*************************** DEFINES ****************************/
#define UPDATE_RATE		50			// ms

// module 1 - 2Rel4Di2Ai (basic module)
#define MOD1_REL1		1,1,0,0		// Node 1, Module 1, Byte 0, Bit 0
#define MOD1_REL2		1,1,0,1		// Node 1, Module 1, Byte 0, Bit 1
#define MOD1_DI1		1,1,4,0		// Node 1, Module 1, Byte 4, Bit 0
#define MOD1_DI2		1,1,4,1		// Node 1, Module 1, Byte 4, Bit 1
#define MOD1_DI3		1,1,4,2		// Node 1, Module 1, Byte 4, Bit 2
#define MOD1_DI4		1,1,4,3		// Node 1, Module 1, Byte 4, Bit 3
#define MOD1_AI1		1,1,2		// Node 1, Module 1, Byte 0
#define MOD1_AI2		1,1,0		// Node 1, Module 1, Byte 2

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
	static int counter;
	char b;
	
	counter++;		
	b = bB_getBit(MOD1_DI2);	// Read DI2
	bB_putBit(MOD1_REL1, b);	// write DI2 to Rel1
	
	bB_putBit(MOD1_REL2, (counter & 0x10) ? 0:1); // Relays switching periodically, set/clear output bit (1,1,0,0) Node 1, Module 1, Byte 0, Bit 0


	printf("AI1=%04d  AI2=%04d  DI=%1d%1d%1d%1d\r", // Values are shown on the screen
				bB_getWord(MOD1_AI1),					// these functions just read the buffered data, ...
				bB_getWord(MOD1_AI2),					// .. the data exchange is done in a separate thread asynchronsously
				bB_getBit(MOD1_DI1),
				bB_getBit(MOD1_DI2),
				bB_getBit(MOD1_DI3),
				bB_getBit(MOD1_DI4)
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



