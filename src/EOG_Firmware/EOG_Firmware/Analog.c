/******************************************************************************
*
*    /file    Analog.c
*
*    /desc    The Precision Voltage Shield is an 8-channel, 16-bit analog-
*             to-digital converter that plugs on top of an Arduino micro
*             controller. It is similar to the analog input pins on the 
*             Arduino Uno, but with 64 times better resolution and 8 channels
*             instead of 6.
*
*             There is no firmware on the Shield itself, it instead self
*             regulates using hardware. The voltage range can span either
*             -5 to 5 or -10 to 10 volts, depending on the coniguration of
*             an on-board jumper.
*
*    /log     2/19/15  gcg - Initial release.
*
******************************************************************************/

// ***** Include Files ********************************************************

// Arduino Source

#include <SPI.h>

// Local Modules

#include "Analog.h"

// ***** Local Definitions ****************************************************

// DAC to voltage coversion factor for a -5 to 5 range
// 10/(2^16) = 0.000152587890625

#define ANALOG_SCALE_10   0.000152587890625

// DAC to voltage coversion factor for a -10 to 10 range
// 20/(2^16) = 0.00030517578

#define ANALOG_SCALE_20   0.00030517578

// ***** Local Variables ******************************************************

// The mode setting of the Analog board

static Analog_Mode_t Analog__meMode;

// ***** Function Definitions *************************************************

/******************************************************************************
*
*    /name       Analog_Initialize
*
*    /purpose    Performs any needed SPI configuration for the Analog board
*                along with saving off any parameters for software config.
*
*    /param[in]  zeMode    Mode to set (-5 to 5 or 10 to 10)
*
*    /ret        void
*
******************************************************************************/

void Analog_Initialize (Analog_Mode_t zeMode)
{
  
  // Initialize SPI communitcations
  
  pinMode(BUSY, INPUT);
  pinMode(RESET, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(START_CONVERSION, OUTPUT);
  pinMode(MISO, INPUT);
  
  SPI.begin();
   
  digitalWrite(START_CONVERSION, HIGH);  
  digitalWrite(CHIP_SELECT, HIGH);
  digitalWrite(RESET, HIGH);
  delay(1);
  digitalWrite(RESET, LOW);
  
  // Save the mode setting

  Analog__meMode = zeMode;
}

