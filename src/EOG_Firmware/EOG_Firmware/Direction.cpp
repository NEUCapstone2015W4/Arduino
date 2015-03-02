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
#include "Direction.h"

// ***** Local Definitions ****************************************************

// DEBUG - For now, all detection thresholds are constant values, in Volts

#define DELTA_UP     0.060
#define DELTA_DOWN   0.030
#define DELTA_RIGHT  0.090
#define DELTA_LEFT   0.070

#define HORIZONTAL   ANALOG_CH5
#define VERTICAL     ANALOG_CH4

// Serial Direction characters

static const String Direction__mkanSerialChars[DIRECTION_MAX] =
{
  "i",
  "u",
  "d",
  "l",
  "r"
};

// ***** Local Variables ******************************************************

// Detection thresholds

static float Direction__mafThreshold[DIRECTION_MAX];

// Channel structures - store currently detected and previous voltage by 
// channel.

static Direction_Channel_t Direction__msUpDown;
static Direction_Channel_t Direction__msLeftRight;

// Direction state - master direction state. This is what will be transmitted
// to the application and also dictates which channel has priority.

static Direction_t Direction__meState;

// ***** Local Funtions *******************************************************

// Weight functions. A value greater than 1 indicaties a detection.

static float Direction__WeightUpDown();
static float Direction__WeightLeftRight();

// Update direction state. Saves of the delta required to drop back to
// idle.

static void Direction__SetState(Direction_t zeDir);

// ***** Function Definitions *************************************************

/******************************************************************************
*
*    /name       Direction_Initialize
*
*    /purpose    Initializes the direction module. Currently only initializes
*                the local static variables used to track the direction
*                state.
*
*    /ret        void
*
******************************************************************************/
void Direction_Initialize ()
{
  
  // DEBUG - Set the thresholds to the predefined values.
  
  Direction__mafThreshold[DIRECTION_NONE] = 0;
  Direction__mafThreshold[DIRECTION_UP] = DELTA_UP;
  Direction__mafThreshold[DIRECTION_DOWN] = DELTA_DOWN;
  Direction__mafThreshold[DIRECTION_LEFT] = DELTA_LEFT;
  Direction__mafThreshold[DIRECTION_RIGHT] = DELTA_RIGHT;
  
  // The initial mode should always be looking straight ahead. Set both 
  // channels and direction state accordingly.
  
  Direction__msUpDown.sfCurrVoltage = Analog_ReadVolts(VERTICAL);
  Direction__msUpDown.sfPrevVoltage = Analog_ReadVolts(VERTICAL);
  
  Direction__msLeftRight.sfCurrVoltage = Analog_ReadVolts(HORIZONTAL);
  Direction__msLeftRight.sfPrevVoltage = Analog_ReadVolts(HORIZONTAL);
  
  Direction__meState = DIRECTION_NONE;
}

/******************************************************************************
*
*    /name       Direction_Update
*
*    /purpose    The direction update function behaves differently depending
*                on the direction state. If no direction is detected, it
*                checks both channels for a possible detection. In the event
*                of a direction detection on BOTH channels, it weights the
*                differentials and selects the "heaviest" detection.
*
*                If a direction is currently detected, the update funtion only
*                cares about detecting a return to idle for that channel.
*
*    /ret        void
*
******************************************************************************/
void Direction_Update()
{
  
  // First update the analog readings
  
  Analog_Update();
  
  // Update channel structures
  
  Direction__msUpDown.sfPrevVoltage = Direction__msUpDown.sfCurrVoltage;
  Direction__msUpDown.sfCurrVoltage = Analog_ReadVolts(VERTICAL);
  
  Direction__msLeftRight.sfPrevVoltage = Direction__msLeftRight.sfCurrVoltage;
  Direction__msLeftRight.sfCurrVoltage = Analog_ReadVolts(HORIZONTAL);
  
  // Behave according to current Direction state
  
  switch (Direction__meState)
  {
    
    // If no direction is detected, attempt to identify one
    
    case DIRECTION_NONE:
    {
       float xfUpDownWeight, xfLeftRightWeight;
       
       // Check UP-DOWN
       
       xfUpDownWeight = Direction__WeightUpDown();
       
       // Check LEFT-RIGHT
       
       xfUpDownWeight = Direction__WeightLeftRight();
       
       // Decide which, if any, direction to assign
       
       if ((abs(xfUpDownWeight) < 1.0) && 
           (abs(xfLeftRightWeight) < 1.0))
       {
         
         // No direction detected
         
         Direction__SetState(DIRECTION_NONE);
       }
       
       else if (abs(xfUpDownWeight) > abs(xfLeftRightWeight))
       {
         
         // UP-DOWN detected. A negative weight is a DOWN,
         // a positive weight is an UP.
         
         Direction__SetState((xfUpDownWeight < 0.0) ? 
                                   DIRECTION_DOWN : DIRECTION_UP);
       }
       
       else
       {
         
         // LEFT-RIGHT detected. A negative weight is a LEFT,
         // a positive weight is a RIGHT.
         
         Direction__SetState((xfLeftRightWeight < 0.0) ? 
                                DIRECTION_LEFT : DIRECTION_RIGHT);
       }
      
       break; 
    }
    
    // If UP was read, check for a drop back to idle.
    // If DOWN was read, check for a rise back to idle.
     
    case DIRECTION_UP:
    case DIRECTION_DOWN:
    {
       float xfDelta;
       
       // Determine delta
       
       xfDelta = abs(Direction__msUpDown.sfCurrVoltage - 
                     Direction__msUpDown.sfPrevVoltage);
                     
       // Check against threshold
       
       if (xfDelta >= Direction__mafThreshold[DIRECTION_NONE])
       {
          
         // Threshold exceeded
         
         Direction__SetState(DIRECTION_NONE);
       }
       
       break; 
    }
    
    // If LEFT was read, check for a rise back to idle.
    // If RIGHT was read, check for a drop back to idle. 
    
    case DIRECTION_LEFT:
    case DIRECTION_RIGHT:
    {
       float xfDelta;
       
       // Determine delta
       
       xfDelta = abs(Direction__msLeftRight.sfCurrVoltage - 
                     Direction__msLeftRight.sfPrevVoltage);
                     
       // Check against threshold
       
       if (xfDelta >= Direction__mafThreshold[DIRECTION_NONE])
       {
          
         // Threshold exceeded
         
         Direction__SetState(DIRECTION_NONE);
       }
       
       break; 
    }
  }
  
}

/******************************************************************************
*
*    /name       Direction__WeightUpDown
*
*    /purpose    Returns a weight for the detection status of the channel. A
*                value greater than 1 (positive or negative) indicates a
*                detection. Negative weights correspond to DOWN and positive
*                weights correspond to UP.
*
*    /ret        float    Current weighted detection status
*
******************************************************************************/
static float Direction__WeightUpDown()
{
  float xfWeight = 0.0f;
  
  // If the current is greater than the previous, we are dealing with a
  // possible up. Since it is irrelevant where we include the "equal" case,
  // we'll put it here.
  
  if (Direction__msUpDown.sfCurrVoltage  >= Direction__msUpDown.sfPrevVoltage)
  {
    
    // Find the differential
    
    xfWeight = abs(Direction__msUpDown.sfCurrVoltage -
                   Direction__msUpDown.sfPrevVoltage);
                   
    // Convert to a "weight" which will be >1 if the detection is over
    // the threshold.
    
    xfWeight = xfWeight / Direction__mafThreshold[DIRECTION_UP];  
  }
  
  // If the current is less than the previous, we are dealing with a
  // possible down.
  
  else
  {
    
    // Find the differential
    
    xfWeight = abs(Direction__msUpDown.sfCurrVoltage -
                   Direction__msUpDown.sfPrevVoltage);
                   
    // Convert to a "weight" which will be >1 if the detection is over
    // the threshold.
    
    xfWeight = xfWeight / Direction__mafThreshold[DIRECTION_DOWN];
   
    // Flip the sign, as negative weights correspond to DOWN
    
    xfWeight = xfWeight * -1.0f;
  }
  
  // Return the result
  
  return xfWeight;
}

/******************************************************************************
*
*    /name       Direction__WeightLeftRight
*
*    /purpose    Returns a weight for the detection status of the channel. A
*                value greater than 1 (positive or negative) indicates a
*                detection. Negative weights correspond to LEFT and positive
*                weights correspond to RIGHT.
*
*    /ret        float    Current weighted detection status
*
******************************************************************************/
static float Direction__WeightLeftRight()
{
  float xfWeight = 0.0f;
  
  // If the current is greater than the previous, we are dealing with a
  // possible RIGHT. Since it is irrelevant where we include the "equal" case,
  // we'll put it here.
  
  if (Direction__msLeftRight.sfCurrVoltage  >= 
      Direction__msLeftRight.sfPrevVoltage)
  {
    
    // Find the differential
    
    xfWeight = abs(Direction__msLeftRight.sfCurrVoltage -
                   Direction__msLeftRight.sfPrevVoltage);
                   
    // Convert to a "weight" which will be >1 if the detection is over
    // the threshold.
    
    xfWeight = xfWeight / Direction__mafThreshold[DIRECTION_RIGHT];  
  }
  
  // If the current is less than the previous, we are dealing with a
  // possible LEFT.
  
  else
  {
    
    // Find the differential
    
    xfWeight = abs(Direction__msLeftRight.sfCurrVoltage -
                   Direction__msLeftRight.sfPrevVoltage);
                   
    // Convert to a "weight" which will be >1 if the detection is over
    // the threshold.
    
    xfWeight = xfWeight / Direction__mafThreshold[DIRECTION_LEFT]; 
   
    // Flip the sign, as negative weights correspond to DOWN
    
    xfWeight = xfWeight * -1.0f;
  }
  
  // Return the result
  
  return xfWeight;
}

/******************************************************************************
*
*    /name       Direction__SetState
*
*    /purpose    Sets a the direction state. Updates the delta required to
*                return to the idle state.
*
*    /ret        void
*
******************************************************************************/
static void Direction__SetState(Direction_t zeDir)
{
  
  // Set the delta accordingly, stored in the DIRECTION_NONE entry of
  // the Direction__mafThreshold array.
  
  if (zeDir == DIRECTION_NONE)
  {
   
    // Clear entry to 0
  
    Direction__mafThreshold[DIRECTION_NONE] = 0;
  }
  else
  {
    
    // Update to match current state. This may be removed, but for now
    // we set it again here as we may need to adjust the delta as
    // the use time progresses and the voltage drifts.
    
    Direction__mafThreshold[DIRECTION_NONE] = Direction__mafThreshold[zeDir];    
  }
  
  // Finally update the state variable
  
  Direction__meState == zeDir;
}

/******************************************************************************
*
*    /name       Direction_GetState
*
*    /purpose    Return the current state.
*
*    /ret        Direction_t    The current state
*
******************************************************************************/
Direction_t Direction_GetState()
{
  
  // Simply return the value of the state variable
  
  return Direction__meState;
}

/******************************************************************************
*
*    /name       Direction_BroadcastState
*
*    /purpose    Broadcasts the current state variable to the serial port.
*                DIRECTION_NONE  = "i\r\n"
*                DIRECTION_UP    = "u\r\n"
*                DIRECTION_DOWN  = "d\r\n"
*                DIRECTION_LEFT  = "l\r\n"
*                DIRECTION_RIGHT = "r\r\n"     
*
*    /ret        void
*
******************************************************************************/
void Direction_BroadcastState()
{
  
  // Send the apropriate character. Using println for the automatic
  // \r\n.
  
  Serial.println(Direction__mkanSerialChars[Direction__meState]);
}


// ***** Command Definitions **************************************************




