//
// DAC_DMA.C
// DMA supplied DAC
// On STM32F4 DISCO, DAC OUT2 outputs on PA5
//


#include <stdio.h>
#include <string.h>

#include "main.h"
#include "app.h"
#include "dac.h"
#include "sig_gen.h"







#define BUF_SAMPLE_COUNT (BLK_SAMPLE_COUNT * 2)   // Two blocks for ping-pong









// DAC DMA ping-pong buffer should be in D2, which the standard DMA controllers can handle
// (By default they will wind up in DTCM, which DMA1 cannot access)
// Also explicitly align to 32 bytes (good practice for AXI bus burst transfers)

// DAC Input Buffer
uint16_t dac_buffer[BLK_SAMPLE_COUNT*2] __attribute__((section(".d2_dma_buffers"), aligned(32)));


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
// generate_sine(&dac_buffer[0], BLK_SAMPLE_COUNT);                 // Fill 1st half
// generate_sawtooth(&dac_buffer[0], BLK_SAMPLE_COUNT);             // Fill 1st half
// generate_square(&dac_buffer[0], BLK_SAMPLE_COUNT);               // Fill 1st half
   generate_triangle(&dac_buffer[0], BLK_SAMPLE_COUNT);             // Fill 1st half
}



// DAC buffer empty interrupt
void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef *hdac)
{
   RESET_BLUE();
// generate_sine(&dac_buffer[BLK_SAMPLE_COUNT], BLK_SAMPLE_COUNT);      // Fill 2nd half
// generate_sawtooth(&dac_buffer[BLK_SAMPLE_COUNT], BLK_SAMPLE_COUNT);  // Fill 2nd half
// generate_square(&dac_buffer[BLK_SAMPLE_COUNT], BLK_SAMPLE_COUNT);    // Fill 2nd half
   generate_triangle(&dac_buffer[BLK_SAMPLE_COUNT], BLK_SAMPLE_COUNT);  // Fill 2nd half
}



void HAL_DACEx_ErrorCallbackCh2(DAC_HandleTypeDef *hdac)
{
   printf("ERR: Underrun on DAC channel 2\r\n");
}

