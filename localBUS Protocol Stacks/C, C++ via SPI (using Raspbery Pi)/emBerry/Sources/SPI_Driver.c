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
*   file SPI_Driver.c	                                        *
*                                                               *
*                                                               *
* Copyright [C] 2013 by											*
*		IMACS GmbH												*
*       Mittelfeldstr. 25, D-70806 Kornwestheim					*
*       Tel.: 07154/8083-0, Fax.: 07154/8083-29					*
*       E-Mail: info@imacs-gmbh.de								*
****************************************************************/
#include <bcm2835.h>
#include <stdio.h>
#include "bBConfigs.h"
#include "bBDefines.h"

/****************************************************************
*FUNCTION NAME:		spiInit()									*
*                                                               *
*DESCRIPTION:		The function initializes spi interface		*
*                                                               *
*PARAMETER:			None										*
*                                                               *
*RETURN:			0-initialization is successfully			*
* 					1-Error: initialization is not successfully	*
*                                                               *
****************************************************************/
int spiInit()
{
	if (!bcm2835_init())
		return -1; 
	
	bcm2835_gpio_fsel(RPI_GPIO_P1_22, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_clr(RPI_GPIO_P1_22);
	bcm2835_gpio_fsel(RPI_GPIO_P1_18, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_clr(RPI_GPIO_P1_18);
	
	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);    // Arrangement of byte transference MSB First. 
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);                 // CPOL=0 (Clock Idle High level), CPHA=1 (SDO transmit/change edge active to idle)
	bcm2835_spi_setClockDivider(2500); 							// 250MHz/2500 = 100kHz 
	   
	return(0);
}
/****************************************************************
*FUNCTION NAME:		spi_Close()									*
*                                                               *
*DESCRIPTION:		The function closes spi	interfase			*
*                                                               *
*PARAMETER:			none										*
*                                                               *
*RETURN:			none					                    *
*                                                               *
****************************************************************/
void spiClose(void)
{	
	bcm2835_spi_end(); 
	bcm2835_close();
}

/****************************************************************
*FUNCTION NAME:		bBDoSPI()									*
*                                                               *
*DESCRIPTION:		The function writes and reads 				*
* 					data from the spi interfase					*
*                                                               *
*PARAMETER:			txbuf- Buffer for send data					*
* 					rxbuf- Buffer for receive data				*
*                                                               *
*RETURN:			always 0				                    *
*                                                               *
****************************************************************/
short bBDoSPI(unsigned char * txbuf, unsigned char * rxbuf, short numOfBytes,short postMessageDelay)
{
	short i ;
	unsigned long calc; 
	uint8_t x;

/*---exchange data-----*/
	for (i=0;i<numOfBytes;i++)
	{
		x = ~(*(txbuf++)) ;
		*(rxbuf++) = bcm2835_spi_transfer(x);
		if (2==i)				// delay between header and data block
			delayMicroseconds (40L) ;
	}
	return 0 ;		
}
