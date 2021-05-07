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

#pragma once

#define MAX_SUPPORTED_ETH_PROT_VERSION		4
#define MIN_SUPPORTED_ETH_PROT_VERSION		4

#include <string>
#include <vector>

/** \brief Initializes communication with emBRICK-Master
 *
 * \param[in] List of IP-Adress of all Nodes to connect to
 * \return 1 on success 0 on failure
 */
char bB_Init(std::vector<std::string> ipList);

/** \brief Starts the periodical communication with emBRICK-Master
 *
 * \param[in] updaterate Update rate in milliseconds
 * \return 1 if communication was started 0 on error
 */
short bB_Start(short updaterate);

/** \brief Returns the number of found modules
 *
 * \param[in] node index of node (as specified in datei.txt) if automatically set: 1
 * \return Number of modules
 */
char bB_GetConfNumModules(char node);

/** \brief Returns the LAN protocol version of the chosen node
 *
 * \return LAN protocol version
 */
unsigned char bB_GetLANProtVersion(char node);

/** \brief Gets the module ID of a specified slave module
 *
 * \param[in] node index of node (as specified in datei.txt) if automatically set: 1
 * \param[in] module 1 based index of slave module on specified node
 * \return module ID of specified slave module
 */
short bB_getModulID(char node, char module);

/** \brief Gets the software version of a specified slave module
 *
 * \param[in] node index of node (as specified in datei.txt) if automatically set: 1
 * \param[in] module 1 based index of slave module on specified node
 * \return software version of specified slave module
 */
short bB_getModulSWVers(char node, char module);

/** \brief Gets the BrickBus protocol version of a specified slave module
 *
 * \param[in] node index of node (as specified in datei.txt) if automatically set: 1
 * \param[in] module 1 based index of slave module on specified node
 * \return Brickbus protocol version of specified slave module
 */
short bB_getModulbBVers(char node, char module);

/** \brief Gets module communication status (error codes) of a specifed slave module
 *
 * \param[in] node index of node (as specified in datei.txt) if automatically set: 1
 * \param[in] module 1 based index of slave module on specified node
 * return communication status of specified slave module
 */
short bB_getModulComStatus(char node, char module);

/** \brief Ends communication with emBRICK-Master and cleans up resources reserved by bB_Init and bB_Start
 */
void bB_terminate(void);

/** \brief Gets 16 bit of data from a specified slave module
 *
 * \param[in] node index of node (as specified in datei.txt) if automatically set: 1
 * \param[in] module 1 based index of slave module on specified node
 * \param[in] bytePos 0 based offset in bytes within the data
 * \return 16 Bit of data at the specified position from the specified slave module
 */
unsigned short bB_getShort(char node, char module, char bytePos);

/** \brief Gets 8 bit of data from a specified slave module
 *
 * \param[in] node index of node (as specified in datei.txt) if automatically set: 1
 * \param[in] module 1 based index of slave module on specified node
 * \param[in] bytePos 0 based offset in bytes within the data
 * \return 8 Bit of data at the specified position from the specified slave module
 */
unsigned char bB_getByte(char node, char module, char bytePos);

/** \brief Gets 1 bit of data from a specified slave module
 *
 * \param[in] node index of node (as specified in datei.txt) if automatically set: 1
 * \param[in] module 1 based index of slave module on specified node
 * \param[in] bytePos 0 based offset in bytes within the data
 * \param[in] bitPos 0 based selection of bit to get
 * \return 1 Bit of data at the specified position from the specified slave module
 */
unsigned char bB_getBit(char node, char module, char bytePos, char bitPos);

/** \brief Sends 16 bit of data to a specified slave module
 *
 * \param[in] node index of node (as specified in datei.txt) if automatically set: 1
 * \param[in] module 1 based index of slave module on specified node
 * \param[in] bytePos 0 based offset in bytes within the data
 * \param[in] value data to send
 */
void bB_putShort(char node, char module, char bytePos, short value);

/** \brief Sends 8 bit of data to a specified slave module
 *
 * \param[in] node index of node (as specified in datei.txt) if automatically set: 1
 * \param[in] module 1 based index of slave module on specified node
 * \param[in] bytePos 0 based offset in bytes within the data
 * \param[in] value data to send
 */
void bB_putByte(char node, char module, char bytePos, char value);

/** \brief Sends 1 bit of data to a specified slave module
 *
 * \param[in] node index of node (as specified in datei.txt) if automatically set: 1
 * \param[in] module 1 based index of slave module on specified node
 * \param[in] bytePos 0 based offset in bytes within the data
 * \param[in] bitPos 0 based selection of bit to send
 * \param[in] value data to send
 */
void bB_putBit(char node, char module, char bytePos, char bitPos, char value);