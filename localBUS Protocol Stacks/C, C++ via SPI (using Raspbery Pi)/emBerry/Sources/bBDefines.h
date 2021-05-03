/* 
 * File:   bBDefines.h
 * Author: Jörg
 *
 * Created on 30. August 2012, 10:07
 */
#if USE_BRICKBUS != 0
#ifndef BRICKBUSDEFINES_H
#define	BRICKBUSDEFINES_H

// Version Number range of the bB Protocol. Only Slaves having one of these Protocol Versions are compatible
#define BRICKBUS_PROT_VERSION_MIN	11
#define BRICKBUS_PROT_VERSION_MAX	11
// Version Number of the bB Stack
#define BRICKBUS_STACK_VERSION	4

//****************************************************************************
//*********************** SPI-Definitionen************************************
//****************************************************************************

#define BRICKBUS_MAX_RETRIES	5						// maximale Anzahl an Versuchen ein Packet neu zu verschicken, nachdem die Checksumme fehlerhaft war
#define BRICKBUS_CHECKSUM		0x55					//Das Checksummenbyte wird so angepasst, dass alle Bytes zusammengezählt diesen Wert ergeben (Bei Master zB. AdressByte+Kommandobyte+Checksummenbyte=BRICKBUS_CHECKSUM
#define BB_BROADCAST			0						//ID unter der alle Slaves zu erreichen sind

// IDs der jeweiligen Befehle
#define BB_INITSLAVE			2
#define BB_NEXTSLAVE			3
#define BB_DATAMSG				10

// min Time the Bus must be closed and the daisy chain line must be inactive in ms. Value is a guess.
#ifndef BB_SPI_INIT_TIME
#define	BB_SPI_INIT_TIME		500
#endif

// synchgap during Init in ms. By default it is twice the SynchGap transferred to the Modules
#ifndef BB_INITSYNCHGAP_MS
#define BB_INITSYNCHGAP_MS		((BB_SYNCHGAP * 2) / 1000) + 1 
#endif

#ifndef BB_FAILIURELIMIT
#define BB_FAILIURELIMIT		10			// permitted number of consecutive bus errors before bus is reinitialized
#endif

#ifndef BB_TIMEOUTFACTOR
#define	BB_TIMEOUTFACTOR		(((BB_SYNCHGAP - 10) /36) + 1)		// Timoutfactor as transferred via bB
#endif

#if (BB_SYNCHGAP < 90)
#error "BB_SYNCHGAP to low, must be >= 90"
#endif

#if (BB_SYNCHGAP > 50000)
#error "BB_SYNCHGAP to high, must be <= 50000"
#endif

//****************************************************************************
//*********************** Dimensions *****************************************
//****************************************************************************
#ifndef MAX_MODULE_MESSAGE_LEN
#define MAX_MODULE_MESSAGE_LEN	32			// max number of bytes per Module in message
#endif
#ifndef MAX_MODULE
#define MAX_MODULE 32						// max number of (possible) modules
#endif
#ifndef MAX_KNOTEN
#define MAX_KNOTEN 1						// max number of nodes supported
#endif

#if (7>MAX_MODULE_MESSAGE_LEN)
#define TXRXDATASIZE	10								// size of Buffer for Data exchange (minimum size for Init string)
#else
#define TXRXDATASIZE	MAX_MODULE_MESSAGE_LEN+4		// size of Buffer for Data exchange
#endif

// Enhancement : could defines different MAX_MODULE_MESSAGE_LEN's for MOSI and MISO and thus define different Default Buffer sizes
#ifndef MISOBUFFER_SIZE
#define MISOBUFFER_SIZE (MAX_MODULE_MESSAGE_LEN*MAX_MODULE)
#endif
#ifndef MOSIBUFFER_SIZE
#define MOSIBUFFER_SIZE (MAX_MODULE_MESSAGE_LEN*MAX_MODULE)
#endif

//****************************************************************************
//*********************** Structure defines **********************************
//****************************************************************************
// a packet and its data
typedef struct brickBUS_Data{
	unsigned char	laenge;							// length of actual data package (max of Rx and Tx)
    char			fehlVersuche;					// counts retries on wrong checksum
	short			State;							// State variable for State machines in brickBUS processing
	long			TimeHandle ;					// Timer Handle used by macro SetPauseMS and IsPauseMS
	unsigned char	currentSlave ;					// currently addressed slave
	unsigned char	currentOffsetIn ;				// current Data Offset in bBMOSIBuffer
	unsigned char	currentOffsetOut ;				// current Data Offset in bBMISOBuffer
    unsigned char	txData[TXRXDATASIZE];			// data buffer for Data to Slave
    unsigned char	rxData[TXRXDATASIZE];			// data buffer for Data from Slave
}BB_DATA;

//	Data of a single Module on the bB
typedef struct brickBUS_Slave{  
    unsigned char status;							// current status of Slave
    unsigned char datenLaengeIn;					// Number of bytes transmitted to slave
    unsigned char datenLaengeOut;					// Number of bytes received from slave
    unsigned char ProtokollVersion;					// information received from Slave
    unsigned char HardwareVersion;					// information received from Slave
    unsigned short  deviceID;						// information received from Slave
    unsigned short  HerstellerID;					// information received from Slave
    unsigned char offsetIn;							// offset of data of this slave in bBMOSIBuffer
    unsigned char offsetOut;						// offset of data of this slave in bBMISOBuffer
	unsigned short	fehlerCounter ;					// internal counter, counts failed transactions
	unsigned long	totalfailCounter ;				// counts all failed transactions since last BUSInit (for debugging purposes)
}BB_SLAVE;

// Description of the Bus
typedef struct brickBUS_Bus{     
    unsigned char	initialized;						// Bus is initialized, i.e. all Slaves have their address and the system is operational
    unsigned char	size;								// then number of modules on bus
    unsigned char	sizecount;							// counting then number of modules on bus during init
    unsigned char	expectedSize;						// the application may set a required bus size, Default is 0xFF = undefined
	unsigned char	transactionfailed;					// set during cycle (requesting all slaves) if a transaction failed
    unsigned char	lengthTimeout;						// Timeout - Factor : effective Timeout is (Factor + 1) * 30us + 16us
	char			LastInitFail ;						// last error returned by InitbBUS(). Set to 0 while no error ever has occured
	unsigned short	NumOfSucceededInits ;				// Number of successfull Inits since PowerUp, can be sused for debugging purposes
	unsigned short	NumOfFailedInits ;					// Number of failed Inits since PowerUp, can be sused for debugging purposes
    unsigned char	dataWaitingForTransmission;			// data from remote Master to be transmitted (unused in case no Remote Master)
}BB_BUS;

// Remote Master status Data (preliminary, currently not yet supported)
typedef struct brickBUS_Master{  
    char bus_status;
    unsigned char status[7];
    unsigned char txData[56];
    unsigned char rxData[56];
}BB_MASTER;

typedef struct brickBUS
{
    unsigned char	bBMOSIBuffer[MOSIBUFFER_SIZE];
    unsigned char	bBMISOBuffer[MISOBUFFER_SIZE];
	unsigned char	BufferSizeError;					// bit 1 : sizeof bBMOSIBuffer insufficient / bit 2 :  sizeof bBMISOBuffer insufficient
    BB_BUS			bB_BUS;
    BB_SLAVE		bB_Slaves[MAX_MODULE];
    BB_DATA			bB_Data;
    BB_MASTER *		pbB_Master;							// the Descriptor of the Remote Master, NULL if without Remote Master
}BRICKBUS;


//****************************************************************************
//*********************** externals ******************************************
//****************************************************************************

extern BRICKBUS brickBUS[MAX_KNOTEN];		

extern void setupbBData(void);
extern void bB_Processing (void);

#endif	// BRICKBUSDEFINES_H
#endif // USE_BRICKBUS