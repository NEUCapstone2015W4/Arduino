/******************************************************************************
*
*    /file    Calibrate.cpp
*
*    /desc    Module containing all of the needed commands to calibrate the 
*             EOG Application. The calibration processess is handled by 5
*             commands that let the Arduino application know to sample for
*             the given threshold or state. These commands are defined in
*             the Direction module as they primarily exist to set the local
*             statics in that file.
*
*             The Calibration state is set so the application knows to wait
*             for calibration or re-calibrate.
*
*    /log     3/16/15  gcg - Initial release.
*
******************************************************************************/

// ***** Include Files ********************************************************

// Arduino Source

#include <SPI.h>
#include <Arduino.h>

// Local Modules

#include "Analog.h"
#include "Command.h"
#include "Calibrate.h"

// ***** Local Definitions ****************************************************

#define DEBUG        0

// ***** Local Variables ******************************************************

// The calibration state of the application.

static Cal_State_t Calibrate__meCalState;

// ***** Local Funtions *******************************************************

static void Cmd__Ok(String znArg);
static void Cmd__ReCal(String znArg);

// ***** Function Definitions *************************************************

/******************************************************************************
*
*    /name       Calibration_Initialize
*
*    /purpose    Sets the initial Calibrations state. Adds any relevant
*                Commands.
*
*    /ret        void
*
******************************************************************************/
void Calibration_Initialize()
{

  // Set the Calibration state

  Calibrate__meCalState = CALIBRATION_NONE;
  
  // Add commands
  
  Command_AddCmd("ok", Cmd__Ok);
  Command_AddCmd("recal", Cmd__ReCal);
}

/******************************************************************************
*
*    /name       Calibration_GetState
*
*    /purpose    Returns the current Calibration state
*
*    /ret        Cal_State_t    The current calibration state
*
******************************************************************************/
Cal_State_t Calibration_GetState()
{
  
  // Return the Calibration State
  
  return Calibrate__meCalState;
}

/******************************************************************************
*
*    /name       Calibration_CheckState
*
*    /purpose    Sets the initial Calibrations state. Adds any relevant
*                Commands.
*
*    /ret        boolean    true if Calibration is OK, false otherwise
*
******************************************************************************/
boolean Calibration_CheckState()
{
  
  // Any non-OK state is a bad calibration
  
  return Calibrate__meCalState == CALIBRATION_OK;
}


// ***** Command Definitions **************************************************

/******************************************************************************
*
*    /name       Cmd__Ok
*
*    /purpose    Set the RIGHT threshold
*
*    /ret        void
*
******************************************************************************/
static void Cmd__Ok(String znArg)
{
  
  // Set the Calibration State to OK
  
  Calibrate__meCalState = CALIBRATION_OK;
}

/******************************************************************************
*
*    /name       Cmd__ReCal
*
*    /purpose    Set the RIGHT threshold
*
*    /ret        void
*
******************************************************************************/
static void Cmd__ReCal(String znArg)
{
  
  // Set the Calibration State to ReCal
  
  Calibrate__meCalState = CALIBRATION_RECAL;
}
