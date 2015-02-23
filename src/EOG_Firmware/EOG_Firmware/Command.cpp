/******************************************************************************
*
*    /file    Command.c
*
*    /desc    The Command module creates a very simple command interface via
*             the Serial port through witch a user or application can talk to
*             the device via ASCII commands. 
*
*             Any command strings are registered with the module at init by 
*             the programmer. This table is then walked when the serial port 
*             registers a newline character and executed with any provided 
*             arguments. This will allow dynamic addition of funtionality as 
*             needed across the development process.
*
*    /log     2/20/15  gcg - Initial release.
*
******************************************************************************/

// ***** Include Files ********************************************************

// Arduino Source

#include <Arduino.h>

// Local Modules

#include "Command.h"

// ***** Local Definitions ****************************************************

// Max number of commands

#define MAX_NUM_CMDS     64
#define CMD_BUFFER_SIZE  200

// ***** Local Variables ******************************************************

// Command Count

static unsigned int Command__muhCmdCount;

// Command Table

static Command_t Command__masCommands[MAX_NUM_CMDS];

// Command buffer

static String Command_mnCmdBuffer;

// ***** Local Funtions *******************************************************

static int Command__GetCmd(String znCmd);

// Commands

static void Cmd__Test(String znArg);

// ***** Function Definitions *************************************************

/******************************************************************************
*
*    /name       Command_Initialize
*
*    /purpose    Begins Serial communications at the supplied baud rate.
*                Registers any needed commands.
*
*    /param[in]  zuwBaud    Baud rate to set. Allowable rates:
*                           - 300          - 14400
*                           - 600          - 19200
*                           - 1200         - 28800
*                           - 2400         - 38400
*                           - 4800         - 57600
*                           - 9600         - 115200
*
*    /ret        void
*
******************************************************************************/
void Command_Initialize(long zwBaud)
{
  
  // Clear the command table
  
  Command__muhCmdCount = 0;
  
  for (int i=0; i<MAX_NUM_CMDS; i++)
  {
    
    Command__masCommands[i] = (Command_t){"", NULL};      
  }
  
  // Clear the buffer
  
  Command_mnCmdBuffer.reserve(CMD_BUFFER_SIZE);
  Command_mnCmdBuffer = "";
  
  // Add any commands
  
  Command_AddCmd("test", Cmd__Test);
  
  // Open Serial communications
  
  Serial.begin(zwBaud);
}

/******************************************************************************
*
*    /name       Command_AddCmd
*
*    /purpose    Add the given command to the command table.
*                NOTE: The index is not checked against the maximum here. This
*                      is because it can easily be increased and I do not
*                      expect us to have many commands.
*
*    /param[in]  znName       The name of the command
*    /param[in]  znCallback   The command function
*
*    /ret        void
*
******************************************************************************/
void Command_AddCmd(String znName, Command_Function_t zpvCallback)
{
 
  // Add the given command to the command table, increment the index
  
  Command__masCommands[Command__muhCmdCount] = 
                                    (Command_t){znName, zpvCallback};
                                    
  Command__muhCmdCount++;
}


/******************************************************************************
*
*    /name       Command__GetCmd
*
*    /purpose    Searches the command table for the given command. Returns
*                the index of that command, or -1 if it was not found.
*
*    /param[in]  znCmd    The requested command
*
*    /ret        int      The index of the given command, -1 if it was not
*                         found in the table.
*
******************************************************************************/
static int Command__GetCmd (String znCmd)
{
  int xwIndex = -1;
  
  // Search for the requested command
  
  for (int i=0; i<Command__muhCmdCount; i++)
  {
    
    // Check the command name
   
    if (znCmd.equals(Command__masCommands[i].snName))
    {
      xwIndex = i;
      break;
    }
  }
  
  return xwIndex;
}

/******************************************************************************
*
*    /name       Command_Execute
*
*    /purpose    Searches the command table for the given string, executes
*                its associated callback.
*
*    /param[in]  znCmd    The given command
*    /param[in]  znArg    The given Argument (if applicable)
*
*    /ret        void
*
******************************************************************************/
void Command_Execute (String znCmd, String znArg)
{
  int xhCmdIndex;
  
  // Try to find the given command
  
  xhCmdIndex = Command__GetCmd(znCmd);
  
  // Execute or display error
  
  if (xhCmdIndex == -1)
  {
    Serial.println("Invalid Command!");
  }
  else
  {
    Command__masCommands[xhCmdIndex].spvCallback(znArg);
  }
}

/******************************************************************************
*
*    /name       serialEvent
*
*    /purpose    Arduino internal funtion, occurs in between "loops" and
*                whenever a new character is recieved. Echos the character,
*                and executes the command if needed.
*
*    /ret        void
*
******************************************************************************/
void serialEvent() {
  
  // Check for multiple characters
  
  while (Serial.available()) {
   
    // Get the new byte
    
    char xcInChar = (char)Serial.read();
    
    // Add it to the buffer
    
    if (isalnum(xcInChar))
    {
      Command_mnCmdBuffer += xcInChar;
    }
    
    // Print the the character
    
    Serial.print(xcInChar);
    
    // If the incoming character is a newline, attempt
    // to execute it
    
    if (xcInChar == '\n') 
    {
      Command_Execute(Command_mnCmdBuffer, "");
      Command_mnCmdBuffer = "";
    } 
  }
}

// ***** Command Definitions *************************************************
