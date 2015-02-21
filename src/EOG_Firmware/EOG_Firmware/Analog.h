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

// ***** Function Headers *****************************************************

#ifdef __cplusplus
extern "C" {
#endif

// Initialization functions

void Analog_Initialize (Analog_Mode_t);

#ifdef __cplusplus
}
#endif

#endif    // !defined _ANALOG_H
