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
#include <SPI.h>

// Local Modules

#include "Analog.h"

// ***** Local Definitions ****************************************************

// Config flags - placed here for easy updating during development

#define ADC_RANGE  ANALOG_10_TO_10
#define BAUD_RATE  115200

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
  
  // Open Serial communications
  
  Serial.begin(BAUD_RATE);
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
  
  // Update Analog readings
  
  Analog_Update();
  
  // DEBUG: Print analog voltages to terminal
  
  for(i=0; i<8; i++) {
    Serial.print(Analog_ReadVolts((Analog_Channel_t)i), 5);
    Serial.print(",");
  }
  Serial.print("\r\n");
  delay(1000);  
}
