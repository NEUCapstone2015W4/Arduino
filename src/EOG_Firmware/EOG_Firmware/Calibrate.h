/******************************************************************************
*
*    /file    Calibrate.h
*
*    /desc    Header file for Calibration module.
*
*    /log     3/16/15  gcg - Initial release.
*
******************************************************************************/

#ifndef _CALIBRATION_H
#define _CALIBRATION_H

// ***** Definitions **********************************************************

// Different possible calibration states

typedef enum Cal_State_e
{
  CALIBRATION_NONE,
  CALIBRATION_RECAL,
  
  CALIBRATION_OK
} Cal_State_t;

// ***** Function Headers *****************************************************

// Initialization functions

void Calibration_Initialize ();

// Get Functions

Cal_State_t Calibration_GetState();
boolean Calibration_CheckState();

#endif    // !defined _CALIBRATION_H
