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

/**** emBRICK Startkit Example *****************************************************
*
* This Application is a fully functional example how to use the provided emBRICK-Driver.
* While this application might seem overwhelming to an unexperienced user, the principles are pretty easy.
*
*  Here is a short introduction of the essential functions:
*
*
* The Initialisation:
*
* char bB_Init(std::vector<std::string> ipList):
*		This Function is used to connect to the localmaster. A very basic way to connect to a single
*		localmaster would like this:
*
*		std::vector<std::string> ipList;	// The IP-number list
*		std::string ip = "192.168.3.10";	// The IP of your localmaster
*		ipList.push_back(ip);				// Add the IP of your localmaster to the IP-number list
*		bB_Init(ipList);					// Connect to all localmasters in the list
*
* Your localmaster is now connected, but the communication is NOT yet running. The emBRICK localmaster
* expects periodic updates otherwise he will assume that the connection has been lost. We already provided
* you with an easy way to do so.
*
* short bB_Start(short updatetime):
*		This function will start a thread that will periodically send an update of the inputs and receive an update
*		of the outputs. The updatetime is given in milliseconds. A good starting point would be an updatetime of 10ms.
*		Lower updatetimes are not recommended due to Windows not being a realtime operating system.
*		Updatetimes higher than 40ms are also not recommended.
*
*		A basic way to use this function would look like this:
*
*		bB_Start(10);
*
* That's it. You should now see the yellow LED on the LWCS. You are now connected to the localmaster. Even if the
* localmaster briefly looses power or network connectivity it will automatically reconnect.
*
*
* The Application Itself:
*
* To write outputs/read inupts you can use the 6 I/O-functions:
* 
*	For single-bit operations (relays, single LED...):
*		void bB_putBit(char node, char module, char bytePos, char bitPos, char value);
*		unsigned char bB_getBit(char node, char module, char bytePos, char bitPos);
*
*	For single-byte operations (group of relays, LEDs...):
*		void bB_putByte(char node, char module, char bytePos, char value);
*		unsigned char bB_getByte(char node, char module, char bytePos);
*
*	For 2-byte operations (temperature/resistance sensors, PWM...)
*		void bB_putShort(char node, char module, char bytePos, short value);
*		unsigned short bB_getShort(char node, char module, char bytePos);
*
* Check out the example below or "embrick.h" aswell as the product documentation of your emBrick-module on how
* to use these functions.
*
*
*
* emBRICK is a project that is aimed at the community. We are publishing nearly all sources so that the community is
* able to adapt/improve the software to its needs. You can even design your own emBRICK-modules. We are merely
* providing a convenient way to obtain emBRICK-Hardware. 
* Feel free to share your designs and applications on our website www.embrick.de. Feedback is also much appreciated.
* Should you have further questions send us a mail to support@embrick.de.
*
* emBRICK is what you make of it, surprise us!
*
************************************************************************/

#include "stdafx.h"
#include "embrick.h"

#include <Windows.h>
#include <iostream>
#include <conio.h>

/*---DEFINES---*/
#define UPDATE_RATE		10		// delay in ms for access on emBRICK Bus(update)
#define RELAY1			1,1,0,0 // Macros for Relay1 (Node 1, Module 1, Byte 0, Bit 0)

/*---Main function---*/
int _tmain(int argc, _TCHAR* argv[])
{
	unsigned long loopCounter = 0;
	unsigned char flash = 0;
	char numModules = 0;
	unsigned char ethProtVersion = 0;
	std::vector<std::string> ipList;

	// Get IP address of node from user (only accept valid IPs)
	std::string ip = "";
	while ((inet_addr(ip.c_str()) == INADDR_NONE) || ((inet_addr(ip.c_str())) == INADDR_ANY))
	{
		std::cout << "Please enter the IP address of the emBRICK node: ";
		std::getline(std::cin, ip);
	}
	// add IP to list
	ipList.push_back(ip);
	 
	if (!bB_Init(ipList))	//Initialize the brickBUS and the communication ports 
	{
		std::cout << "Error could not initialize emBRICK.\n";
		return 1;
	}

	std::cout << "---------------------------------------------------\n";
	std::cout << "emBRICK(R), Starterkit Z-CouplingBrick_MSVC\n";
	std::cout << "BSP and sample application, V1.04\n";
	if(MIN_SUPPORTED_ETH_PROT_VERSION == MAX_SUPPORTED_ETH_PROT_VERSION)
		std::cout << "LAN communication version: " << static_cast<unsigned int>(MIN_SUPPORTED_ETH_PROT_VERSION) << "\n";
	else
		std::cout << "LAN communication version: " << static_cast<unsigned int>(MIN_SUPPORTED_ETH_PROT_VERSION) << " to " << static_cast<unsigned int>(MAX_SUPPORTED_ETH_PROT_VERSION) << "\n";
	std::cout << "(c) 2019 by IMACS GmbH \n";
	std::cout << "---------------------------------------------------\n\n";

	
	numModules = bB_GetConfNumModules(1);
	ethProtVersion = bB_GetLANProtVersion(1);
	if( ethProtVersion == 0 )
	{
		std::cout << "Local master could not be found\n";
		while(!kbhit());
	} else if( (ethProtVersion  > MAX_SUPPORTED_ETH_PROT_VERSION) || (ethProtVersion  < MIN_SUPPORTED_ETH_PROT_VERSION) )
	{
		std::cout << "The local-master at node 1 works with ethernet protocol version number " << static_cast<unsigned int>(ethProtVersion) << "\n";
		std::cout << "This program can only work with versions " << static_cast<unsigned int>(MIN_SUPPORTED_ETH_PROT_VERSION) << " to " << static_cast<unsigned int>(MAX_SUPPORTED_ETH_PROT_VERSION) << ".\n";
		while(!kbhit());
	} else
	{
		std::cout << static_cast<unsigned int>(numModules) << " modules found.\n";
		for (char i=1; i<=numModules; ++i)
		{
			std::cout << "module " << static_cast<unsigned int>(i) << ": ";
			std::cout << static_cast<unsigned int>(bB_getModulID(1, i)/1000) << "-";
			std::cout << static_cast<unsigned int>(bB_getModulID(1, i)%1000) << "\t";
			std::cout << "Version information: SW-Vers ";
			std::cout << static_cast<unsigned int>(bB_getModulSWVers(1, i)) << ", bB-Vers ";
			std::cout << static_cast<unsigned int>(bB_getModulbBVers(1, i)) << "\n";
		}
		if (!bB_Start(UPDATE_RATE)) // Start the communication
		{
			std::cout << "Error: Could not start emBRICK-communication.\n";
			return 1;
		}
		else
			std::cout << "Connection to local-master is running \n\n";

		std::cout << "To abort the program please press ESC\n\n";
		unsigned short led = 0;
		while (!GetAsyncKeyState(VK_ESCAPE))
		{
			unsigned char bitVal = bB_getBit(1, 1, 4, 2);	// Read in bit 2 on byte 4 from module 1 in node 1
			bB_putBit(1, 1, 0, 1, bitVal);			// Write that bit to bit 1 in byte 0 of module 1 in node 1

			if ((loopCounter % 100) == 0)	// Let bit 0 in byte 0 of module 1 in node 1 flash every second
				flash = !flash;
			bB_putBit(RELAY1, flash);	
		
			unsigned short AI_voltage = bB_getShort(1, 1, 0);	// Read out and display Voltage (unsigned short) from Byte 0 of module 1 in node 1
			unsigned short Di1 = bB_getBit( 1, 1, 4, 0);			// Read out Bit 0 from Byte 4 of module 1 in node 1
			unsigned short Di2 = bB_getBit( 1, 1, 4, 1);			// Read out Bit 1 from Byte 4 of module 1 in node 1
			unsigned short Di3 = bB_getBit( 1, 1, 4, 2);			// Read out Bit 2 from Byte 4 of module 1 in node 1
			unsigned short Di4 = bB_getBit( 1, 1, 4, 3);			// Read out Bit 3 from Byte 4 of module 1 in node 1
		
			std::cout << "Ai1 = " << AI_voltage<< "  Di = " << Di1<< Di2 << Di3 << Di4 <<"  \r";
			
			/* example to dim LEDs
			//void bB_putShort(char node, char module, char bytePos, short value)
			bB_putShort(1, 1, 0, led);
			bB_putShort(1, 1, 2, led);
			bB_putShort(1, 1, 4, led);
			bB_putShort(1, 1, 6, led);

			bB_putShort(1, 2, 0, led);
			bB_putShort(1, 2, 2, led);
			bB_putShort(1, 2, 4, led);
			bB_putShort(1, 2, 6, led);

			bB_putShort(1, 3, 0, led);
			bB_putShort(1, 3, 2, led);
			bB_putShort(1, 3, 4, led);
			bB_putShort(1, 3, 6, led);

			unsigned short AI_voltage1 = bB_getShort(1, 4, 0);	
			unsigned short AI_voltage2 = bB_getShort(1, 4, 2);
			unsigned short AI_voltage3 = bB_getShort(1, 4, 4);
			unsigned short AI_voltage4 = bB_getShort(1, 4, 6);
			unsigned short AI_voltage5 = bB_getShort(1, 4, 8);
			unsigned short AI_voltage6 = bB_getShort(1, 4, 10);
			unsigned short AI_voltage7 = bB_getShort(1, 4, 12);
			unsigned short AI_voltage8 = bB_getShort(1, 4, 14);

			std::cout << "Ai1 = " << AI_voltage1
				<< "| Ai2 = " << AI_voltage2
				<< "| Ai3 = " << AI_voltage3
				<< "| Ai4 = " << AI_voltage4
				<< "| Ai5 = " << AI_voltage5 
				<< "| Ai6 = " << AI_voltage6
				<< "| Ai7 = " << AI_voltage7 
				<< "| Ai8 = " << AI_voltage8 << "\r";

			led += 100;
			if (led > 20000)
				led = 0;
			*/

			Sleep(10);
			++loopCounter;
		}
	}

	bB_terminate();	// stop the communication and deinitialize the brickBUS
	return 0;
}