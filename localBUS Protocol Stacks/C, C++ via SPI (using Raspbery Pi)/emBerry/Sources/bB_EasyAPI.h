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

#include <stdio.h>
#include "bBDefines.h"
#include <bcm2835.h>

// module 1 - 2Rel4Di2Ai (basic module) 
#define MOD1_REL1		1,1,0,0		// Node 1, Module 1, Byte 0, Bit 0
#define MOD1_REL2		1,1,0,1		// Node 1, Module 1, Byte 0, Bit 1
#define MOD1_DI1		1,1,4,0	// Node 1, Module 1, Byte 4, Bit 0
#define MOD1_DI2		1,1,4,1	// Node 1, Module 1, Byte 4, Bit 1
#define MOD1_DI3		1,1,4,2	// Node 1, Module 1, Byte 4, Bit 2
#define MOD1_DI4		1,1,4,3	// Node 1, Module 1, Byte 4, Bit 3
#define MOD1_AI1		1,1,0		// Node 1, Module 1, Byte 0
#define MOD1_AI2		1,1,2		// Node 1, Module 1, Byte 2
/*---For exemple------
// module 2
#define MOD2_DO1		1,2,0,0		// Node 1, Module 2, Byte 0, Bit 0
#define MOD2_DO2		1,2,0,1		// Node 1, Module 2, Byte 0, Bit 1
#define MOD2_DO3		1,2,0,2		// Node 1, Module 2, Byte 0, Bit 2
#define MOD2_DO4		1,2,0,3		// Node 1, Module 2, Byte 0, Bit 3
#define MOD2_DO5		1,2,0,4		// Node 1, Module 2, Byte 0, Bit 4
#define MOD2_DO6		1,2,0,5		// Node 1, Module 2, Byte 0, Bit 5
#define MOD2_D07		1,2,0,6		// Node 1, Module 2, Byte 0, Bit 6
#define MOD2_D08		1,2,0,7		// Node 1, Module 2, Byte 0, Bit 7
#define MOD2_DI1		1,2,0,0		// Node 1, Module 2, Byte 0, Bit 0
#define MOD2_DI2		1,2,0,1		// Node 1, Module 2, Byte 0, Bit 1
#define MOD2_DI3		1,2,0,2		// Node 1, Module 2, Byte 0, Bit 2
#define MOD2_DI4		1,2,0,3		// Node 1, Module 2, Byte 0, Bit 3
#define MOD2_DI5		1,2,0,4		// Node 1, Module 2, Byte 0, Bit 4
#define MOD2_DI6		1,2,0,5		// Node 1, Module 2, Byte 0, Bit 5
#define MOD2_DI7		1,2,0,6		// Node 1, Module 2, Byte 0, Bit 6
#define MOD2_DI8		1,2,0,7		// Node 1, Module 2, Byte 0, Bit 7
// modul 3
#define MOD3_REL1		1,3,0,0		// Node 1, Module 3, Byte 0, Bit 0
#define MOD3_REL2		1,3,0,1		// Node 1, Module 3, Byte 0, Bit 1
#define MOD3_REL3		1,3,0,2		// Node 1, Module 3, Byte 0, Bit 2
#define MOD3_REL4		1,3,0,3		// Node 1, Module 3, Byte 0, Bit 3
#define MOD3_REL5		1,3,0,4		// Node 1, Module 3, Byte 0, Bit 4
#define MOD3_REL6		1,3,0,5		// Node 1, Module 3, Byte 0, Bit 5
*/

int bB_Init(void);
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

