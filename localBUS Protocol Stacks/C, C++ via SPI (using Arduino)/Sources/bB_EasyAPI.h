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

#include "bBDefines.h"


void bB_Close(void);
				
unsigned char bB_getNumModules(unsigned short node);
short bB_getModulID(unsigned short node, unsigned short slaveNo);
unsigned char bB_getModulSwVers(unsigned short node, unsigned short slaveNo);
unsigned char bB_getModulbBVers(unsigned short node, unsigned short slaveNo);
unsigned char bB_getModulStatus(unsigned short node, unsigned short slaveNo);
void bB_terminate(void);							

unsigned short bB_getWord(unsigned short node, unsigned short slaveNo, unsigned short bytePos);
unsigned char bB_getByte(unsigned short node, unsigned short slaveNo, unsigned short bytePos);
unsigned char bB_getBit(unsigned short node, unsigned short slaveNo, unsigned short bytePos, unsigned char bitPos);

short bB_putWord(unsigned short node, unsigned short slaveNo, unsigned short bytePos, short value);
short bB_putByte(unsigned short node, unsigned short slaveNo, unsigned short bytePos, unsigned char value);
short bB_putBit(unsigned short node, unsigned short slaveNo, unsigned short bytePos, unsigned char bitPos, unsigned char value);
void bB_Start();
