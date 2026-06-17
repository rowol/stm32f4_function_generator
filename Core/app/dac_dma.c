//
// DAC_DMA.C
// DMA supplied DAC
// On STM32F4 DISCO, DAC OUT2 outputs on PA5
//


#include <stdio.h>
#include <string.h>

#include "main.h"
#include "dac.h"

#include "app.h"
#include "dac_dma.h"
#include "sig_gen.h"







#define BUF_SAMPLE_COUNT (BLK_SAMPLE_COUNT * 2)   // Two blocks for ping-pong







// DAC DMA ping-pong buffer.  The 32 byte alignment is a carry over from when this was in
// H7 code. The H7's D-cache line size is 32 bytes.  It is not necessary on the F4, the
// alignment just needs to be 2 or 4, since the DAC transfer size is 16 bits.   Leaving it
// at 32 just means if I reuse the code in an H7 project later, stuff won't break.
// This was also good practice for H7 AXI bus burst transfers

uint16_t dac_buffer[BLK_SAMPLE_COUNT*2] __attribute__((aligned(32)));


//Combination ADC and DAC init
//Called from main, after the generated peripheral initialization sequence
void dac_init(void)
{
   // Before starting the ADC, fill the buffer with mid-scale silence (12-bit = 2048)
   // Just for safety and to make testing easier
   for (int i = 0; i < BUF_SAMPLE_COUNT; i++) 
      dac_buffer[i] = DAC_MID;

   // Start the DAC DMA, which will generate interrupts when it's half empty, etc...
   HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t*)dac_buffer, BUF_SAMPLE_COUNT, DAC_ALIGN_12B_R);
}




// DAC DMA buffer half empty interrupt
void HAL_DACEx_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef *hdac)
{
   SET_BLUE();
   fg_fill_dac_buffer(&dac_buffer[0], BLK_SAMPLE_COUNT);                 // Fill 1st half
   RESET_BLUE();
}



// DAC buffer empty interrupt
void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef *hdac)
{
   SET_BLUE();
   fg_fill_dac_buffer(&dac_buffer[BLK_SAMPLE_COUNT], BLK_SAMPLE_COUNT);  // Fill 2nd half
   RESET_BLUE();
}



void HAL_DACEx_ErrorCallbackCh2(DAC_HandleTypeDef *hdac)
{
   printf("ERR: Underrun on DAC channel 2\r\n");
}

