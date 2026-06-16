//
// DBGMSG.H
// Debug input and output
//

#pragma once

#include <stdio.h>
#include <stdbool.h>


#define db_printf(...)          printf(__VA_ARGS__)

void db_outChar(char c);
void db_outStr(const char* sz);
void db_outMsg(const char* sz);   //Adds \r\n

bool db_getChar(char* p);

