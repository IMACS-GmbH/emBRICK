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
#include <stdio.h>
#include "brickbus.h"
#include "bBConfigs.h"

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <linux/spi/spidev.h>

#if USE_BRICKBUS != 0

/*************************** DEFINES ****************************/
#define BRICKBUS_PRIORITY           89

/******************* EXTERNAL VARIABLES ************************/


/******************* FUNCTION DECLARATIONS *********************/

int bB_SetPauseMS(unsigned long zeit);
int bB_StartDataXChange(void);
void bB_SlaveDeSelect(void);
void bB_SlaveSelect(void);
void bB_Close(void);
void bB_setTransferSpeed(unsigned long bitrate);
unsigned long bB_getTransferTime(void);
void initBrickBus(void);
int bB_IsPauseMS(int handle);

/********************* GLOBAL VARIABLES ************************/
static int fdEmbrick = -1;
static int fd_do_spics;


static unsigned long g_bus_speed = 250000; // 250kHz


// Momentan wird nur die Speicherung von einer Pausenzeit unterstuetzt
static struct timeval g_ti_PauseStart;
static unsigned long g_tPauseMS;

/***************************************************************/

/****************************************************************
*FUNCTION NAME:		scheduleStack()        		                *
*                                                               *
*DESCRIPTION:		Task for running brickbus communication		*
*                                                               *
*PARAMETER:         none										*
*                                                               *
*RETURN:			none										*
*                                                               *
****************************************************************/
void* scheduleStack(void *arg)
{
	int i;
	struct sched_param param;

	param.sched_priority = BRICKBUS_PRIORITY;
	pthread_setschedparam(pthread_self(), SCHED_RR, &param);
	
	for (;;)
	{
		bB_Processing();
				
		usleep(BB_SYNCHGAP);
	}
}


//-- Configure GPIO funtion --
// no:  GPIO-Number
// dir: 0=Input, 1=Output
int gpio_config(int no, int dir)
{
	int exportfd, directionfd, valuefd, n;
	char buf[10], szDir[40], szValue[40];
	
	if (no < 10)
		{
		buf[0] = '0' + no;
		buf[1] = 0;
		n = 2;
		}
	else if (no < 100)
		{
		buf[0] = '0' + no / 10;
		buf[1] = '0' + no % 10;
		buf[2] = 0;
		n = 3;
		}
	else if (no < 132)
		{
		buf[0] = '0' + no / 100;
		buf[1] = '0' + (no%100) / 10;
		buf[2] = '0' + no % 10;
		buf[3] = 0;
		n = 4;
		}
	else
		return -1;
	
	exportfd = open("/sys/class/gpio/export", O_WRONLY);
	if (exportfd < 0)
		printf("Cannot open GPIO%d to export it\n", no);
	
	write(exportfd, buf, n);
	close(exportfd);
	
	strcpy(szDir, "/sys/class/gpio/gpio");
	strcat(szDir, buf);
	strcat(szDir, "/direction");
	directionfd = open(szDir, O_RDWR);
	if (directionfd < 0)
	   printf("Cannot open GPIO%d direction it\n", no);
	
	if (dir == 0) // Input
		write(directionfd, "low", 3);
	else // Output
		write(directionfd, "high", 4);
	close(directionfd);

	strcpy(szValue, "/sys/class/gpio/gpio");
	strcat(szValue, buf);
	strcat(szValue, "/value");
	printf("%s\n", szValue);
	valuefd = open(szValue, O_RDWR);
	if (valuefd < 0)
		printf("Cannot open GPIO%d for value\n", no);

	return valuefd;
}

int gpio_set(int valuefd, int val)
{
	lseek(valuefd, 0, SEEK_SET);
	
	if (val)
		write(valuefd, "1", 2);
	else
		write(valuefd, "0", 2);

	return 0;
}

int gpio_get(int valuefd)
{
	char buf[5];
	
	lseek(valuefd, 0, SEEK_SET);
	read(valuefd, buf, 2);
				
	if (buf[0] == '1')
		return 1;

	return 0;
}

/****************************************************************
*FUNCTION NAME:		initBrickBus()        		                *
*                                                               *
*DESCRIPTION:		Initializes communication over BrickBus		*
*                                                               *
*PARAMETER:         none										*
*                                                               *
*RETURN:			none										*
*                                                               *
****************************************************************/
void initBrickBus(void)
{
	struct spi_ioc_transfer tr;
	int ret;
	unsigned long speed;
	unsigned char DummyByte[1] = { 0 };
	unsigned char mode, bits;
	
	fd_do_spics = gpio_config(49, 1);
	
	fdEmbrick = open("/dev/spidev1.0", O_RDWR);
	if (fdEmbrick < 0)
	{
		fprintf(stderr, "Cannot initialize SPI-Embrick: %d\n", fdEmbrick);
		exit(-1);
	}

	// emBRICK deselect
	bB_SlaveDeSelect();

	// spi mode:
	//	SPI_CPOL=clock polarity, idle high if this is set
	//	SPI_CPHA=clock phase, sample on trailing edge if this is set
	mode = SPI_CPHA; // invertierte Einstellungen wegen invertiertem Ausgang?
	ret = ioctl(fdEmbrick, SPI_IOC_WR_MODE, &mode);
	if (ret == -1) printf("can't set spi mode\n");
	ret = ioctl(fdEmbrick, SPI_IOC_RD_MODE, &mode);
	if (ret == -1) printf("can't get spi mode\n");

	// Dummy-Byte ausgeben, damit CLK auf HIGH geht, bevor die richtige Kommunikation losgeht
	tr.tx_buf = (unsigned long)DummyByte;
	tr.rx_buf = (unsigned long)DummyByte;
	tr.len    = 1;
	tr.bits_per_word = 8;
	tr.cs_change     = 0;
	tr.delay_usecs   = 0;
	tr.speed_hz      = g_bus_speed * 2;
	tr.pad           = 0;
	ret = ioctl(fdEmbrick, SPI_IOC_MESSAGE(1), &tr);

	int lsb_setting = 0; // => MSB first
	ret = ioctl(fdEmbrick, SPI_IOC_WR_LSB_FIRST, &lsb_setting);
	if (ret == -1) printf("can't set lsb_setting\n");
	ret = ioctl(fdEmbrick, SPI_IOC_RD_LSB_FIRST, &lsb_setting);
	if (ret == -1) printf("can't set lsb_setting\n");

	bits = 8;
	// bits per word
	ret = ioctl(fdEmbrick, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1) printf("can't set bits per word\n");
	ret = ioctl(fdEmbrick, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1) printf("can't get bits per word\n");
	
	// max speed hz
	speed = g_bus_speed * 2;
	ret = ioctl(fdEmbrick, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1) printf("can't set max speed hz\n");
	ret = ioctl(fdEmbrick, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1) printf("can't get max speed hz\n");
}

// Time for 1 byte transfer in µs. Normally this is 8 divided by the Bus frequency, in case of a delay between the bytes this delay must be added. 
// In cases using more than 1 node with different speed, define BB_BYTETIME for the slwliest bus. Values for faster busses may then be overwritten by the application.
unsigned long bB_getTransferTime(void)
{
	// Zeitdauer fuer 1 Byte[µs] = 8 * 1 / Bitrate[Hz] * 1000000
	return (8000000 / (g_bus_speed));
}

// The complete Timing of the brickBUS must be made by the application. Since Timings are different
// for Init and normal opreation, timing can be switched here.
// In case of low bus speed or when different timing is not implemented, this macro can be left empty.
void bB_setTransferSpeed(unsigned long bitrate)
{
	int ret;
	unsigned long speed;
	
	g_bus_speed = bitrate;
	speed = bitrate * 2;
	
	// * max speed hz
	ret = ioctl(fdEmbrick, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't set max speed hz");
	
	ret = ioctl(fdEmbrick, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't get max speed hz");

	return;
}

void bB_SlaveSelect(void)
{
	gpio_set(fd_do_spics, 1);
}

void bB_SlaveDeSelect(void)
{
	gpio_set(fd_do_spics, 0);
}

void bB_Close(void)
{
	bB_SlaveDeSelect();
}

/*
 * Function:            Triggers the data exchange on the bB
 *
 * Notes:               The Buffers brickBUS[node].bB_Data.txData and brickBUS[node].bB_Data.rxData 
 *                      remain unaffected after this macro is invoked until DATA_XCHANGE_ACTIVE returns 0
 *
 * Input:               the node affected
 *
 * Output:              0 if transmission started correctly
 */
int bB_StartDataXChange(void)
{
	int i, ret;
	struct spi_ioc_transfer tr;
	unsigned char ucInvTxBuf[300];

	// Ausgangsdaten invertieren
	for(i = 0; i < brickBUS[0].bB_Data.laenge; i++)
		ucInvTxBuf[i] = ~brickBUS[0].bB_Data.txData[i];

#if 0
	// hier explizite Pause zwischen Header und Datenteil (->Header und Datenteil aufteilen)
	tr.tx_buf = (unsigned long)ucInvTxBuf;
	tr.rx_buf = (unsigned long)brickBUS[0].bB_Data.rxData;
	tr.len    = 3; // 3 Headerbytes
	tr.bits_per_word = 8;
	tr.cs_change     = 0;
	tr.delay_usecs   = 0;
	tr.speed_hz      = g_bus_speed * 2;
	tr.pad           = 0;
	ret = ioctl(fdEmbrick, SPI_IOC_MESSAGE(1), &tr);

	tr.tx_buf = (unsigned long)ucInvTxBuf+3;
	tr.rx_buf = (unsigned long)brickBUS[0].bB_Data.rxData+3;
	tr.len    = brickBUS[0].bB_Data.laenge - 3; // Daten
	tr.bits_per_word = 8;
	tr.cs_change     = 0;
	tr.delay_usecs   = 0;
	tr.speed_hz      = g_bus_speed * 2;
	tr.pad           = 0;
	ret = ioctl(fdEmbrick, SPI_IOC_MESSAGE(1), &tr);
#else
	// hier keine explizite Pause zwischen Header und Datenteil,
	// alles auf einmal senden
	tr.tx_buf = (unsigned long)ucInvTxBuf;
	tr.rx_buf = (unsigned long)brickBUS[0].bB_Data.rxData;
	tr.len    = brickBUS[0].bB_Data.laenge;
	tr.bits_per_word = 8;
	tr.cs_change     = 0;
	tr.delay_usecs   = 0;
	tr.speed_hz      = g_bus_speed * 2;
	tr.pad           = 0;
	ret = ioctl(fdEmbrick, SPI_IOC_MESSAGE(1), &tr);

#endif

	if (ret < 0)
		{
		printf("eB-SPI write error\n");
		return -1;
		}

	return 0;
}

int bB_IsPauseMS(int handle)
{
	long ti_timediff;
	struct timeval ti_curtime;
	
	// akt. Systemzeit holen
	gettimeofday(&ti_curtime, NULL);
	// und davon die PauseStart-Systemzeit abziehen
	ti_timediff = ((ti_curtime.tv_sec - g_ti_PauseStart.tv_sec)*1000)+(ti_curtime.tv_usec-g_ti_PauseStart.tv_usec)/1000;
	// Differenz > als Wartezeit
	if (ti_timediff >= g_tPauseMS)
		return 0; // dann ist die Zeit abgelaufen
	else
		return 1; // Pausezeit ist noch am ablaufen
}

int bB_SetPauseMS(unsigned long zeit)
{
	// Wartezeit und akt. Systemzeit speichern
	g_tPauseMS = zeit;
	gettimeofday(&g_ti_PauseStart, NULL);
				
	return 0;
}

void bB_UpdateAfterInit(void)
{
}

#endif // USE_BRICKBUS
