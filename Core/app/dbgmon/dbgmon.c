//
// DBGMON.C
// Command Interpreter and Dispatcher
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>

#include "dbgmon.h"
#include "dbgmsg.h"

#include "sig_gen.h"


#define CMD_LEN_MAX  256     //Includes trailing NULL



typedef union
{
   int integer;
   float real;
   double long_real;
   char string[CMD_LEN_MAX];
} CommandLineArgument;


#define MAX_ARGUMENTS 5
static CommandLineArgument g_argument[MAX_ARGUMENTS];


static char g_szCmd [CMD_LEN_MAX];
static char g_szResult[CMD_LEN_MAX];


//RSW NOTE: In the C++ version on FM theremin I used the variadic function
//This is illegal in C, can't define a variadic function like this until C23, 
//must have at least one parameter before ... Using the variadic in C compiled,
//but the parameters passed into the dispatch function pointers were wrong
#ifndef __cplusplus
typedef int (*FunctionPointer)();
#else
typedef int (*FunctionPointer)(...);
#endif 

typedef struct {
   const char* szFormat;
   FunctionPointer pFunction;
   const char* szResultFormat;
   const char* szDescription;
} DISPATCH;







static void cmdDispatch(char * commandLine,const DISPATCH dispatchTableEntry[]);
static const DISPATCH* cmdLookup(char * command, const DISPATCH dispatchTable[]);
static int getParamCount(const char *p);
static int getTokenCount(const char *p);
static void help(void);
static void helpSpecific(char * command);
bool processLine(char * szLine, const DISPATCH * dispatchTable);

static inline void outPrompt(void) {db_outStr("$ ");}



//Debugging functions
static int cmd_set_freq(int *pFreq);
static int cmd_set_scale(float *pScale);
static int cmd_set_shape(char *pShape);


////////////////////////////////////////////////////////////
//System specific functions
////////////////////////////////////////////////////////////

static const DISPATCH dispatch_table[] =
{
   {" f %d", (FunctionPointer)cmd_set_freq, NULL, "Set waveform frequency (sample)"},
   {" s %c", (FunctionPointer)cmd_set_shape, NULL, "Set waveform shape (s=sine, w=sawtooth, t=triangle, q=square, l=silence, 1=sequence 1)"},
   {" sc %f", (FunctionPointer)cmd_set_scale, NULL, "Set waveform scale (0.0-1.0), 0.92 is good for my scope"},

   {" ", (FunctionPointer)NULL, NULL, ""},         //RSW - blank line? (for formatting only)

   {" info %s", (FunctionPointer) helpSpecific, "", "Show detailed help information"},

   /* This code is only here to provide the user with useful help  information.  The implimentation of the repeat
      function is actually carried out before the command string is passed to the evaluator.
    */
   {" !", (FunctionPointer) NULL, "", "repeat the last command"},
   {" ?", (FunctionPointer) help, "", "help summary"},
   {" h", (FunctionPointer) help, "", "help summary"},

   /*  zero-length szFormat string indicates end of table ... */
   {""},
};






void dbm_init(void)
{
   db_outStr("\r\nGood day, eh, and welcome to the STM32F4 Function Generator!\r\n");
   outPrompt();
}





inline void clearScreen(void)   {db_printf("\033[?25l\033[2J\033[;H");}  //NCURSES for hide cursor, clear screen and move cursor to upper left
inline void restoreCursor(void) {db_printf("\033[?25h");}                //Restore cursor




//Process a character from serial port
//Returns true if it runs a command
bool dbm_processChar(void)
{
   static char szLine[CMD_LEN_MAX];
   static char *pszHead=szLine;
   
   //Buffer up characters from the debug monitor input.   When there is a complete
   //line, send it to the command interpreter
   char c;
   if (!db_getChar(&c))
      return false;

   db_outChar(c);                            //Echo character to user

   if (c=='\n')                              //Assume lines end with \r\n, so ignore \n if we see one
      return false;

   //Handle backspaces
   if (c==127 || c=='\b') {
      if (pszHead>szLine)
         pszHead--;
      return false;
   }
   
   
   *pszHead = c;
   assert(pszHead-szLine < CMD_LEN_MAX);

   if (c!='\r') {
      *pszHead++ = c;                         //Store char and move on to the next
      return false;
   }

   //If we got a <CR>, process the command
   db_outStr("\r\n");                         //Echo <CR> to user
   *pszHead++ = '\0';                         //NULL terminate
   if (!processLine(szLine, dispatch_table))  //Dispatch the command
      db_outMsg("Error: command not understood");
         
   pszHead = szLine;                          //Reset the buffer
   outPrompt();
   return true;
}



bool processLine(char * szLine, const DISPATCH * dispatchTable)
{
   const DISPATCH *pDispatchEntry ;
   static char szLineLast[CMD_LEN_MAX];   //Process the ! repeat command

   // If the command is the ! character then replace the command with the previous command.
   if(strcmp(szLine,"!") == 0)
      strcpy(szLine, szLineLast);
   else
      strcpy(szLineLast, szLine);         // Otherwise save the current command in the static buffer.

   pDispatchEntry = cmdLookup (szLine, dispatchTable);
   if (pDispatchEntry != NULL) {
      cmdDispatch(szLine, pDispatchEntry);
      return true;
   }

   return false;
}




//RSW HACK - make a "process character function" here to pull one character at a time
//from USB, etc and build up a command...  Call dbm_processChar from loop in main.c
const DISPATCH* cmdLookup(char * szLine, const DISPATCH* pDispatchTable)
{
   char szMaybeCmd[CMD_LEN_MAX];

   /*  Scan the input line for the command. */
   sscanf(szLine, " %s ", szMaybeCmd);

   /*  Iterate through the lookup table.  (Zero-length command format string indicates end of table.) */
   while (strlen(pDispatchTable->szFormat) > 0) {
      /*  Scan the format string from the lookup table to extract the command.  */
      sscanf(pDispatchTable->szFormat, " %s ", g_szCmd);

      /*  If the input matches the table entry, then return the pointer to the table entry.  */
      if ((strlen(szMaybeCmd) == strlen(g_szCmd))
          && (strncmp(szMaybeCmd, g_szCmd, strlen(g_szCmd)) == 0))
      {
         return pDispatchTable ;
      }

      pDispatchTable++;
   }

   return NULL ;
}



void cmdDispatch(char* szLine, const DISPATCH* pDispatchTableEntry)
{
   FunctionPointer matchingFunctionPtr;
   const char* szFormat;
   int result;

   /*  Scan the format string from the lookup table to extract the command.  */
   sscanf(pDispatchTableEntry->szFormat, " %s ", g_szCmd);

   /*  Scan the rest of the input line per the format specified in the lookup table to get the arguments.  */

   /* Clear out the arguments */
   g_argument[0].integer = 0;
   g_argument[1].integer = 0;
   g_argument[2].integer = 0;
   g_argument[3].integer = 0;

   sscanf(szLine, pDispatchTableEntry->szFormat, &g_argument[0], &g_argument[1], &g_argument[2], &g_argument[3]);

   /* The number of tokens must be one fewer than the number of parameters */
   szFormat = pDispatchTableEntry->szFormat;
   if((getTokenCount(szLine) - 1) != getParamCount(szFormat))
      sprintf(g_szResult, "ERROR: expected %d parameters received %d", getParamCount(szFormat), getTokenCount(szLine) - 1);
   else {
      // Get the associated function from the table and call it with the arguments extracted from the input string.
      matchingFunctionPtr = pDispatchTableEntry->pFunction ;

      //Note: original implementation passed more parameters than needed most of the time.  This is undefined in C
      //but seemed to work on the compiler SeaMED was using.
      result = matchingFunctionPtr(&g_argument[0], &g_argument[1], &g_argument[2], &g_argument[3]);

      //Assume if result format is NULL, that the test function took care of displaying the results
      if (!pDispatchTableEntry->szResultFormat)
         return;

      sprintf(g_szResult, pDispatchTableEntry->szResultFormat, result);
   }
   db_outMsg(g_szResult);
}



void help(void)
{
   DISPATCH* pDispatchTable ;

   db_outMsg ("\r\nDebug Monitor Commands ...") ;

   pDispatchTable = (DISPATCH *)dispatch_table ;

   while (strlen(pDispatchTable->szFormat) > 0) {
      db_printf("%-20s", pDispatchTable->szFormat) ;
      db_outMsg(pDispatchTable->szDescription);
      ++pDispatchTable ;
   }
}



void helpSpecific (char * command)
{
   const DISPATCH* pDispatchTable;
   const DISPATCH* pDispatchEntry;

   if (!*command) {
      db_outMsg ("Error: Command not found");
      return ;
   }

   pDispatchTable = (DISPATCH*)dispatch_table;
   pDispatchEntry = cmdLookup(command, pDispatchTable);

   if (pDispatchEntry == NULL) {
      db_outMsg("Error: Command not found - try '?'");
      return ;
   }

   db_outMsg(pDispatchEntry->szFormat);
   db_outMsg(pDispatchEntry->szResultFormat);
   db_outMsg(pDispatchEntry->szDescription);
}





static int getParamCount(const char* p)
{
   int count = 0;

   while(*p != '\0') {
      if(*p == '%')
         count++;
      p++;
   }

   return count;
}




#define WHITE_SPACE 0
#define CHARACTER   1

static int getTokenCount(const char* p)
{
   int count = 0;
   int state = WHITE_SPACE;

   while (*p != '\r' && *p != '\0') {

      switch (state) {
         case WHITE_SPACE:
            if(*p != ' ') {        //Change from white to char is a new token 
               state = CHARACTER;
               count++;
            }
            break;
            
         case CHARACTER:
            if(*p == ' ')
               state = WHITE_SPACE;
            break;
      }
      p++;
   }

   return count;
}



////////////////////////////////////////////////////////////////////////////////////
// Test functions
////////////////////////////////////////////////////////////////////////////////////

static int cmd_set_freq(int* pFreq)
{
   if (fg_set_freq(*pFreq))
      db_printf("Set function generator freq to %d\r\n", *pFreq);
   else
      db_printf("ERR: Setting function generator freq to %d failed\r\n", *pFreq);

   return 0;
}



static int cmd_set_shape(char* pShape)
{
   FG_SHAPE shape;

   switch (*pShape) {
      case 'l': shape = FG_SILENCE;   break;
      case 's': shape = FG_SINE;      break;
      case 'w': shape = FG_SAWTOOTH;  break;
      case 't': shape = FG_TRIANGLE;  break;
      case 'q': shape = FG_SQUARE;    break;
      case '1': shape = FG_SEQUENCE1; break;

      default:
         db_printf("ERR: Illegal shape %c\r\n", *pShape);
         return false;
   }

   if (fg_set_shape(shape))
      db_printf("Set function generator shape to %c\r\n", *pShape);
   else
      db_printf("ERR: Setting function generator shape to %c failed\r\n", *pShape);

   return 0;
}

   

static int cmd_set_scale(float *pScale)
{
   if (fg_set_scale(*pScale))
      db_printf("Set function generator scale to %0.3f\r\n", (double)*pScale);
   else
      db_printf("ERR: Setting function generator scale to %0.3f failed\r\n", (double)*pScale);

   return 0;
}

