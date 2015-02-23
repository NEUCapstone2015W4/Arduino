/******************************************************************************
*
*    /file    Analog.cpp
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
#include <Arduino.h>

// Local Modules

#include "Analog.h"

// ***** Local Definitions ****************************************************

// DAC to voltage coversion factor for a -5 to 5 range
// 10/(2^16) = 0.000152587890625

#define ANALOG_SCALE_10   0.000152587890625

// DAC to voltage coversion factor for a -10 to 10 range
// 20/(2^16) = 0.00030517578

#define ANALOG_SCALE_20   0.00030517578

// Pin definitions

#define BUSY 3
#define RESET 4
#define START_CONVERSION 5
#define CHIP_SELECT 10
#define MISO 12
#define LED 13
#define TOTAL_RAW_BYTES 16

// ***** Local Variables ******************************************************

// The mode setting of the Analog board

static Analog_Mode_t Analog__meMode;

// Unformatted, raw data from ADC

static byte Analog__mucRawData[TOTAL_RAW_BYTES];

// ADC count values for each channel

static signed long Analog__mlParsedData[ANALOG_NUM_CHANNELS];


// ***** Local Funtions *******************************************************

static void Analog__ReadRaw();

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

/******************************************************************************
*
*    /name       Analog__ReadRaw
*
*    /purpose    Reads 16 bytes of data from the ADC. This will need to be
*                converted before being used for any calculation.
*
*    /ret        void
*
******************************************************************************/

static void Analog__ReadRaw()
{
  unsigned char xucBytesToRead = TOTAL_RAW_BYTES;
  
  // Toggle the start conversion line 
  
  digitalWrite(START_CONVERSION, LOW);
  delayMicroseconds(10);
  digitalWrite(START_CONVERSION, HIGH);

  // Wait for conversion to complete
  
  while (digitalRead(BUSY) == HIGH) {}
  digitalWrite(CHIP_SELECT, LOW);
  
  // Store raw data
  
  while (xucBytesToRead > 0) {
    Analog__mucRawData[TOTAL_RAW_BYTES - xucBytesToRead] = SPI.transfer(0x00);
    xucBytesToRead--;
  }
  
  // Wait for next conversion
  
  digitalWrite(CHIP_SELECT, HIGH);  
}

/******************************************************************************
*
*    /name       Analog_Update
*
*    /purpose    Update the readings for each channel.
*
*    /ref        Analog__ReadRaw
*
*    /ret        void
*
******************************************************************************/

void Analog_Update()
{
  unsigned char xucCurrByte = 0;
 
  // Read in raw data for each channel
 
  Analog__ReadRaw();
 
  // Convert to DAC counts (signed)
 
  for (int xwChannel=0; xwChannel < ANALOG_NUM_CHANNELS; xwChannel++)
  {
    
    // Write value
    
    Analog__mlParsedData [xwChannel] = 
     (Analog__mucRawData[xucCurrByte] << 8) + Analog__mucRawData[xucCurrByte];
    
    // Update raw byte index
    
    xucCurrByte += 2;
  }
}

/******************************************************************************
*
*    /name       Analog_ReadCounts
*
*    /purpose    Returns the channel reading in counts
*
*    /param[in]  zeChannel    Channel to read
*
*    /ret        int          Channel count reading (signed)
*
******************************************************************************/
signed long Analog_ReadCounts (Analog_Channel_t zeChannel)
{
  
  // Simply return the internal static variable
  
  return Analog__mlParsedData[zeChannel];
}

/******************************************************************************
*
*    /name       Analog_ReadVolts
*
*    /purpose    Returns the channel reading in volts
*
*    /param[in]  zeChannel    Channel to read
*
*    /ret        float        Channel voltage reading (signed)
*
******************************************************************************/
float Analog_ReadVolts (Analog_Channel_t zeChannel)
{
  
  // Depending on the mode, return the count value multiplied by
  // the correct scale factor
  
  if (Analog__meMode == ANALOG_5_TO_5)
  {
    
    return (float)Analog__mlParsedData[zeChannel] * ANALOG_SCALE_10;
  }
  else    // Analog__meMode == ANALOG_10_TO_10
  {
    
    return (float)Analog__mlParsedData[zeChannel] * ANALOG_SCALE_20;
  }
}

