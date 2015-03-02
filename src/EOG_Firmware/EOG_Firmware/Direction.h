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
  DIRECTION_RIGHT,
  
  DIRECTION_MAX
} Direction_t;

// The channel state - contains the current and previous voltage

typedef struct Direction_Channel_s
{
  float       sfCurrVoltage;
  float       sfPrevVoltage;
} Direction_Channel_t;

// ***** Function Headers *****************************************************

// Initialization functions

void Direction_Initialize ();

// Update Funtions

void Direction_Update();

// Get Functions

Direction_t Direction_GetState();
void Direction_BroadcastState();

#endif    // !defined _DIRECTION_H
