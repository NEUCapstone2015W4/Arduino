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
*             3/17/15  gcg - Added Calibration commands.
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

#define DELTA_UP     0.0706f //0.068f
#define DELTA_DOWN   0.0706f //0.075f
#define DELTA_RIGHT  0.1f
#define DELTA_LEFT   0.1f

#define HORIZONTAL   ANALOG_CH5
#define VERTICAL     ANALOG_CH4

#define DEBUG        0

#define THRESHOLD_SCALEDOWN   0.9f

// Serial Direction characters

static String Direction__manSerialChars[DIRECTION_MAX];

// ***** Local Variables ******************************************************

// Detection thresholds

static float Direction__mafThreshold[DIRECTION_MAX];

// Resting Voltaages for both channels

static float Direction__mfUpDownResting;
static float Direction__mfLeftRightResting;

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

// Commands

static void Cmd__Idle(String znArg);
static void Cmd__Up(String znArg);
static void Cmd__Down(String znArg);
static void Cmd__Left(String znArg);
static void Cmd__Right(String znArg);
static void Cmd__Clear(String znArg);

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
  
  //Direction__mafThreshold[DIRECTION_NONE] = 0;
  //Direction__mafThreshold[DIRECTION_UP] = DELTA_UP;
  //Direction__mafThreshold[DIRECTION_DOWN] = DELTA_DOWN;
  //Direction__mafThreshold[DIRECTION_LEFT] = DELTA_LEFT;
  //Direction__mafThreshold[DIRECTION_RIGHT] = DELTA_RIGHT;
  
  // Set direction outputs
  
  Direction__manSerialChars[DIRECTION_NONE] = "i";
  Direction__manSerialChars[DIRECTION_UP] = "u";
  Direction__manSerialChars[DIRECTION_DOWN] = "d";
  Direction__manSerialChars[DIRECTION_LEFT] = "l";
  Direction__manSerialChars[DIRECTION_RIGHT] = "r";
  
  // The initial mode should always be looking straight ahead. Set both 
  // channels and direction state accordingly.
  
  Analog_Update();
  
  Direction__msUpDown.sfCurrVoltage = Analog_ReadVolts(VERTICAL);
  Direction__msUpDown.sfPrevVoltage = Analog_ReadVolts(VERTICAL);
  Direction__msUpDown.sfDeltaVoltage = 0;
  
  Direction__msLeftRight.sfCurrVoltage = Analog_ReadVolts(HORIZONTAL);
  Direction__msLeftRight.sfPrevVoltage = Analog_ReadVolts(HORIZONTAL);
  Direction__msLeftRight.sfDeltaVoltage = 0;
  
  Direction__meState = DIRECTION_NONE;
  
  // Add Direction Commands
  
  Command_AddCmd("i", Cmd__Idle);
  Command_AddCmd("u", Cmd__Up);
  Command_AddCmd("d", Cmd__Down);
  Command_AddCmd("l", Cmd__Left);
  Command_AddCmd("r", Cmd__Right);
  Command_AddCmd("clr", Cmd__Clear);
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
  Direction__msUpDown.sfDeltaVoltage += (Direction__msUpDown.sfCurrVoltage - 
                                         Direction__msUpDown.sfPrevVoltage);
  
  Direction__msLeftRight.sfPrevVoltage = Direction__msLeftRight.sfCurrVoltage;
  Direction__msLeftRight.sfCurrVoltage = Analog_ReadVolts(HORIZONTAL);
  Direction__msLeftRight.sfDeltaVoltage += 
  (Direction__msLeftRight.sfCurrVoltage - Direction__msLeftRight.sfPrevVoltage);    
  
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
       
       xfLeftRightWeight = Direction__WeightLeftRight();
       
     #if DEBUG  
         Serial.print("VERTICAL: ");
         Serial.print(Direction__msUpDown.sfDeltaVoltage, 4);
  
         Serial.print("    HORIZONTAL: ");
         Serial.print(Direction__msLeftRight.sfDeltaVoltage, 4);
  
         Serial.print("\r\n");  
     #endif
     
       // Decide which, if any, direction to assign
       
       if ((abs(xfUpDownWeight) < 1.0) && 
           (abs(xfLeftRightWeight) < 1.0))
       {
         
         // No direction detected
         
         //Direction__SetState(DIRECTION_NONE);
         
         break;
       }
       
       else if (abs(xfUpDownWeight) > abs(xfLeftRightWeight))
       {
         
         // UP-DOWN detected. A negative weight is a DOWN,
         // a positive weight is an UP.
         if (xfUpDownWeight < 0.0)
         {
           Direction__SetState(DIRECTION_DOWN);
         }
         else
         {
           Direction__SetState(DIRECTION_UP);
         }
       }
       
       else
       {
         
         // LEFT-RIGHT detected. A negative weight is a LEFT,
         // a positive weight is a RIGHT.
         
         if (xfLeftRightWeight < 0.0)
         {
           Direction__SetState(DIRECTION_LEFT);
         }
         else
         {
           Direction__SetState(DIRECTION_RIGHT);
         }
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
       
       xfDelta = abs(Direction__msUpDown.sfDeltaVoltage);
                     
       #if DEBUG 
         Serial.print("VERTICAL: ");
         Serial.print(Direction__msUpDown.sfDeltaVoltage, 4);
         
         Serial.print("     THRESHOLD: ");
         Serial.print(Direction__mafThreshold[DIRECTION_NONE], 4);
  
         Serial.print("\r\n");  
       #endif
                     
       // Check against threshold
       
       if (xfDelta < Direction__mafThreshold[DIRECTION_NONE])
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
       
       xfDelta = abs(Direction__msLeftRight.sfDeltaVoltage);
                     
       #if DEBUG  
         Serial.print("HORIZONTAL: ");
         Serial.print(Direction__msLeftRight.sfDeltaVoltage, 4);
         
         Serial.print("     THRESHOLD: ");
         Serial.print(Direction__mafThreshold[DIRECTION_NONE], 4);
  
         Serial.print("\r\n");  
       #endif
                     
       // Check against threshold
       
       if (xfDelta < Direction__mafThreshold[DIRECTION_NONE])
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
  
  if (Direction__msUpDown.sfDeltaVoltage  >= 0.0)
  {
    
    // Find the differential
    
    xfWeight = Direction__msUpDown.sfDeltaVoltage;
                   
    // Convert to a "weight" which will be >1 if the detection is over
    // the threshold.
    
    xfWeight = xfWeight / Direction__mafThreshold[DIRECTION_UP];  
  }
  
  // If the current is less than the previous, we are dealing with a
  // possible down.
  
  else
  {
    
    // Find the differential
    
    xfWeight = Direction__msUpDown.sfDeltaVoltage;
                   
    // Convert to a "weight" which will be >1 if the detection is over
    // the threshold.
    
    xfWeight = xfWeight / Direction__mafThreshold[DIRECTION_DOWN];
  }
  
  // Ignore states
  
  if (abs(xfWeight) > 2.5)
  {
    xfWeight = 0;
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
  
  if (Direction__msLeftRight.sfDeltaVoltage  >= 0.0)
  {
    
    // Find the differential
    
    xfWeight = Direction__msLeftRight.sfDeltaVoltage;
                   
    // Convert to a "weight" which will be >1 if the detection is over
    // the threshold.
    
    xfWeight = xfWeight / Direction__mafThreshold[DIRECTION_RIGHT];  
  }
  
  // If the current is less than the previous, we are dealing with a
  // possible LEFT.
  
  else
  {
    
    // Find the differential
    
    xfWeight = Direction__msLeftRight.sfDeltaVoltage;
                   
    // Convert to a "weight" which will be >1 if the detection is over
    // the threshold.
    
    xfWeight = xfWeight / Direction__mafThreshold[DIRECTION_LEFT]; 
  }
  
  // Ignore spikes
  
  if (abs(xfWeight) > 2.5)
  {
    xfWeight = 0;
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
    
    Direction__mafThreshold[DIRECTION_NONE] = 
                              Direction__mafThreshold[zeDir];    
  }
  
  // Finally update the state variable
  
  Direction__meState = zeDir;
  
  Direction_BroadcastState();
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
  
  Serial.println(Direction__manSerialChars[Direction__meState]);
}


// ***** Command Definitions **************************************************

/******************************************************************************
*
*    /name       Cmd__Idle
*
*    /purpose    Set the resting voltage
*
*    /ret        void
*
******************************************************************************/

static void Cmd__Idle(String znArg)
{
   Direction__mfUpDownResting = 0.0;
   Direction__mfLeftRightResting = 0.0;
   
   // First update the analog readings
  
   Analog_Update();
   
   // Set the Idle voltages for both the Horizontal and Vertical Circuits
   // Sample every half second for 3 seconds.
   
   for (int i=0; i<6; i++)
   {
     Direction__mfUpDownResting += Analog_ReadVolts(VERTICAL);
     Direction__mfLeftRightResting += Analog_ReadVolts(HORIZONTAL);
    
    delay(500);
   } 
   Direction__mfUpDownResting /= 6.0;
   Direction__mfLeftRightResting /= 6.0;
   
   #if DEBUG
      Serial.print("VER IDLE Threshold set to: ");
      Serial.print(Direction__mfUpDownResting, 4);
      
      Serial.print("    HOR IDLE Threshold set to: ");
      Serial.print(Direction__mfLeftRightResting, 4);
  
      Serial.print("\r\n");  
   #endif
   
   Serial.print("1\r\n");
}

/******************************************************************************
*
*    /name       Cmd__Up
*
*    /purpose    Set the UP threshold
*
*    /ret        void
*
******************************************************************************/

static void Cmd__Up(String znArg)
{
   float xfVoltage = 0.0;
  
   // First update the analog readings
  
   Analog_Update();
   
   // Read the current voltage, known UP due to application instructions
   // Sample every half second for 3 seconds.
   
   for (int i=0; i<6; i++)
   {
     xfVoltage += Analog_ReadVolts(VERTICAL);
    
    delay(500);
   } 
   xfVoltage /= 6.0;
   
   // Set the threshold to the positive differential between the current
   // reading and the known idle reading and scale down by the scale factor.
  
   Direction__mafThreshold[DIRECTION_UP] = 
          abs(xfVoltage - Direction__mfUpDownResting) * THRESHOLD_SCALEDOWN;
          
   // Check for bad threshold
   
   if (Direction__mafThreshold[DIRECTION_UP] == 0)
   {
     Serial.println("Bad reading! 0V!");
     
     Serial.print("0\r\n");
     return;
   }
          
   // DEBUG - Print the setting
   
   #if DEBUG
         Serial.print("UP Threshold set to: ");
         Serial.print(Direction__mafThreshold[DIRECTION_UP], 4);
  
         Serial.print("\r\n");  
   #endif
   
   Serial.print("1\r\n");
}

/******************************************************************************
*
*    /name       Cmd__Down
*
*    /purpose    Set the DOWN threshold
*
*    /ret        void
*
******************************************************************************/

static void Cmd__Down(String znArg)
{
   float xfVoltage = 0.0;
  
   // First update the analog readings
  
   Analog_Update();
   
   // Read the current voltage, known DOWN due to application instructions
   // Sample every half second for 3 seconds.
   
   for (int i=0; i<6; i++)
   {
     xfVoltage += Analog_ReadVolts(VERTICAL);
    
    delay(500);
   } 
   xfVoltage /= 6.0;
   
   
   // Set the threshold to the positive differential between the current
   // reading and the known idle reading and scale down by the scale factor.
  
   Direction__mafThreshold[DIRECTION_DOWN] = 
          abs(xfVoltage - Direction__mfUpDownResting) * THRESHOLD_SCALEDOWN;
          
   // Check for bad threshold
   
   if (Direction__mafThreshold[DIRECTION_DOWN] == 0)
   {
     Serial.println("Bad reading! 0V!");
     
     Serial.print("0\r\n");
     return;
   }
          
   // DEBUG - Print the setting
   
   #if DEBUG
         Serial.print("DOWN Threshold set to: ");
         Serial.print(Direction__mafThreshold[DIRECTION_DOWN], 4);
  
         Serial.print("\r\n");  
   #endif
   
   Serial.print("1\r\n");
}

/******************************************************************************
*
*    /name       Cmd__Left
*
*    /purpose    Set the LEFT threshold
*
*    /ret        void
*
******************************************************************************/

static void Cmd__Left(String znArg)
{
   float xfVoltage = 0.0;
  
   // First update the analog readings
  
   Analog_Update();
   
   // Read the current voltage, known DOWN due to application instructions
   // Sample every half second for 3 seconds.
   
   for (int i=0; i<6; i++)
   {
     xfVoltage += Analog_ReadVolts(HORIZONTAL);
    
    delay(500);
   } 
   xfVoltage /= 6.0;
   
   
   // Set the threshold to the positive differential between the current
   // reading and the known idle reading and scale down by the scale factor.
  
   Direction__mafThreshold[DIRECTION_LEFT] = 
          abs(xfVoltage - Direction__mfLeftRightResting) * THRESHOLD_SCALEDOWN;
          
   // Check for bad threshold
   
   if (Direction__mafThreshold[DIRECTION_LEFT] == 0)
   {
     Serial.println("Bad reading! 0V!");
     
     Serial.print("0\r\n");
     return;
   }
          
   // DEBUG - Print the setting
   
   #if DEBUG
         Serial.print("LEFT Threshold set to: ");
         Serial.print(Direction__mafThreshold[DIRECTION_LEFT], 4);
  
         Serial.print("\r\n");  
   #endif
   
   Serial.print("1\r\n");
}

/******************************************************************************
*
*    /name       Cmd__Right
*
*    /purpose    Set the RIGHT threshold
*
*    /ret        void
*
******************************************************************************/

static void Cmd__Right(String znArg)
{
   float xfVoltage = 0.0;
  
   // First update the analog readings
  
   Analog_Update();
   
   // Read the current voltage, known DOWN due to application instructions
   // Sample every half second for 3 seconds.
   
   for (int i=0; i<6; i++)
   {
     xfVoltage += Analog_ReadVolts(HORIZONTAL);
    
    delay(500);
   } 
   xfVoltage /= 6.0;
   
   // Set the threshold to the positive differential between the current
   // reading and the known idle reading and scale down by the scale factor.
  
   Direction__mafThreshold[DIRECTION_RIGHT] = 
          abs(xfVoltage - Direction__mfLeftRightResting) * THRESHOLD_SCALEDOWN;
          
   // Check for bad threshold
   
   if (Direction__mafThreshold[DIRECTION_RIGHT] == 0)
   {
     Serial.println("Bad reading! 0V!");
     
     Serial.print("0\r\n");
     return;
   }
          
   // DEBUG - Print the setting
   
   #if DEBUG
         Serial.print("RIGHT Threshold set to: ");
         Serial.print(Direction__mafThreshold[DIRECTION_RIGHT], 4);
  
         Serial.print("\r\n");  
   #endif
   
   Serial.print("1\r\n");
}

/******************************************************************************
*
*    /name       Cmd__Clear
*
*    /purpose    Clear the deltas, reset to idle state
*
*    /ret        void
*
******************************************************************************/

static void Cmd__Clear(String znArg)
{
  
  // The initial mode should always be looking straight ahead. Set both 
  // channels and direction state accordingly.
  
  Analog_Update();
  
  Direction__msUpDown.sfCurrVoltage = Analog_ReadVolts(VERTICAL);
  Direction__msUpDown.sfPrevVoltage = Analog_ReadVolts(VERTICAL);
  Direction__msUpDown.sfDeltaVoltage = 0;
  
  Direction__msLeftRight.sfCurrVoltage = Analog_ReadVolts(HORIZONTAL);
  Direction__msLeftRight.sfPrevVoltage = Analog_ReadVolts(HORIZONTAL);
  Direction__msLeftRight.sfDeltaVoltage = 0;
  
  Direction__meState = DIRECTION_NONE;
}

