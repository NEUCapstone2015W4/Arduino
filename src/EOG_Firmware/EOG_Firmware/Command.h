/******************************************************************************
*
*    /file    Command.h
*
*    /desc    Header file for Command module.
*
*    /log     2/20/15  gcg - Initial release.
*
******************************************************************************/

#ifndef _COMMAND_H
#define _COMMAND_H

// ***** Definitions **********************************************************

// Command funtion callback

typedef void (*Command_Function_t)(String);

// Command Structure

typedef struct Command_s
{
  String snName;
  Command_Function_t spvCallback; 
} Command_t;

// ***** Function Headers *****************************************************

// Initialization Functions

void Command_Initialize(long zwBaud);
void Command_AddCmd(String znName, Command_Function_t zpvCallback);

#endif    // !defined _COMMAND_H
