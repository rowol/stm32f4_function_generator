//
// DBGMSG.C
// Debug output and input for STM32H7,
// with STLink's USB vcom port connected to UART3 on dev board
//

#include <stdio.h>
#include <stdbool.h>

#include "dbgmsg.h"
#include "app.h"


void db_outChar(char c)        {putchar(c);}
void db_outStr(const char* sz) {printf("%s", sz);}
void db_outMsg(const char* sz) {printf("%s\r\n", sz);}



#if 0
// Redirect getChar to USART3, which is connected to the USB virtual comm port on the STLink
extern UART_HandleTypeDef huart3;

bool db_getChar(char* p)
{
   __HAL_UART_CLEAR_OREFLAG(&huart3);      // Clear any existing overrun flags to prevent stuck status
   HAL_StatusTypeDef status = HAL_UART_Receive(&huart3, (uint8_t*)p, 1, 0);    //Returns immediately if no char available

   switch (status) {
      case HAL_OK:
         if (*p!=EOF)
            return true; // Char successfully received in *p
         return false;

      case  HAL_TIMEOUT:
         return false;   // Timeout occurred, no character received

      default:
         return false;   // Some other error
   }
}
#else

bool db_getChar(char* p)
{
   if (!is_usb_read_ready())
      return false;

   _read(0, p, 1);   //Read from stdin
//   scanf("%c", p);
   return true;
}

#endif






