/******************************************************************************
*
*    /file    Direction.cpp
*
*    /desc    The direction module sits atop the Analog module and translates
*             the ADC readings to diections that the application can use. This
*             is transfer is facilitated via the defined commands.
*
*             The direction module is also responsible for the management of
*             the various calibrations and detection thresholds needed to
*             translate Voltage -> Motion
*
*    /log     2/23/15  gcg - Initial release.
*
******************************************************************************/

// ***** Include Files ********************************************************

// Arduino Source

#include <Arduino.h>

// Local Modules

#include "Analog.h"
#include "Command.h"

// ***** Local Definitions ****************************************************

// DEBUG - For now, all detection thresholds are constant values, in Volts

#define DELTA_UP     0.060
#define DELTA_DOWN   0.030
#define DELTA_RIGHT  0.090
#define DELTA_LEFT   0.070

// ***** Local Variables ******************************************************

// Direction state

// ***** Local Funtions *******************************************************

// ***** Function Definitions *************************************************

// ***** Command Definitions **************************************************




