//
// DBGMSG.C
// Debug output and input for STM32F4 Disco
//
// Uses the virtual com port on USB device connector (the micro usb connector, not the STLink connector)
// (Most of the STDOUT/STDIN remapping is done in app.c)
//

#include <stdio.h>
#include <stdbool.h>

#include "dbgmsg.h"
#include "app.h"


void db_outChar(char c)        {putchar(c);}
void db_outStr(const char* sz) {printf("%s", sz);}
void db_outMsg(const char* sz) {printf("%s\r\n", sz);}



bool db_getChar(char* p)
{
   if (!is_usb_read_ready())
      return false;

   _read(0, p, 1);   //Read from stdin
   return true;
}







