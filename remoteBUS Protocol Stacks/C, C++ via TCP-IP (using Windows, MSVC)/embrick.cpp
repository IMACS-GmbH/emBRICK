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

#include "stdafx.h"
#include "embrick.h"

#include <Windows.h>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

#pragma comment (lib,"Ws2_32.lib")

/** \def SIZE_TX_BUFFER
 * Size of buffers for sending data
 */
#define SIZE_TX_BUFFER			500

/** \def SIZE_TX_BUFFER
 * Size of buffers for sending data
 */
#define SIZE_MASTERDATA			64

/** \def SIZE_RX_BUFFER
 * Size of buffers for receiving data
 */
#define SIZE_RX_BUFFER			512

/** \def MAX_TXBYTES_V5PLUS
 * Maximum number of bytes for sending in protocol version 5 or higher
 */
#define MAX_TXBYTES_V5PLUS		SIZE_TX_BUFFER

/** \def MAX_RXBYTES_V5PLUS
 * Maximum number of bytes for receiving in protocol version 5 or higher
 */
#define MAX_RXBYTES_V5PLUS		SIZE_RX_BUFFER

/** \def MAX_TXBYTES_V4MINUS
 * Maximum number of bytes for sending in protocol version 4 or lower
 */
#define MAX_TXBYTES_V4MINUS		60

/** \def MAX_RXBYTES_V4MINUS
 * Maximum number of bytes for receiving in protocol version 4 or lower
 */
#define MAX_RXBYTES_V4MINUS		256

/** \def EB_NODEINFO
 * Command for requesting node info
 */
#define EB_NODEINFO				0x02
/** \def EB_RESETBUS
 * Command for performing a reset on the embrickBUS
 */
#define EB_RESETBUS				0x03
/** \def EB_SETPERMISSION
 * Command for setting permission level (for future use)
 */
#define EB_SETPERMISSION		0x04
/** \def EB_EXCHANGEDATA
 * Command for requesting data exchange (data is sent with telegram and sending of data from node is triggered
 */
#define EB_EXCHANGEDATA			0x10
/** \def EB_CLOSECONNECTION
 * Command for closing the connection to the node
 */
#define EB_CLOSECONNECTION		0xFE

/** \brief Structure of an emBRICK telegram
 */
typedef struct tEmBrickTelegram
{
	unsigned short len; // length of data to send
	unsigned char cmd; // Command to send
	unsigned char reserved1;	// reserved for later use
	unsigned char reserved2;
	unsigned char reserved3;
	unsigned char data[SIZE_RX_BUFFER]; // Buffer for data to send
} emBrickTelegram;

/** \brief Structure of an emBRICK master */
typedef struct tEmBrickMaster {
	unsigned short id; // ID of master
	unsigned short version;	// Softwareversion of Master
	unsigned char txData[SIZE_MASTERDATA];
	unsigned char rxData[SIZE_MASTERDATA];
} emBrickMaster;

/** \brief Structure of an emBRICK slave */
typedef struct tEmBrickSlave {
	unsigned short dev_id; // Device ID of slave
	unsigned char offsetIn; // Offset of input data in buffer the slave receives
	unsigned char offsetOut; // Offset of output data in global node buffer the slave sends
	unsigned char dataLengthIn; // Length of data the slave receives
	unsigned char dataLengthOut; // Length of data the slave sends
	unsigned char hardwareVersion; // Hardware version of slave
	unsigned char protocolVersion; // Protocol version of slave
} emBrickSlave;

/** \brief Structure for a connection to an emBRICK node */
typedef struct tEmBrickConnection {
	std::string ip_address; // IP address of node
	SOCKET sock; // Socket of connection
	unsigned char health; // Health of connection
	unsigned char rxData[SIZE_RX_BUFFER]; // Buffer for receiving data
	unsigned char txData[SIZE_TX_BUFFER]; // Buffer for sending data
	unsigned short rxDataSize; // Maximum number of bytes the local master sends
	unsigned short txDataSize; // Maximum number of bytes the local master can receive
	HANDLE comMutex; // Mutex for synchronizing access to receive/send buffer
	bool connectionEstablished; // Flag which is set on receiving data
	unsigned char ethProtVersion;	// Ethernet Protocol Version of Node
} emBrickConnection;

/** \brief Structure of an emBRICK node (here: node = local master = gateway) */
typedef struct tEmBrickNode {
	unsigned char index; // Index of node
	emBrickMaster master; // Master of node
	std::vector<emBrickSlave> slaves; // Slaves of node
	emBrickConnection connection; // Connection to node
} emBrickNode;

/** \brief Maximum number of allowed nodes */
static const int MAX_NODES = 32;
/** \brief Port which is used for emBRICK communication */
static const unsigned short emBrickPort = 7086;

/** \brief Event which is triggered for exiting the emBRICK task */
static HANDLE exitEmBrickEvent = NULL;
/** \brief Task handle for emBRICK task (for cleanup) */
static HANDLE emBrickTaskID = NULL;
/** \brief Flag to detect if bus is initialized */
static HANDLE busAvailableMutex = NULL;
/** \brief Flag to detect if the Mutex can be used */
bool busAvailableMutexInitialized=false;
/** \brief Contains all emBRICK nodes */
static std::vector<emBrickNode> emBrickNodes;

/** \brief Establishes a connection to an embrick node
 *
 * \param[in,out] connection The connection to establish
 * \return true if successfull
 */
static bool establishConnection(emBrickConnection* connection)
{
	SOCKADDR_IN addr;
	unsigned long enable; 
	
	// Initialize socket
	connection->sock = socket(AF_INET, SOCK_STREAM, 0);
	if (connection->sock == INVALID_SOCKET)
	{
		return false;
	}

	// Initialize address information
	memset(&addr, 0, sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(emBrickPort);
	addr.sin_addr.s_addr = inet_addr(connection->ip_address.c_str());

	// Configure as non blocking socket
	enable = 1;
	ioctlsocket(connection->sock, FIONBIO, &enable);

	// Connect socket
	connect(connection->sock, (SOCKADDR*)&addr, sizeof(SOCKADDR));

	// Start with full connection health, if connection can't be established
	// health will go to zero and cause reconnect
	connection->health = 50;

	// No data received for connection
	connection->connectionEstablished = false;

	return true;
}

/*****************************************************************
 * Funktion:    Stelle fest ob Kommando-Byte gültig ist
 *            
 * Input:       Kommando
 * Output:
 * Return:      positiv:    ja
 *              0:          nein
 *******************************************************************/
BOOL CommandIsValid(unsigned char cmd)
{
	// List with all allowed commands
	static const unsigned char cmdList[] = {EB_NODEINFO, EB_EXCHANGEDATA, EB_CLOSECONNECTION};

	// Search through the List if cmd matches one of its elements
	for(unsigned int i = 0; i < sizeof(cmdList); i++)
	{
		if(cmd == cmdList[i])
			return 1;	// Yes it does
	}
	return 0;	// No it doesn't
}
/*****************************************************************
 * Funktion:    Empfange LWCS-Protokoll Daten
 *            
 * Input:       Socket, Pointer auf Telegram struktur
 * Output:
 * Return:      positiv:    Empfangene Bytes ohne Header
 *              negativ:    -1: Socket ungültig
 *                          -2: Header ungültig
 *              0:          nicht implementiert
 *******************************************************************/
signed short ReceiveData(SOCKET mySocket, emBrickTelegram* rxData)
{
	int rxResult;	
	unsigned long enable = 1;
	// Check Socket
	if(mySocket == INVALID_SOCKET)
		return -1;
	// Other Checks come here
	
	// Make Socket non-blocking
	ioctlsocket(mySocket, FIONBIO, &enable);
	// Receive Header
	rxResult = recv(mySocket, (char*)rxData, 6, 0);
	// Check received Data
	if(rxResult != 6)	// valid amount data received?
		return -2;

	if(CommandIsValid(rxData->cmd))
		rxResult = recv(mySocket, (char*)rxData->data, rxData->len, 0);	// Right command received, read the remaining Data
	else
	{
		while( (recv(mySocket, (char*)rxData->data, 6, 0)) > 0);		// Wrong command received, purge all Data
		return -3;
	}
	return rxResult;
}

/*****************************************************************
 * Funktion:    Sende LWCS-Protokoll Daten
 *            
 * Input:       Socket, Pointer auf Telegram struktur
 * Output:
 * Return:      positiv:    Gesendete Bytes
 *              negativ:    -1: Socket ungültig
 *                          -2: Kommando ungültig
 *              0:          nicht implementiert
 *******************************************************************/
signed short TransmitData(SOCKET mySocket, emBrickTelegram* txData)
{
	int txResult;
	unsigned long enable = 1;
	// Check Socket
	if(mySocket == INVALID_SOCKET)
		return -1;
	// Other Checks come here
	if(!CommandIsValid(txData->cmd))
		return -2;

	// Make Socket non-blocking
	ioctlsocket(mySocket, FIONBIO, &enable);
	// Send Data
	txResult = send(mySocket, (char*)txData, txData->len + 6, 0);

	return txResult;
}

/** \brief Ends a connection to an embrick node
 *
 * \param[in] connection The connection to close
 */
static void endConnection(emBrickConnection* connection)
{
	if (connection->sock != INVALID_SOCKET)
	{
		emBrickTelegram telegram;
		telegram.cmd = EB_CLOSECONNECTION;
		telegram.len = 0;
		telegram.reserved1 = 0;
		telegram.reserved2 = 0;
		telegram.reserved3 = 0;
		TransmitData(connection->sock, &telegram);
		closesocket(connection->sock);
	}
}


/** \brief Function for exchanging data with the emBRICK nodes
 */
static void dataExchange(void)
{
	emBrickTelegram rxData;
	std::vector<emBrickNode>::iterator itNode;
	// Receive data from all nodes
	for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
	{
		if (WaitForSingleObject(itNode->connection.comMutex, INFINITE) == WAIT_OBJECT_0)
		{
			if (itNode->connection.sock != INVALID_SOCKET)
			{
				short receivedBytes = ReceiveData(itNode->connection.sock, &rxData);

				if(rxData.cmd == 0x10)
				{
					static unsigned int masterOffset;
					// get Offset for Master-Data
					memcpy(&masterOffset, rxData.data, 2);

					if(masterOffset > 2 && masterOffset < (SIZE_TX_BUFFER-2) )
					{
						// load Bus Data into rxBuffer
						memcpy(itNode->connection.rxData, &(rxData.data[2]), masterOffset );
						// load Master Data into Master Data Buffer
						memcpy(itNode->master.rxData, &(rxData.data[masterOffset]), receivedBytes - 6 - masterOffset);
					}

					// Evaluate Connection
					if (itNode->connection.health < 50)
						itNode->connection.health++;
				}
				else
				{
					if (itNode->connection.health > 0)
						itNode->connection.health--;

					if (!itNode->connection.health)
					{
						// Reconnect
						endConnection(&itNode->connection);
						establishConnection(&itNode->connection);
					}
				}
			}
			else
			{
				establishConnection(&itNode->connection);
			}
			ReleaseMutex(itNode->connection.comMutex);
		}
	}
	// Send data to all nodes
	for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
	{
		if (WaitForSingleObject(itNode->connection.comMutex, INFINITE) == WAIT_OBJECT_0)
		{
			emBrickTelegram telegram;
			telegram.cmd = EB_EXCHANGEDATA;
			telegram.len = itNode->connection.txDataSize;
			telegram.reserved1 = 0;
			telegram.reserved2 = 0;
			telegram.reserved3 = 0;
			// load Busdata
			memcpy(&telegram.data[2], itNode->connection.txData, SIZE_TX_BUFFER - SIZE_MASTERDATA);
			// load Master-Offset
			unsigned int masterOffset = SIZE_TX_BUFFER - SIZE_MASTERDATA;
			memcpy(&telegram.data[0], &masterOffset, 2);
			//load Master-Data
			memcpy(&telegram.data[SIZE_TX_BUFFER - SIZE_MASTERDATA], &itNode->master.txData[0], SIZE_MASTERDATA);

			// Send Data
			TransmitData(itNode->connection.sock, &telegram);

			ReleaseMutex(itNode->connection.comMutex);
		}
	}
}

/** \brief Task for emBRICK data communication
 *
 * \param[in] lpParam Pointer to updaterate (allocated with new short and will be deleted in this thread
 * \return Exit code of thread
 */
static DWORD WINAPI emBrickTask(LPVOID lpParam)
{
	short updaterate = *static_cast<short*>(lpParam);
	delete reinterpret_cast<short*>(lpParam);

	std::vector<emBrickNode>::iterator itNode;
	for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
	{
		for(unsigned int i = 0; i < SIZE_MASTERDATA; i++)
		{
			itNode->master.txData[i] = 0;
			itNode->master.rxData[i] = 0;
		}
		// sizes depend on protocoll version
		if(itNode->master.version <= 4)
		{
			itNode->connection.rxDataSize = MAX_RXBYTES_V4MINUS;
			itNode->connection.txDataSize = MAX_TXBYTES_V4MINUS;
		}
		else
		{
			itNode->connection.rxDataSize = MAX_RXBYTES_V5PLUS;
			itNode->connection.txDataSize = MAX_TXBYTES_V5PLUS;
		}
	}

	for (;;)
	{
		dataExchange();
		if (WaitForSingleObject(exitEmBrickEvent, updaterate) != WAIT_TIMEOUT)
		{
			return 0;
		}
	}
	return 1;
}

/** \brief Returns path of executable
 *
 * \return Path of executable
 */
static std::wstring getExePath(void)
{
	DWORD bufSize = MAX_PATH/2;
	DWORD numRead;
	std::wstring ret=L"";
	wchar_t *path;

	do
	{
		bufSize *= 2;
		path = new wchar_t[bufSize+1];
		numRead = GetModuleFileName(NULL, path, bufSize);
		path[numRead] = 0;
		ret = path;
		delete[] path;
	}
	while (bufSize <= numRead);

	ret.erase(ret.find_last_of(L"\\"));

	return ret;
}

/** \brief Trims a string
 *
 * \param[in,out] str String to trim
 */
static void trim(std::string& str)
{
	size_t pos = str.find_last_not_of(" \t");
	if(pos != std::string::npos)
	{
		str.erase(pos + 1);
		pos = str.find_first_not_of(" \t");
		if(pos != std::string::npos)
			str.erase(0, pos);
	}
	else
	{
		str.erase(str.begin(), str.end());
	}
}

unsigned short AddNode(std::string ip)
{
	emBrickNode node;

	// Check if IP is valid
	if((inet_addr(ip.c_str()) == INADDR_NONE) || ((ip.c_str()) == INADDR_ANY))
		return 0;

	// Set index to next index
	node.index = emBrickNodes.size() + 1;

	// Set IP
	node.connection.ip_address = ip;

	// Initialize mutex & buffers
	node.connection.comMutex = CreateMutex(NULL, FALSE, NULL);
	memset(node.connection.rxData, 0, sizeof(node.connection.rxData));
	memset(node.connection.txData, 0, sizeof(node.connection.txData));

	// Push node into list
	emBrickNodes.push_back(node);

	// Return with index
	return node.index;
}

/** \brief Initializes communication for all nodes
 *
 * \return true on Success
 */
static bool emBrickInitCommunication(void)
{
	std::vector<emBrickNode>::iterator itNode;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2,0), &wsa) != 0)
	{
		return false;
	}

	for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
	{
		establishConnection(&(itNode->connection));
	}
	return true;
}

/** \brief Initializes the bus by reading out information from the nodes
 */
static void emBrickInitBus(void)
{
	std::wstring filename;
	FILE* fp;
	std::vector<emBrickNode>::iterator itNode;
	std::stringstream configLog;
	configLog.str("");
	emBrickTelegram rxData;

	// Send command for getting node info to every node
	for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
	{
		if (itNode->connection.sock != INVALID_SOCKET)
		{
			emBrickTelegram telegram;
			telegram.cmd = EB_NODEINFO;
			telegram.len = 6;
			telegram.reserved1 = 0;
			telegram.reserved2 = 0;
			telegram.reserved3 = 0;
			TransmitData(itNode->connection.sock, &telegram);
		}
	}

	// Give masters some time to answer
	Sleep(20);

	// Receive data and output specifications
	for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
	{
		if (itNode->connection.sock != INVALID_SOCKET)
		{
			unsigned char i;
			unsigned char moduleCount=0;			// Number of slave modules found
			unsigned short masterID=0;				// eB-Id of local master
			unsigned char ethProtocolVersion=0;     // Ethernet protocol version
			unsigned char manufacturerID=0;
			unsigned short periphery=0;
			emBrickSlave *slaveIDs=NULL;
			unsigned char masterDataSizeMISO=0;
			unsigned char masterDataSizeMOSI=0;

			// Receive bus data
			if (WaitForSingleObject(itNode->connection.comMutex, INFINITE) == WAIT_OBJECT_0)
			{
				signed short rxResult = ReceiveData(itNode->connection.sock, &rxData);
				if( (rxResult > 0) && (rxData.cmd == EB_NODEINFO))
				{
					moduleCount = rxData.data[0];
					masterID = (rxData.data[2] << 8) + rxData.data[3];
					itNode->master.version = rxData.data[5];
					ethProtocolVersion = rxData.data[4];
					manufacturerID = rxData.data[6];
					periphery = (rxData.data[7] << 8) + rxData.data[8];
					if (moduleCount > 0)
					{
						slaveIDs = new emBrickSlave[moduleCount];
						for (i=0; i<moduleCount; i++)
						{
							slaveIDs[i].dev_id =          (rxData.data[(i*11)+14] << 8) + rxData.data[(i*11)+15];
							slaveIDs[i].protocolVersion = rxData.data[(i*11)+12];
							slaveIDs[i].hardwareVersion = rxData.data[(i*11)+13];
							slaveIDs[i].dataLengthIn =    rxData.data[(i*11)+10];
							slaveIDs[i].dataLengthOut =   rxData.data[(i*11)+11];
							slaveIDs[i].offsetIn =        rxData.data[(i*11)+18];
							slaveIDs[i].offsetOut =       rxData.data[(i*11)+19];
						}
					}
				}
				ReleaseMutex(itNode->connection.comMutex);
			}

			// Check bus data against configuration (and correct configuration)
			if (moduleCount != itNode->slaves.size())
			{ 
				// if less modules expected delete those that are unxepected
				while (moduleCount < itNode->slaves.size())
				{
					itNode->slaves.pop_back();
				}
				// if more modules are expected create some empty modules
				while (moduleCount > itNode->slaves.size())
				{
					emBrickSlave slave;
					slave.dev_id = 0;
					itNode->slaves.push_back(slave);
				}
			}
			if (masterID != itNode->master.id)
			{
				itNode->master.id = masterID;
			}
			
			itNode->connection.ethProtVersion = ethProtocolVersion;
			if (slaveIDs)
			{
				for (i=0; i<moduleCount; i++)
				{
					itNode->slaves.at(i).dev_id = slaveIDs[i].dev_id;
					itNode->slaves.at(i).offsetIn = slaveIDs[i].offsetIn;
					itNode->slaves.at(i).offsetOut = slaveIDs[i].offsetOut;
					itNode->slaves.at(i).dataLengthIn = slaveIDs[i].dataLengthIn;
					itNode->slaves.at(i).dataLengthOut = slaveIDs[i].dataLengthOut;
					itNode->slaves.at(i).hardwareVersion = slaveIDs[i].hardwareVersion;
					itNode->slaves.at(i).protocolVersion = slaveIDs[i].protocolVersion;
				}
				delete[] slaveIDs;
			}

			// Generate entries of current configuration
			configLog << static_cast<unsigned int>(itNode->index) << ": ";
			configLog << itNode->connection.ip_address;
			configLog << ", " << static_cast<unsigned int>(itNode->master.id);

			for (i=0; i<itNode->slaves.size(); i++)
			{
				configLog << ", " << static_cast<unsigned int>(itNode->slaves.at(i).dev_id);
			}
			configLog << "\n";
		}
	}

	filename = getExePath();
	filename += L"\\datei.txt";

	_wfopen_s(&fp, filename.c_str(), L"w");
	
	// Write log contents to logfile
	if (fp)
	{
		fputs(configLog.str().c_str(), fp);
		fclose(fp);
	}
}

char bB_Init(std::vector<std::string> ipList)
{
	exitEmBrickEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	busAvailableMutex = CreateMutex(NULL, TRUE, NULL); // Initially owned
	
	for (int pos = 0; pos < ipList.size(); pos++)
	{
		if(!AddNode(ipList.at(pos)))
			return 0;
	}
	if (!emBrickInitCommunication())
	{
		CloseHandle(busAvailableMutex);
		return 0;
	}
	busAvailableMutexInitialized = true;
	Sleep(100);			// wait a little for Nodes to be ready
	emBrickInitBus();
	ReleaseMutex(busAvailableMutex);
	return 1;
}

short bB_Start(short updaterate)
{
	short * param = new short; // Will be deleted in thread
	*param = updaterate;
	if (!busAvailableMutexInitialized)
		return 0;
	if ((emBrickTaskID = CreateThread(NULL, 0, emBrickTask, param, 0, NULL)) == NULL)
		return 0;
	return 1;
}

char bB_GetConfNumModules(char node)
{
	char ret = 0;
	if ((busAvailableMutexInitialized) && (WaitForSingleObject(busAvailableMutex, 1) == WAIT_OBJECT_0))
	{
		// In the unlikely event that the first check is passed, then a task switch sets the
		// busAvailableMutexInitialized to false and the now not valid Mutex will be interpreted
		// as vaild and the code gets here
		if (!busAvailableMutexInitialized)
			return ret;

		std::vector<emBrickNode>::iterator itNode;
		for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); ++itNode)
		{
			if (itNode->index == node)
			{
				ret = static_cast<char>(itNode->slaves.size());
			}
		}
		ReleaseMutex(busAvailableMutex);
	}
	return ret;
}

unsigned char bB_GetLANProtVersion(char node)
{
	char ret = 0;
	if ((busAvailableMutexInitialized) && (WaitForSingleObject(busAvailableMutex, 1) == WAIT_OBJECT_0))
	{
		// In the unlikely event that the first check is passed, then a task switch sets the
		// busAvailableMutexInitialized to false and the now not valid Mutex will be interpreted
		// as vaild and the code gets here
		if (!busAvailableMutexInitialized)
			return ret;

		std::vector<emBrickNode>::iterator itNode;
		for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); ++itNode)
		{
			if (itNode->index == node)
			{
				ret = static_cast<char>(itNode->connection.ethProtVersion);
			}
		}
		ReleaseMutex(busAvailableMutex);
	}
	return ret;
}

short bB_getModulID(char node, char module)
{
	short ret = -1;
	if ((busAvailableMutexInitialized) && (WaitForSingleObject(busAvailableMutex, 1) == WAIT_OBJECT_0))
	{
		// In the unlikely event that the first check is passed, then a task switch sets the
		// busAvailableMutexInitialized to false and the now not valid Mutex will be interpreted
		// as vaild and the code gets here
		if (!busAvailableMutexInitialized)
			return ret;

		std::vector<emBrickNode>::iterator itNode;
		for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); ++itNode)
		{
			if ((itNode->index == node) && (itNode->slaves.size() >= static_cast<unsigned char>(module)) && (module > 0))
			{
				ret = itNode->slaves.at(module-1).dev_id;
			}
		}
		ReleaseMutex(busAvailableMutex);
	}
	return ret;
}

short bB_getModulSWVers(char node, char module)
{
	short ret=-1;
	if ((busAvailableMutexInitialized) && (WaitForSingleObject(busAvailableMutex, 1) == WAIT_OBJECT_0))
	{
		// In the unlikely event that the first check is passed, then a task switch sets the
		// busAvailableMutexInitialized to false and the now not valid Mutex will be interpreted
		// as vaild and the code gets here
		if (!busAvailableMutexInitialized)
			return ret;

		std::vector<emBrickNode>::iterator itNode;
		for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
		{
			if ((itNode->index == node) && (itNode->slaves.size() >= static_cast<unsigned char>(module)) && (module > 0))
			{
				ret = itNode->slaves.at(module-1).hardwareVersion;
			}
		}
		ReleaseMutex(busAvailableMutex);
	}
	return ret;
}

short bB_getModulbBVers(char node, char module)
{
	short ret=-1;
	if ((busAvailableMutexInitialized) && (WaitForSingleObject(busAvailableMutex, 1) == WAIT_OBJECT_0))
	{
		// In the unlikely event that the first check is passed, then a task switch sets the
		// busAvailableMutexInitialized to false and the now not valid Mutex will be interpreted
		// as vaild and the code gets here
		if (!busAvailableMutexInitialized)
			return ret;

		std::vector<emBrickNode>::iterator itNode;
		for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
		{
			if ((itNode->index == node) && (itNode->slaves.size() >= static_cast<unsigned char>(module)) && (module > 0))
			{
				ret = itNode->slaves.at(module-1).protocolVersion;
			}
		}
		ReleaseMutex(busAvailableMutex);
	}
	return ret;
}

short bB_getModulComStatus(char node, char module)
{
	short ret=0;
	if ((busAvailableMutexInitialized) && (WaitForSingleObject(busAvailableMutex, 1) == WAIT_OBJECT_0))
	{
		// In the unlikely event that the first check is passed, then a task switch sets the
		// busAvailableMutexInitialized to false and the now not valid Mutex will be interpreted
		// as vaild and the code gets here
		if (!busAvailableMutexInitialized)
			return ret;

		std::vector<emBrickNode>::iterator itNode;
		for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
		{
			if ((itNode->index == node) && (itNode->slaves.size() >= static_cast<unsigned char>(module)) && (module > 0))
			{
				if (WaitForSingleObject(itNode->connection.comMutex, INFINITE) == WAIT_OBJECT_0)
				{
					ret = itNode->connection.rxData[itNode->slaves.at(module-1).offsetOut];
					ReleaseMutex(itNode->connection.comMutex);
				}
			}
		}
		ReleaseMutex(busAvailableMutex);
	}
	return ret;
}

void bB_terminate(void)
{
	if ((busAvailableMutexInitialized) && (WaitForSingleObject(busAvailableMutex, 5000) == WAIT_OBJECT_0))
	{
		std::vector<emBrickNode>::iterator itNode;
		for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
		{
			endConnection(&(itNode->connection));
			CloseHandle(itNode->connection.comMutex);
		}

		busAvailableMutexInitialized = false;
		CloseHandle(busAvailableMutex);
	}

	SetEvent(exitEmBrickEvent);

	if (WaitForSingleObject(emBrickTaskID, 5000) != WAIT_OBJECT_0)
	{
		TerminateThread(emBrickTaskID, 1);
		WaitForSingleObject(emBrickTaskID, INFINITE);
	}
	CloseHandle(emBrickTaskID);
	CloseHandle(exitEmBrickEvent);
}

unsigned short bB_getShort(char node, char module, char bytePos)
{
	unsigned short ret = 0;
	if ((busAvailableMutexInitialized) && (WaitForSingleObject(busAvailableMutex, 1) == WAIT_OBJECT_0))
	{
		// In the unlikely event that the first check is passed, then a task swithc sets the
		// busAvailableMutexInitialized to false and the now not valid Mutex will be interpreted
		// as vaild and the code gets here
		if (!busAvailableMutexInitialized)
			return ret;

		std::vector<emBrickNode>::iterator itNode;
		for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
		{
			if ((itNode->index == node) && (itNode->slaves.size() >= static_cast<unsigned short>(module)) && (module > 0))
			{
				if (WaitForSingleObject(itNode->connection.comMutex, INFINITE) == WAIT_OBJECT_0)
				{
					// First byte of receive buffer is a status byte of the emBRICK-BUS, because of that +1
					ret = itNode->connection.rxData[itNode->slaves.at(module-1).offsetOut+bytePos+1] << 8;
					ret += itNode->connection.rxData[itNode->slaves.at(module-1).offsetOut+bytePos+2];
					ReleaseMutex(itNode->connection.comMutex);
				}
			}
		}
	}
	ReleaseMutex(busAvailableMutex);
	return ret;
}

unsigned char bB_getByte(char node, char module, char bytePos)
{
	unsigned char ret = 0;
	if ((busAvailableMutexInitialized) && (WaitForSingleObject(busAvailableMutex, 1) == WAIT_OBJECT_0))
	{
		// In the unlikely event that the first check is passed, then a task swithc sets the
		// busAvailableMutexInitialized to false and the now not valid Mutex will be interpreted
		// as vaild and the code gets here
		if (!busAvailableMutexInitialized)
			return ret;

		std::vector<emBrickNode>::iterator itNode;
		for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
		{
			if ((itNode->index == node) && (itNode->slaves.size() >= static_cast<unsigned short>(module)) && (module > 0))
			{
				if (WaitForSingleObject(itNode->connection.comMutex, INFINITE) == WAIT_OBJECT_0)
				{
					// First byte of receive buffer is a status byte of the emBRICK-BUS, because of that +1
					ret = itNode->connection.rxData[itNode->slaves.at(module-1).offsetOut+bytePos+1];
					ReleaseMutex(itNode->connection.comMutex);
				}
			}
		}
	}
	ReleaseMutex(busAvailableMutex);
	return ret;
}

unsigned char bB_getBit(char node, char module, char bytePos, char bitPos)
{
	unsigned char ret = 0;
	if ((busAvailableMutexInitialized) && (WaitForSingleObject(busAvailableMutex, 1) == WAIT_OBJECT_0))
	{
		// In the unlikely event that the first check is passed, then a task swithc sets the
		// busAvailableMutexInitialized to false and the now not valid Mutex will be interpreted
		// as vaild and the code gets here
		if (!busAvailableMutexInitialized)
			return ret;

		std::vector<emBrickNode>::iterator itNode;
		for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
		{
			if ((itNode->index == node) && (itNode->slaves.size() >= static_cast<unsigned short>(module)) && (module > 0))
			{
				if (WaitForSingleObject(itNode->connection.comMutex, INFINITE) == WAIT_OBJECT_0)
				{
					// First byte of receive buffer is a status byte of the emBRICK-BUS, because of that +1
					ret = itNode->connection.rxData[itNode->slaves.at(module-1).offsetOut+bytePos+1];
					ret = (ret & (1 << bitPos)) ? 1 : 0;
					ReleaseMutex(itNode->connection.comMutex);
				}
			}
		}
	}
	ReleaseMutex(busAvailableMutex);
	return ret;
}

void bB_putShort(char node, char module, char bytePos, short value)
{
	if ((busAvailableMutexInitialized) && (WaitForSingleObject(busAvailableMutex, 1) == WAIT_OBJECT_0))
	{
		// In the unlikely event that the first check is passed, then a task switch sets the
		// busAvailableMutexInitialized to false and the now not valid Mutex will be interpreted
		// as vaild and the code gets here
		if (!busAvailableMutexInitialized)
			return;

		std::vector<emBrickNode>::iterator itNode;
		for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
		{
			if ((itNode->index == node) && (itNode->slaves.size() >= static_cast<unsigned short>(module)) && (module > 0))
			{
				if (WaitForSingleObject(itNode->connection.comMutex, INFINITE) == WAIT_OBJECT_0)
				{
					itNode->connection.txData[itNode->slaves.at(module-1).offsetIn+bytePos] = value/256;
					itNode->connection.txData[itNode->slaves.at(module-1).offsetIn+bytePos+1] = value%256;
					ReleaseMutex(itNode->connection.comMutex);
				}
			}
		}
		ReleaseMutex(busAvailableMutex);
	}
}

void bB_putByte(char node, char module, char bytePos, char value)
{
	if ((busAvailableMutexInitialized) && (WaitForSingleObject(busAvailableMutex, 1) == WAIT_OBJECT_0))
	{
		// In the unlikely event that the first check is passed, then a task switch sets the
		// busAvailableMutexInitialized to false and the now not valid Mutex will be interpreted
		// as vaild and the code gets here
		if (!busAvailableMutexInitialized)
			return;

		std::vector<emBrickNode>::iterator itNode;
		for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
		{
			if ((itNode->index == node) && (itNode->slaves.size() >= static_cast<unsigned short>(module)) && (module > 0))
			{
				if (WaitForSingleObject(itNode->connection.comMutex, INFINITE) == WAIT_OBJECT_0)
				{
					itNode->connection.txData[itNode->slaves.at(module-1).offsetIn+bytePos] = value;
					ReleaseMutex(itNode->connection.comMutex);
				}
			}
		}
		ReleaseMutex(busAvailableMutex);
	}
}

void bB_putBit(char node, char module, char bytePos, char bitPos, char value)
{
	if ((busAvailableMutexInitialized) && (WaitForSingleObject(busAvailableMutex, 1) == WAIT_OBJECT_0))
	{
		// In the unlikely event that the first check is passed, then a task switch sets the
		// busAvailableMutexInitialized to false and the now not valid Mutex will be interpreted
		// as vaild and the code gets here
		if (!busAvailableMutexInitialized)
			return;

		std::vector<emBrickNode>::iterator itNode;
		for (itNode = emBrickNodes.begin(); itNode != emBrickNodes.end(); itNode++)
		{
			if ((itNode->index == node) && (itNode->slaves.size() >= static_cast<unsigned short>(module)) && (module > 0))
			{
				if (WaitForSingleObject(itNode->connection.comMutex, INFINITE) == WAIT_OBJECT_0)
				{
					if (value)
						itNode->connection.txData[itNode->slaves.at(module-1).offsetIn+bytePos] |= (1 << bitPos);
					else
						itNode->connection.txData[itNode->slaves.at(module-1).offsetIn+bytePos] &= ~(1 << bitPos);
					ReleaseMutex(itNode->connection.comMutex);
				}
			}
		}
		ReleaseMutex(busAvailableMutex);
	}
}