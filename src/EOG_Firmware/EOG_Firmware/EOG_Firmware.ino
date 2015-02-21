/******************************************************************************
*
*    /file    EOG_Firmware.c
*
*    /desc    Main file for EOG Arduino application.
*
*    /log     2/19/15  gcg - Initial release.
*
******************************************************************************/

// ***** Include Files ********************************************************

// Arduino Source

#include <Arduino.h>

// Local Modules

#include "Analog.h"

// ***** Local Definitions ****************************************************

// Config flags - placed here for easy updating during development

#define ADC_RANGE  ANALOG_10_TO_10
#define BAUD_RATE  9600u

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
  
  // Initialize the Analog Module
  
  Analog_Initialize(ADC_RANGE);
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
  
}
