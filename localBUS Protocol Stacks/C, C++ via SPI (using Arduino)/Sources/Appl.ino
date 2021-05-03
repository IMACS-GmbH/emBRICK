#include "bBConfigs.h"
#include "bB_EasyAPI.h"
#include "brickbus.h"

/*************************** DEFINES ****************************/
#define UPDATE_RATE		10			// ms
#define PRINT_TIME    500    // ms

/******************* EXTERNAL VARIABLES ************************/

/******************* FUNCTION DECLARATIONS *********************/
void delayMS(long delayTime);
void application();

/********************* GLOBAL VARIABLES ************************/
static long i = 0;  // Laufvariable
unsigned long prevMillis = 0;

// Definition Inputvariablen
short input1;
short input2;
short input3;
//...

/**************** DEMO FUNCTIONS ***************************/
void application()
{
  //setzt aktuelle Systemzeit in currentMillis
  unsigned long currentMillis = millis();
  
  // Aufruf von put Funktionen
  bB_putByte(1,1,0,55);
  bB_putByte(1,2,0,255);
  bB_putByte(1,3,0,22); 

  // Aufruf von get Funktionen
  input1 = bB_getByte(1,1,0);
  input2 = bB_getWord(1,3,0);
  input3 = bB_getByte(1,4,0);
  
  // Aufruf der Hauptkommunikationsfunktion für den brickBUS -> muss alle 10ms aufgerufen werden
  bB_Processing();

  // Aufruf der Wartezeiten abhängig von der Initialisierung des brickBuses
  if(!brickBUS[0].bB_BUS.initialized)
    delayMS(UPDATE_RATE);  
  else
  {
    if(i == (bB_getNumModules(1)*2)-1)  
    {
      i=0;
      delayMS(UPDATE_RATE);

      // Wartet mit der Ausgabe der Plots bis PRINT_TIME erreicht wurde
      if(currentMillis - prevMillis >= PRINT_TIME)
      {
        prevMillis = currentMillis;

        
        // Ausgabe von Variablen der get Funktionen -> nur in dieser Abfrage einfügen, sonst stimmen die internen Zeiten des Busses nicht mehr!
        Serial.print("Input 1: ");
        Serial.println(input1);
        Serial.print("Input 2: ");
        Serial.println(input2);
        Serial.print("Input 3: ");
        Serial.println(input3);

        Serial.println();
      }
    }
    else i++;
  }
}

/****************************************************************
*FUNCTION NAME:		delayMS(long delayTime)					            	*
*                                                               *
*DESCRIPTION:		wait x ms									                      *
*PARAMETER:			None										                        *
*RETURN:			None					                                    *
****************************************************************/
void delayMS(long delayTime)
{
	delayMicroseconds(delayTime * 1000);
}


/****************************************************************
*FUNCTION NAME:		setup()               					            	*
*                                                               *
*DESCRIPTION:		grundlegende Einstellungen werden hier einmal   *
*               aufgerufen                                      *
*PARAMETER:			None										                        *
*RETURN:			None					                                    *
****************************************************************/
void setup()
{
  // Startet SPI Kommunikation und stellt diese passend zum emBRICK Protokoll ein
  bB_Start();	
}


/****************************************************************
*FUNCTION NAME:		loop()                					            	*
*                                                               *
*DESCRIPTION:		dauerschleife -> hier eigentlicher zyklischer   *
*               Aufruf von put und get Funktionen               *
*PARAMETER:			None										                        *
*RETURN:			None					                                    *
****************************************************************/
void loop()
{
  application();
}
