/******************************************************************************
*
*    /file    Analog.h
*
*    /desc    Header file for Analog module.
*
*    /log     2/19/15  gcg - Initial release.
*
******************************************************************************/

#ifndef _ANALOG_H
#define _ANALOG_H

// ***** Definitions **********************************************************

// State of the on board range jumper

typedef enum
{
  ANALOG_5_TO_5,
  ANALOG_10_TO_10,
} Analog_Mode_t;

// ADC Channels

typedef enum
{
  ANALOG_CH0,
  ANALOG_CH1,
  ANALOG_CH2,
  ANALOG_CH3,
  ANALOG_CH4,
  ANALOG_CH5,
  ANALOG_CH6,
  ANALOG_CH7,
  
  ANALOG_NUM_CHANNELS
} Analog_Channel_t;

// ***** Function Headers *****************************************************

// Initialization functions

void Analog_Initialize (Analog_Mode_t zeMode);

// Read Functions

void Analog_Update ();
signed long Analog_ReadCounts (Analog_Channel_t zeChannel);
float Analog_ReadVolts (Analog_Channel_t zeChannel);

#endif    // !defined _ANALOG_H
