/******************************************************************************
*
*    /file    EOG_Firmware.ino
*
*    /desc    Main file for EOG Arduino application.
*
*    /log     2/19/15  gcg - Initial release.
*
******************************************************************************/

// ***** Include Files ********************************************************

// Arduino Source

#include <Arduino.h>
#include <SPI.h>

// Local Modules

#include "Analog.h"
#include "Command.h"
#include "Direction.h"

// ***** Local Definitions ****************************************************

// Config flags - placed here for easy updating during development

#define ADC_RANGE    ANALOG_10_TO_10
#define BAUD_RATE    115200
#define HORIZONTAL   ANALOG_CH5
#define VERTICAL     ANALOG_CH4

// ***** Function Definitions *************************************************

/******************************************************************************
*
*    /name       setup
*
*    /purpose    Arduino internal funtion, runs once on startup. Initializes
*                Analog module and opens Serial port.
*
*    /ret        void
*
******************************************************************************/

void setup()
{
  
  // Initialize Command Module - must be first for command table int
  
  Command_Initialize((long)BAUD_RATE);
  
  // Initialize the Analog Module
  
  Analog_Initialize(ADC_RANGE);
  
  // Initialize the direction module
  
  Direction_Initialize();
}

/******************************************************************************
*
*    /name       loop
*
*    /purpose    Arduino internal funtion, main application code. Loops
*                through and performs any repetative tasks.
*
*    /ret        void
*
******************************************************************************/

void loop()
{
  int i;
  
#if 0
  
  // Update Direction reading
  
  Direction_Update();
  
  // DEBUG: Broadcast Direction
  
  Direction_BroadcastState();
  
  // DEBUG: Long delay for now
  
  delay(5000);
  
#else
  
  // DEBUG: Print analog voltages to terminal
    
  Serial.print("VERTICAL: ");
  Serial.print(Analog_ReadVolts(VERTICAL), 5);
  
  Serial.print("    HORIZONTAL: ");
  Serial.print(Analog_ReadVolts(HORIZONTAL), 5);
  
  Serial.print("\r\n");  
  
  delay(1000);  
  
#endif
}
