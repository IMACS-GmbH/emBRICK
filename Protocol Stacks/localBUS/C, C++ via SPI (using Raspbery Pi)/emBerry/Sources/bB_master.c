/****************************************************************
*   file bB_master.c                                            *
*                                                               *
*   brickBUS stack. Might be used within a radCASE-Design but   *
*   also seperately. For standalone use, see notes in           *
*   bBConfigs_template.h. 										*
*   For operation, normally the function bB_Processing()		*
*   must be called periodically.							    *
*   All other functions except for setupbBData() and            *
*   eventually bBGatherInitData() should not be called from		*
*   the application.											*
*   see comments in bBConfigstemplate.h and bBDerfines.h		*
*   how to configure this brickBUS stack.						*
*                                                               *
* Copyright [C] 2007..16 by										*
*		IMACS GmbH												*
*       Mittelfeldstr. 25, D-70806 Kornwestheim					*
*       Tel.: 07154/8083-0, Fax.: 07154/8083-29					*
*       E-Mail: info@imacs-gmbh.de								*
****************************************************************/

/************************* INCLUDES ****************************/
#include <bBConfigs.h>		// this is a project specific file, see bBConfigs_template.h how to create it. It must include also include_all.h when radCASE is used
#include <stdlib.h>
#include <string.h>
#if FLOWMETER !=0
#include "FlowMeter.h"		// radCASE specific stuff
#endif
#if USE_BRICKBUS != 0

/*************************** DEFINES ****************************/

#ifdef RD_REV_NO					// this is a define used exclusivly by radCASE, the following must not be used in non-radCASE applications
#ifndef BBUPDATEAFTERINIT
#define	BBUPDATEAFTERINIT				bB_UpdateAfterInit();			// this is the correct setting for radCASE (by default)
#endif
#endif

/******************* EXTERNAL VARIABLES ************************/

/******************* FUNCTION DECLARATIONS *********************/

#ifdef RD_REV_NO					// this is a define used exclusively by radCASE, the following must not be used in non-radCASE applications
extern void ao_put_bB(E_AO* eao, short phys);
#if FLOWMETER !=0
extern void flowm_set_bB(FLOWPAR * pPar);
#endif
#endif

/********************* GLOBAL VARIABLES ************************/

BRICKBUS brickBUS[MAX_KNOTEN];
#ifdef USE_BB_REMOTE
BB_MASTER bBMaster;
#endif

/*************** BRICKBUS MASTER FUNCTIONS *********************/

/****************************************************************
 * FUNCTION NAME:	setupbBData()								*
 *																*
 * DESCRIPTION:		initializes complete brickBUS[]	 			*
 *					must be called during startup before		*
 *					any other brickBUS function is invoked.		*
 *					in radCASE : before SysInit() is invoked.	*
 *																*
 * PARAMETER:		none										*
 *																*
 * RETURN:			none										*
 *																*
 ***************************************************************/
void setupbBData(void)
{
register int i ;

	// set all members to 0
	memset ((void*)brickBUS,0,sizeof(brickBUS));
	for (i=0;MAX_KNOTEN > i;i++)
	{
		brickBUS[i].bB_BUS.expectedSize = 0xFF ;
		brickBUS[i].bB_BUS.size = MAX_MODULE ;
		brickBUS[i].bB_BUS.lengthTimeout = BB_TIMEOUTFACTOR ;
	}
}


/****************************************************************
 * FUNCTION NAME:	bBGatherInitData()							*
 *																*
 * DESCRIPTION:		gathering initial data			 			*
 *																*
 * PARAMETER:		brickBUS data structure						*
 *																*
 * RETURN:			none										*
 *																*
 ***************************************************************/
#ifdef USE_BB_REMOTE
void bBGatherInitData(BRICKBUS *pbBData)
{
	unsigned short gatherInitCounter = 0;
	//Initialisierungsdaten des Masters in den Buffer schreiben
	pbBData->bBMISOBuffer[0] = pbBData->bB_BUS.size;
	pbBData->bBMISOBuffer[1] = pbBData->boardBUSMaster.bus_status;
	pbBData->bBMISOBuffer[2] = BB_MASTER_ID/256;
	pbBData->bBMISOBuffer[3] = BB_MASTER_ID%256;
	pbBData->bBMISOBuffer[4] = BB_MASTER_SOFTWARE/256;
	pbBData->bBMISOBuffer[5] = BB_MASTER_SOFTWARE%256;
	pbBData->bBMISOBuffer[6] = BB_MASTER_TYPE;
	pbBData->bBMISOBuffer[7] = BB_UART_TYPE/256;
	pbBData->bBMISOBuffer[8] = BB_UART_TYPE%256;

	//Initialisierungsdaten der Slaves
	for( ; gatherInitCounter < pbBData->bB_BUS.size; gatherInitCounter++)
	{
		pbBData->bBMISOBuffer[ (gatherInitCounter * 11) + 1 + 8] = pbBData->bB_Slaves[gatherInitCounter].status;
		pbBData->bBMISOBuffer[ (gatherInitCounter * 11) + 2 + 8] = pbBData->bB_Slaves[gatherInitCounter].datenLaengeIn;
		pbBData->bBMISOBuffer[ (gatherInitCounter * 11) + 3 + 8] = pbBData->bB_Slaves[gatherInitCounter].datenLaengeOut;
		pbBData->bBMISOBuffer[ (gatherInitCounter * 11) + 4 + 8] = pbBData->bB_Slaves[gatherInitCounter].ProtokollVersion;
		pbBData->bBMISOBuffer[ (gatherInitCounter * 11) + 5 + 8] = pbBData->bB_Slaves[gatherInitCounter].HardwareVersion;
		pbBData->bBMISOBuffer[ (gatherInitCounter * 11) + 6 + 8] = (pbBData->bB_Slaves[gatherInitCounter].deviceID) / 256;
		pbBData->bBMISOBuffer[ (gatherInitCounter * 11) + 7 + 8] = (pbBData->bB_Slaves[gatherInitCounter].deviceID) % 256;
		pbBData->bBMISOBuffer[ (gatherInitCounter * 11) + 8 + 8] = (pbBData->bB_Slaves[gatherInitCounter].HerstellerID) / 256;
		pbBData->bBMISOBuffer[ (gatherInitCounter * 11) + 9 + 8] = (pbBData->bB_Slaves[gatherInitCounter].HerstellerID) % 256;
		pbBData->bBMISOBuffer[ (gatherInitCounter * 11) + 10 + 8] = pbBData->bB_Slaves[gatherInitCounter].bB_Data.currentOffsetIn;
		pbBData->bBMISOBuffer[ (gatherInitCounter * 11) + 11 + 8] = pbBData->bB_Slaves[gatherInitCounter].bB_Data.currentOffsetOut;
	}
}
#endif


/****************************************************************
 * FUNCTION NAME:		bB_checksum								*
 *																*
 * DESCRIPTION:			calculate checksum						*
 *																*
 * PARAMETER:			first data for checksum					*
 *						number of bytes							*
 *																*
 * RETURN:				calculated checksum						*
 *																*
 ***************************************************************/
static inline unsigned char bB_checksum(unsigned char * pData, short size)
{
register short i ;
unsigned char result = BRICKBUS_CHECKSUM;

	for (i=0;size>i;i++)
		result -= *(pData++) ;
	return result ;
}

/****************************************************************
 * FUNCTION NAME:		bB_checksumtest							*
 *																*
 * DESCRIPTION:			check checksum							*
 *																*
 * PARAMETER:			first data for checksum					*
 *						number of bytes							*
 *																*
 * RETURN:				1 : checksum ok / 0 : failed			*
 *																*
 ***************************************************************/
static inline short bB_checksumtest(unsigned char * pData, short size)
{
register short i ;
unsigned char result = 0 ;

	for (i=0;size>i;i++)
		result += *(pData++) ;
	return (result==BRICKBUS_CHECKSUM) ;
}

/****************************************************************
 * FUNCTION NAME:		bBResetData()							*
 *																*
 * DESCRIPTION:			Reset of Bus Data for reinit			*
 *																*
 * PARAMETER:			brickBUS data structure					*
 *																*
 * RETURN:				none									*
 *																*
 ***************************************************************/
static inline void bBResetData(BRICKBUS *pbBData)
{
register short i;

	pbBData->bB_BUS.sizecount = 0;
	pbBData->bB_Data.laenge			= 0;
	pbBData->bB_Data.currentOffsetIn = 0;
	pbBData->bB_Data.currentOffsetOut = 0;

	// clear tx Buffer, Rx buffer will be cleared preceeding every transaction
	memset ((void*)(&(pbBData->bB_Data.txData[0])),0,TXRXDATASIZE);

	for( i = 0; i < MAX_MODULE; i++ )
	{
		pbBData->bB_Slaves[i].status             = 0;
		pbBData->bB_Slaves[i].fehlerCounter		 = 0;
		pbBData->bB_Slaves[i].totalfailCounter	 = 0L;
	}
}

/****************************************************************
 * FUNCTION NAME:		InitbBUS()								*
 *																*
 * DESCRIPTION:			state machine for brickBUS init			*
 *						Builds up communication to all bricks	*
 *						sequentially, inititializes these bricks*
 *						and configures internal brickBUS data	*
 *						according to received brick data		*
 *						must be sequentially called while 0		*
 *						is returned								*
 *																*
 * PARAMETER:			node number								*
 *																*
 * RETURN:				>0 : number of bricks on bus			*
 *						0 : Init not finished					*
 *						-1 : Protocol version of at least on	*
 *							brick does not match requrements	*
 *						-3 : to many bricks (>MAX_MODULE)		*
 *							connected to bus					*
 *						-4 : Number of bricks is not as expected*
 *							if brickBUS[node]bB_BUS.expectedSize*
 *							is set								*
 *						-5 : no brick found = bus unusable		*
 *						-6 : Buffers to small					*
 *						-7 = illegal brick response received	*
 *																*
 ***************************************************************/
char InitbBUS(short node)
{
BRICKBUS *pbBData = &brickBUS[node] ;

	switch (pbBData->bB_Data.State)
	{
		case 0 :										// ** first call : prepare **
			SET_BB_TIMING(node,1) ;						// set Timing for Init
			CLOSE_BB_SPI(node);							// close bus
			pbBData->bB_Data.TimeHandle = SetPauseMS(BB_SPI_INIT_TIME) ;	// set time until bB SPI is idle
			pbBData->bB_Data.State = 1 ;									// nxt state
			break ;
		case 1 :										// ** wait for SPI and slaves to be idle, then initialize bus and data **
			if (IsPauseMS(pbBData->bB_Data.TimeHandle))
				break ;
			bBResetData(pbBData);
			INIT_BB_SPI(node);
			BB_SLAVE_SELECT(node);		// activate first slave via daisy chain line
			pbBData->bB_Data.TimeHandle = SetPauseMS(2) ;
			pbBData->bB_Data.State = 2 ;
			break ;
		case 2 :							// ** first step in loop over all modules : prepare Init package **
			if (IsPauseMS(pbBData->bB_Data.TimeHandle))
				break ;

			memset ((void*)(&(pbBData->bB_Data.rxData[0])),0,TXRXDATASIZE);		// clear Rx Buffer

			pbBData->bB_Data.laenge = 13;							// set length
			pbBData->bB_Data.txData[0] = BB_BROADCAST;
			pbBData->bB_Data.txData[1] = BB_INITSLAVE;
			pbBData->bB_Data.txData[2] = bB_checksum(&(pbBData->bB_Data.txData[0]), 2)  ;	// header checksum
			pbBData->bB_Data.txData[3] = ( pbBData->bB_BUS.sizecount + 1 );		// set address of slave
			pbBData->bB_Data.txData[4] = pbBData->bB_BUS.lengthTimeout / 256;	// set Timeout
			pbBData->bB_Data.txData[5] = pbBData->bB_BUS.lengthTimeout % 256;
			// further data currently unused, remains 0
			pbBData->bB_Data.txData[11] = bB_checksum(&(pbBData->bB_Data.txData[3]), 3) ;		// data checksum

			pbBData->bB_Data.fehlVersuche = 0;		//Fehlerdaten im Packet zurücksetzen
			pbBData->bB_Data.State = 3 ;
			// fall thru
		case 3 :							// ** start or restart transmission **
			if (IsPauseMS(pbBData->bB_Data.TimeHandle))
				break ;
			if (DATA_BUS_IDLE(node))
			{
				if (!START_DATA_XCHANGE(node))
					pbBData->bB_Data.State = 4 ;
				else
				{
					//>>>>> Error handling ???? <<<<<
				}
			}
			break ;
		case 4 :							// ** wait for transmission finished and evaluate received package **
			if (DATA_XCHANGE_ACTIVE(node))	
				break ;						// not yet finished

			if (!bB_checksumtest(&(pbBData->bB_Data.rxData[3]), 10))					// incorrect checksum received package : failed
			{
				if (pbBData->bB_Data.fehlVersuche >= BRICKBUS_MAX_RETRIES)		// already tried to often ?
				{
					if (pbBData->bB_Data.rxData[4] | 
						pbBData->bB_Data.rxData[5] | 
						pbBData->bB_Data.rxData[6] | 
						pbBData->bB_Data.rxData[7] | 
						pbBData->bB_Data.rxData[8] | 
						pbBData->bB_Data.rxData[9] | 
						pbBData->bB_Data.rxData[10] | 
						pbBData->bB_Data.rxData[11])		// if not all recieved bytes are 0, some module responded with a wrong message
					{
						return -7 ;
					}
					else									// there is no module at that address
					{
						if (!(pbBData->bB_BUS.sizecount))					// no module found
							return -5;
						if( (pbBData->bB_BUS.sizecount != pbBData->bB_BUS.expectedSize) && ( pbBData->bB_BUS.expectedSize != 0xFF) )		// not the expected size
							return -4;
						else
						{
							pbBData->bB_BUS.size = pbBData->bB_BUS.sizecount ;
							pbBData->bB_Data.TimeHandle = SetPauseMS(BB_INITSYNCHGAP_MS) ;	// set init synch gap time
							pbBData->bB_Data.State = 8 ;
						}
					}
				}
				else				// we do have another retry
				{
					pbBData->bB_Data.fehlVersuche++;
					memset ((void*)(&(pbBData->bB_Data.rxData[0])),0,TXRXDATASIZE);		// clear Rx Buffer
					pbBData->bB_Data.TimeHandle = SetPauseMS(BB_INITSYNCHGAP_MS) ;
					pbBData->bB_Data.State = 3 ;									//  restart transmission on nxt cycle 
				}
			}
			else								// checksum correct, no error
			{
				if (pbBData->bB_BUS.sizecount < MAX_MODULE)				// otherwise number of legal modules exceeded
				{
					pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].status = pbBData->bB_Data.rxData[3];		// save status

					// save received data properly
					pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].ProtokollVersion		=   pbBData->bB_Data.rxData[4];
					pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].HardwareVersion		=   pbBData->bB_Data.rxData[5];
					pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].deviceID				=   (pbBData->bB_Data.rxData[7] << 8) | pbBData->bB_Data.rxData[6];
					pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].HerstellerID			=   (pbBData->bB_Data.rxData[9] << 8) | pbBData->bB_Data.rxData[8];
					pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].datenLaengeIn			=   pbBData->bB_Data.rxData[10];
					pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].datenLaengeOut		=   pbBData->bB_Data.rxData[11];
					pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].offsetIn				=   pbBData->bB_Data.currentOffsetIn;
					pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].offsetOut				=   pbBData->bB_Data.currentOffsetOut;
					//advance Offsetcounter
					if ((pbBData->bB_Data.currentOffsetIn + pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].datenLaengeIn) <= sizeof (pbBData->bBMOSIBuffer))
						pbBData->bB_Data.currentOffsetIn += pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].datenLaengeIn;
					else
					{
						pbBData->BufferSizeError |= 1 ;
						return -6 ;								// Buffer size exceeded by slaves
					}
					if ((pbBData->bB_Data.currentOffsetOut + pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].datenLaengeOut)  <= sizeof (pbBData->bBMISOBuffer))
						pbBData->bB_Data.currentOffsetOut += pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].datenLaengeOut;
					else
					{
						pbBData->BufferSizeError |= 2 ;
						return -6 ;								// Buffer size exceeded by slaves
					}
					if ((pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].ProtokollVersion < BRICKBUS_PROT_VERSION_MIN) || 
						(pbBData->bB_Slaves[pbBData->bB_BUS.sizecount].ProtokollVersion > BRICKBUS_PROT_VERSION_MAX))
						return -1 ;								// module has incompatible protocol version

					pbBData->bB_Data.TimeHandle = SetPauseMS(BB_INITSYNCHGAP_MS) ;	// set init synch gap time
					pbBData->bB_Data.State = 5 ;
				}
				else																		// number of legal modules exceeded
					return -3;
			}
			if (pbBData->bB_Data.State != 5)
				break ;
			// else fall thru
		case 5 :				// ** wait init synch gap, then  select nxt slave and start transmission ***
			if (IsPauseMS(pbBData->bB_Data.TimeHandle))
				break ;

			pbBData->bB_Data.laenge = 5;								// only header data to be xchanged (transmitted)
			pbBData->bB_Data.txData[0] = pbBData->bB_BUS.sizecount + 1;	// actual slave address
			pbBData->bB_Data.txData[1] = BB_NEXTSLAVE;
			pbBData->bB_Data.txData[2] = bB_checksum(&(pbBData->bB_Data.txData[0]), 2)  ;	// header checksum
			pbBData->bB_Data.txData[3] = 0;															// dummy body
			pbBData->bB_Data.txData[4] = bB_checksum(&(pbBData->bB_Data.txData[3]), 1)  ;	// body checksum
			pbBData->bB_Data.fehlVersuche = 0;		//Fehlerdaten im Packet zurücksetzen
			pbBData->bB_Data.State = 6 ;
			// fall thru
		case 6 :							// ** start or restart transmission **
			if (IsPauseMS(pbBData->bB_Data.TimeHandle))
				break ;

			if (DATA_BUS_IDLE(node))
			{
				if (!START_DATA_XCHANGE(node))
					pbBData->bB_Data.State = 7 ;
				else
				{
					//>>>>> Error handling ???? <<<<<
				}
			}
			break ;
		case 7 :							// ** wait for transmission finished and evaluate received package **
			if (DATA_XCHANGE_ACTIVE(node))	
				break ;						// not yet finished
			if (!bB_checksumtest(&(pbBData->bB_Data.rxData[3]), 2))					// incorrect checksum received package : failed
			{	
				if (pbBData->bB_Data.fehlVersuche >= BRICKBUS_MAX_RETRIES)		// already tried to often ?
					return -1;		//	return Error
				else				// we do have another retry
				{
					pbBData->bB_Data.fehlVersuche++;
					memset ((void*)(&(pbBData->bB_Data.rxData[0])),0,TXRXDATASIZE);		// clear Rx Buffer
					pbBData->bB_Data.TimeHandle = SetPauseMS(BB_INITSYNCHGAP_MS) ;	// set init synch gap time
					pbBData->bB_Data.State = 6 ;									//  restart transmission
				}
			}
			else								// checksum correct, no error
			{
				pbBData->bB_BUS.sizecount++;		// advance to next module
				pbBData->bB_Data.TimeHandle = SetPauseMS(BB_INITSYNCHGAP_MS) ;
				pbBData->bB_Data.State = 2 ;
			}
			break ;
		case 8 :							// ** final synch gap **
			if (IsPauseMS(pbBData->bB_Data.TimeHandle))
				break ;

			SET_BB_TIMING(node,0) ;					// set Timing for Operation
			pbBData->bB_BUS.initialized = 1;		// bus now properly initialized
			return pbBData->bB_BUS.size;			// everything ok, return Bus size
			break ;
	}
	return 0;			// still busy
}

/****************************************************************
 * FUNCTION NAME:		bBSendData()							*
 *																*
 * DESCRIPTION:			state machine for brickBUS communication*
 *						sequentially exchanges data with all	*
 *						bricks									*
 *																*
 * PARAMETER:			node number								*
 *																*
 * RETURN:				none									*
 *																*
 ***************************************************************/
void bBSendData(short node)
{
register short			i ;
BRICKBUS *pbBData = &brickBUS[node] ;	

	switch( pbBData->bB_Data.State )
	{
		case 0 :		// ** Begin of a transmision cycle **
			BBTESTHIGH(node) ;
#ifdef USE_BB_REMOTE
			if(0==pbBData->bB_BUS.dataWaitingForTransmission)	// start only if new data available
				break ;
			pbBData->bB_BUS.dataWaitingForTransmission = 0;
#endif
			pbBData->bB_Data.currentSlave = 0;
			pbBData->bB_Data.currentOffsetIn = 0;
			pbBData->bB_Data.currentOffsetOut = 0;
			pbBData->bB_Data.State = 1 ;
			pbBData->bB_BUS.transactionfailed = 0 ;
			// fall thru
		case 1 :		// ** Begin Data exchange with next slave **
			pbBData->bB_Data.fehlVersuche = 0;
			pbBData->bB_Data.State = 2 ;
			// fall thru
		case 2 :		// ** Start Next Data exchange **
			if (DATA_BUS_IDLE(node))
			{
				// clear Buffer
				memset ((void*)pbBData->bB_Data.txData,0,TXRXDATASIZE);
				memset ((void*)pbBData->bB_Data.rxData,0,TXRXDATASIZE);
				pbBData->bB_Data.laenge = (max(pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].datenLaengeOut, pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].datenLaengeIn))+4 ;

				//prepare Packet
				pbBData->bB_Data.txData[0] = pbBData->bB_Data.currentSlave+1;
				pbBData->bB_Data.txData[1] = BB_DATAMSG;
				pbBData->bB_Data.txData[2] = bB_checksum(&(pbBData->bB_Data.txData[0]), 2)  ;	// header checksum

				for( i = 0; i < ( pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].datenLaengeIn ); i++)
					pbBData->bB_Data.txData[i+3] = pbBData->bBMOSIBuffer[pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].offsetIn+i];						// fill data section

				pbBData->bB_Data.txData[pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].datenLaengeIn+3] = 
								bB_checksum(	&(pbBData->bB_Data.txData[3]), 
												pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].datenLaengeIn)  ;	// data checksum

				if (START_DATA_XCHANGE_AND_TIMEOUT(node))
				{
					// >>>>> error handling <<<<<
				}
				pbBData->bB_Data.State = 3 ;
			}
			break ;
		case 3 :							// ** wait for transmission finished and evaluate received package **
			if (DATA_XCHANGE_ACTIVE(node))	
				break ;						// not yet finished

			// Check whick size is bigger, save result in "i" to save RAM
			if(pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].datenLaengeOut > pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].datenLaengeIn)
				i = pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].datenLaengeOut;
			else
				i = pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].datenLaengeIn;
			if (!bB_checksumtest(&(pbBData->bB_Data.rxData[3]),i+1))					// incorrect checksum received package : failed
			{
				pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].totalfailCounter++ ;
				//EmpfangsBuffer für dieses Modul nicht füllen, eventuell sind alte Werte noch Ok
				if( pbBData->bB_Data.fehlVersuche >= BRICKBUS_MAX_RETRIES )								// to many retries ?
				//Abbruch des Versuchs Datenaustausch mit diesem Modul zu führen
				{
					pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].status = 100;				//Fehlerfall im Status speichern
					if ((++pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].fehlerCounter) > BB_FAILIURELIMIT)		// Fehler mitzaehlen, wenn zuviele, Neu-Init
					{
						pbBData->bB_BUS.initialized = 0 ;
						pbBData->bB_Data.State = 0 ;
					}
					else 
						pbBData->bB_Data.State = 4 ;
				}
				else //wenn die maximale Anzahl der Fehlversuche noch nicht erreicht wurde
				{
					//Anzahl Fehlversuche erhöhen
					pbBData->bB_Data.fehlVersuche++;
					//Rücksetzen und Vorbereiten auf nächste Übertragung
 					pbBData->bB_Data.State = 2 ;
				}
			}
			else
			/********** Checksumme nicht fehlerhaft **************/
			{
				//Status des Moduls abspeichern
				pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].status = pbBData->bB_Data.rxData[3];
				//Nutzdaten abspeichern
				//DISABLE_GLOBAL_INTERRUPT;
				for( i = 1; i < pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].datenLaengeOut; i++ )
				{
					pbBData->bBMISOBuffer[ ( i + pbBData->bB_Data.currentOffsetOut - 1) ] = pbBData->bB_Data.rxData[i+3];
				}
				//ENABLE_GLOBAL_INTERRUPT;
				pbBData->bB_Data.State = 4 ;
				pbBData->bB_Slaves[pbBData->bB_Data.currentSlave].fehlerCounter = 0 ;			// Fehler mitzaehlen
			}
			if (4!=pbBData->bB_Data.State)
				break ;
			// else fall thru
		case 4 :							// ** switch to next slave **
			//Offsets mitzählen
			pbBData->bB_Data.currentOffsetOut += pbBData->bB_Slaves[ pbBData->bB_Data.currentSlave ].datenLaengeOut;
			pbBData->bB_Data.currentOffsetIn += pbBData->bB_Slaves[ pbBData->bB_Data.currentSlave ].datenLaengeIn;

			if(++pbBData->bB_Data.currentSlave >= pbBData->bB_BUS.size)			// finished, all slaves requested
			{
//				pbBData->bB_Data.currentSlave = 0 ;	>>> done in case 0
				pbBData->bB_Data.State = 0;
				BBTESTLOW(node) ;
			}
			else
				pbBData->bB_Data.State = 1;
			break ;
	}
}

/****************************************************************
*FUNCTION NAME:		bB_UpdateAfterInit()	                    *
*                                                               *
*DESCRIPTION:		After brickBUS was initialized,				*
*                   set MOSI Buffer according to actual         *
*					settings                                    *
*					this function is only usable in a radCASE   *
*					environment                                 *
*                                                               *
*PARAMETER:			none				                        *
*                                                               *
*RETURN:			none				                        *
*                                                               *
****************************************************************/
#ifdef RD_REV_NO					// this is a define used exclusively by radCASE, the following must not be used in non-radCASE applications
void bB_UpdateAfterInit(void)
{
	register short i ;

	// set all DO's
	for (i = 0; i < NumEdo; i++)
	{
		if (tab_e_do[i].act)
			do_exe_set(&tab_e_do[i]);
		else
			do_exe_res(&tab_e_do[i]);
	}

	// set all AO's
	for (i = 0; i < NumEao; i++)
	{
		ao_put_bB(&tab_e_ao[i],tab_e_ao[i].physact) ;
	}
#if FLOWMETER !=0
	// set all Flowmeter
	for (i = 0; i < FLOWMETER; i++)
	{
		flowm_set_bB(&FlowPars[i]) ;
	}
#endif
}
#endif

/****************************************************************
 * FUNCTION NAME:		bB_Processing							*
 *																*
 * DESCRIPTION:			main processing function of brickBUS	*
 *						This function is called every 			*
 *						1..10 ms								*
 *						Function initializes the Bus and decides*
 *						when to start the next Updatecycle.		*
 *						Also responsible for the handling of	*
 *						errors that may be triggered.			*
 *																*
 * PARAMETER:			None									*
 *																*
 * RETURN:				None									*
 *																*
 ***************************************************************/
void bB_Processing (void) 
{
register short i ;
char initreturn ;

	for (i=0; MAX_KNOTEN > i;i++)			// over all nodes do
	{
		if (!brickBUS[i].bB_BUS.initialized)	// if not initialized, do the Bus init
		{
			initreturn = InitbBUS(i) ;
			if (0!=initreturn)			// one Init cycle finished 
			{
				brickBUS[i].bB_Data.State = 0 ;
				if (0>initreturn)			// InitBUS returned an error message
				{
					brickBUS[i].bB_BUS.LastInitFail = initreturn;
					brickBUS[i].bB_BUS.NumOfFailedInits++ ;
				}
				else
				{
					brickBUS[i].bB_BUS.NumOfSucceededInits++ ;
#ifdef RD_REV_NO					// this is a define used exclusively by radCASE, the following must not be used in non-radCASE applications
					BBUPDATEAFTERINIT
#endif
				}
			}
		}
		else
			bBSendData(i);
	}
}	

#endif