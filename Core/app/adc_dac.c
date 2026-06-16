//
// DAC_DMA.C
// DMA supplied DAC
//


#include <stdio.h>
#include <string.h>

#include "main.h"
#include "app.h"
#include "adc.h"
#include "dac.h"
#include "ring_buffer.h"
#include "dsp.h"



//Uncomment to use the signal generator to output a sine wave to the DAC's DMA buffer, for testing 
//#define USE_SIGNAL_GENERATOR




#define BUF_SAMPLE_COUNT (BLK_SAMPLE_COUNT * 2)   // Two blocks for ping-pong




#ifdef USE_SIGNAL_GENERATOR
//Generates 440Hz sine wave, for testing
static void generate_test_signal(uint16_t *target_sub_buffer, uint16_t count)
{
   static float phase = 0.0f;

   // Calculate increment: 440Hz wave assuming a standard 48kHz trigger clock
   float phase_increment = 2.0f * 3.14159265f * 440.0f / (float)SAMPLING_RATE_HZ;

   for (int i=0; i < count; i++) {
      // 12-bit DAC mid-point is 2048. Amplitude of 1500 keeps it from clipping.
      float sample = 2048.0f + (1500.0f * sinf(phase));

      target_sub_buffer[i] = (uint16_t)sample;

      phase += phase_increment;
      if (phase >= 2.0f * 3.14159265f)
         phase -= 2.0f * 3.14159265f;
   }
}
#endif //USE_SIGNAL_GENERATOR




/////////////////////////////////////////////////////////////////////////////////////////////////
// Combination ADC and DAC stuff
/////////////////////////////////////////////////////////////////////////////////////////////////

// ADC and DAC DMA buffers should be in D2, which the standard DMA controllers can handle
// (By default they will wind up in DTCM, which DMA1 cannot access)
// Also explicitly align to 32 bytes (good practice for AXI bus burst transfers)

// ADC Output Buffer
uint16_t adc_buffer[BUF_SAMPLE_COUNT]__attribute__((section(".d2_dma_buffers"), aligned(32)));

// DAC Input Buffer
uint16_t dac_buffer[BLK_SAMPLE_COUNT*2] __attribute__((section(".d2_dma_buffers"), aligned(32)));




//Combination ADC and DAC init
//Called from main, after the generated peripheral initialization sequence
void adc_dac_init(void)
{
   rb_init();     //Initialize the ring buffer, used to store a string of ADC blocks

   // Before starting the ADC, fill the buffer with mid-scale silence (12-bit = 2048)
   // Just for safety and to make testing easier
   for (int i = 0; i < BUF_SAMPLE_COUNT; i++) {
      adc_buffer[i] = ADC_MID;
      dac_buffer[i] = DAC_MID;
   }

   // Calibrate the ADC for single-ended inputs
   if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK) 
      Error_Handler();       /* Calibration Error Handling */

   //Cast makes compiler happy, DMA setup in CubeMX tells DMA samples are half-word
   // Start ADC DMA, which will generate interrupts when it's half empty, etc...
   HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, BUF_SAMPLE_COUNT);

   // Clear any errors or flags that happened during boot
   __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_OVR | ADC_FLAG_EOC);


   // Start the DAC DMA, which will generate interrupts when it's half empty, etc...
   HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)dac_buffer, BUF_SAMPLE_COUNT, DAC_ALIGN_12B_R);

   // Don't need the HT/TC DAC DMA interrupts.   Can't turn these off in CubeMX, because the option is
   // greyed, so instead silence the specific DMA stream in code.
   // Clear the Half-Transfer and Transfer-Complete Interrupt Enable flags on the DAC's DMA stream
   __HAL_DMA_DISABLE_IT(hdac1.DMA_Handle1, DMA_IT_TC | DMA_IT_HT);
}



//Could maybe do this in the dsp module, not sure if it matters at this point
void do_dsp_on_next_dsp_blk(uint16_t *p_dac_buf)
{
   //Do DSP here, run data in place from ring buffer block into DAC, then mark block read
   //(HACK, for now just do ONE of these, may figure out how to chain them later)
// process_bandpass_filter_blk(p_dac_buf);
// process_digital_delay_blk(p_dac_buf);
   process_reverb_blk(p_dac_buf);

   rb_inc_dsp_blk();   //Mark block read
}



// ADC DMA buffer half full interrupt
// Because the ADC and DAC are running on the same timer, process the DSP,
// and the DAC DMA on this interrupt also 
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
   SET_DB0();

   //ADC processing 
   rb_write_adc_blk(&adc_buffer[0]);                //Copy first half to ring buffer

   //DSP/DAC processing
#ifndef USE_SIGNAL_GENERATOR
   //Do DSP here, run data in place from ring buffer block into DAC, then mark block read
   do_dsp_on_next_dsp_blk(&dac_buffer[0]);
#else
   generate_test_signal(&dac_buffer[0], BLK_SAMPLE_COUNT);  // Fill the 1st half
#endif

   RESET_DB0();
}



// ADC DMA buffer full interrupt
// Because the ADC and DAC are running on the same timer, process the DSP,
// and the DAC DMA on this interrupt also
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
   //ADC processing
   rb_write_adc_blk(&adc_buffer[BLK_SAMPLE_COUNT]); //Copy second half to ring buffer

   //DSP/DAC processing
#ifndef USE_SIGNAL_GENERATOR
   //Do DSP here, run data in place from ring buffer block into DAC, then mark block read
   do_dsp_on_next_dsp_blk(&dac_buffer[BLK_SAMPLE_COUNT]);
#else
   generate_test_signal(&dac_buffer[BLK_SAMPLE_COUNT], BLK_SAMPLE_COUNT);  //Fill 2nd half
#endif
}



void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    // If your code hits this breakpoint the moment you turn on the ADC,
    // an Overrun (OVR) or DMA Transfer Error has occurred.
   printf("ERR: ADC error callback\r\n");
}



void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac)
{
   printf("ERR: Underrun on DAC channel 1\r\n");
}

