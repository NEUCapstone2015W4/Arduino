/******************************************************************************
*
*    /file    Direction.h
*
*    /desc    Header file for Direction module
*
*    /log     2/23/15  gcg - Initial release.
*
******************************************************************************/

#ifndef _DIRECTION_H
#define _DIRECTION_H

// ***** Definitions **********************************************************

// Different possible directions

typedef enum Direction_e
{
  DIRECTION_NONE,
  DIRECTION_UP,
  DIRECTION_DOWN,
  DIRECTION_LEFT,
  DIRECTION_RIGHT
} Direction_t;

// The direcition state - contains the last known state and the volatage
// reading from the appropriate signal

typedef struct Direction_State_s
{
  Direction_t seDirection;
  float       sfVoltage;
} Direction_State_t;

// ***** Function Headers *****************************************************

// Initialization functions

void Direction_Initialize ();

// Update Funtions

void Direction_Update();

// Get Functions

Direction_t Direction_GetState();

#endif    // !defined _DIRECTION_H
