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
* FILE:				brickbus.c									*
*																*
*                   functions for communicating with brickBUS   *
*                                                               *
*                                                               *
* Copyright [C] 2014 by											*
*		IMACS GmbH												*
*       Mittelfeldstr. 25, D-70806 Kornwestheim					*
*       Tel.: 07154/8083-0, Fax.: 07154/8083-29					*
*       E-Mail: info@imacs-gmbh.de								*
****************************************************************/

/************************* INCLUDES ****************************/
#include "brickbus.h"
#include "bBConfigs.h"
#include <Arduino.h>
#include <SPI.h>

#if USE_BRICKBUS != 0

/*************************** DEFINES ****************************/
#define CS_PIN 		10
#define WAIT_SHORT  2   // us
#define WAIT_LONG   20  // us
#define WAIT_DATA   113 // us

/******************* EXTERNAL VARIABLES ************************/


/******************* FUNCTION DECLARATIONS *********************/


/********************* GLOBAL VARIABLES ************************/
static unsigned long ul_bus_speed = 1000000; // 1MHz

// werden zur Berechnung der Pausen benötigt
unsigned long ul_prevTime;
unsigned long ul_currentTime;
unsigned long ul_tPauseMS;

/****************************************************************
*FUNCTION NAME:		bB_getTransferTime()       		            *
*                                                               *
*DESCRIPTION:		Time for 1 byte transfer in µs. Normally 	*
*					this is 8 divided by the Bus frequency, in 	*
*					case of a delay between the bytes this 		*
*					delay must be added.						*
*					In cases using more than 1 node with 		*
*					different speed, define BB_BYTETIME for the *
*					slowlest bus. Values for faster busses may 	*
*					then be overwritten by the application.		*
*                                                               *
*PARAMETER:         none										*
*                                                               *
*RETURN:			none										*
*                                                               *
****************************************************************/
unsigned long bB_getTransferTime(void)
{
	// Zeitdauer fuer 1 Byte[µs] = 8 * 1 / Bitrate[Hz] * 1000000
	return (8000000 / (ul_bus_speed));
}

/****************************************************************
*FUNCTION NAME:		bb_setTiming()        			            *
*                                                               *
*DESCRIPTION:		The complete Timing of the brickBUS must be *
*					made by the application. Since Timings are 	*
*					different for Init and normal opreation, 	*
*					timing can be switched here.				*
*					In case of low bus speed or when different 	*
*					timing is not implemented, this macro can	*
*					be left empty.								*
*                                                               *
*PARAMETER:         none										*
*                                                               *
*RETURN:			none										*
*                                                               *
****************************************************************/
void bb_setTiming(unsigned long bitrate)
{	
	ul_bus_speed = bitrate;

	SPI.begin();
	SPI.beginTransaction(SPISettings(ul_bus_speed, MSBFIRST, SPI_MODE1));	
}

/****************************************************************
*FUNCTION NAME:		bB_SlaveSelect()        		            *
*                                                               *
*DESCRIPTION:		connect CS PIN and set it to HIGH			*
*                                                               *
*PARAMETER:         none										*
*                                                               *
*RETURN:			none										*
*                                                               *
****************************************************************/

void bB_SlaveSelect(void)
{
	pinMode(CS_PIN, OUTPUT);
	digitalWrite(CS_PIN, HIGH);
}

/****************************************************************
*FUNCTION NAME:		bB_SlaveDeSelect()    		                *
*                                                               *
*DESCRIPTION:		set CS PIN to LOW							*
*                                                               *
*PARAMETER:         none										*
*                                                               *
*RETURN:			none										*
*                                                               *
****************************************************************/

void bB_SlaveDeSelect(void)
{
	pinMode(CS_PIN, OUTPUT);
	digitalWrite(CS_PIN, LOW);
}

/****************************************************************
*FUNCTION NAME:		bB_Close()	        		                *
*                                                               *
*DESCRIPTION:		deselect BrickBus							*
*                                                               *
*PARAMETER:         none										*
*                                                               *
*RETURN:			none										*
*                                                               *
****************************************************************/

void bB_Close(void)
{
	bB_SlaveDeSelect();
}

/****************************************************************
 * FUNCTION NAME:	bB_StartDataXChange(void)					*
 * 																*
 * FUNCTION:		Triggers the data exchange on the bB		*
 *																*
 * DESCRIPTION:		The Buffers brickBUS[node].bB_Data.txData 	*
 * 					and brickBUS[node].bB_Data.rxData remain 	*
 * 					unaffected after this macro is invoked 		*
 * 					until DATA_XCHANGE_ACTIVE returns 0			*
 *																*
 * PARAMETER:		the node affected							*
 *																*
 * RETURN:		    0 if transmission started correctly			*
 ***************************************************************/
int bB_StartDataXChange(void)
{
	short i;
	unsigned char ucInvTxBuf[300];

	// Ausgangsdaten invertieren
	for(i = 0; i < brickBUS[0].bB_Data.laenge; i++)
		ucInvTxBuf[i] = ~brickBUS[0].bB_Data.txData[i];

  	for(i = 0; i < brickBUS[0].bB_Data.laenge; i++)
  	{
  		brickBUS[0].bB_Data.rxData[i] = SPI.transfer(ucInvTxBuf[i]);
    	delayMicroseconds(WAIT_SHORT);
  		if(i == 2)
    		delayMicroseconds(WAIT_LONG);
  	}

  	// Hier expliziete Pause zwei Datenpacketen
  	if(brickBUS[0].bB_BUS.initialized)
    	delayMicroseconds(WAIT_DATA);

	return 0;
}


/****************************************************************
*FUNCTION NAME:		bB_IsPauseMS()        		                *
*                                                               *
*DESCRIPTION:		erstellt Zeitdifferenz 						*
*                                                               *
*PARAMETER:         ul_timediff									*
*                                                               *
*RETURN:			0: Pausenzeit ist abgelaufen				*
*					1: Pausenzeit ist noch am ablaufen			*
*                                                               *
****************************************************************/
int bB_IsPauseMS(int handle)
{
	unsigned long ul_timediff;
  
	// akt. Systemzeit holen
	ul_prevTime = millis();
	// und davon die PauseStart-Systemzeit abziehen
	ul_timediff = ul_currentTime - ul_prevTime;
	// Differenz > als Wartezeit
	if (ul_timediff >= ul_tPauseMS)
		return 0; // dann ist die Zeit abgelaufen
	else
		return 1; // Pausezeit ist noch am ablaufen
}

/****************************************************************
*FUNCTION NAME:		bB_SetPauseMS()        		                *
*                                                               *
*DESCRIPTION:		setzt Pausenzeit							*
*                                                               *
*PARAMETER:         none										*
*                                                               *
*RETURN:			0: Zeiten gesetzt							*
*					1: Zeiten nicht gesetzt						*
*                                                               *
****************************************************************/

int bB_SetPauseMS(unsigned long zeit)
{
	// Wartezeit und akt. Systemzeit speichern
	ul_tPauseMS = zeit;
	ul_currentTime = millis();

	if(ul_tPauseMS && ul_currentTime != 0)			
		return 0;
	else
		return 1;
}

void bB_UpdateAfterInit(void)
{
}

#endif // USE_BRICKBUS
