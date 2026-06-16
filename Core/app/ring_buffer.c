//
// RING_BUFFER.C
// Singleton ring buffer used to store historical ADC samples, for use in effects
// This is NOT a general purpose ring buffer.  Because of the way it is sequenced,
// i.e. ADC ISR writes a block then reads a block, no overrun or underrun checking
// is done.
//
// Samples are stored as 32 bit unbiased signed floats
//
// Call hooks:
//    rb_init during startup
//    rb_write_adc_blk from the ADC DMA interrupt callbacks
//    rb_read_dac_blk from the  ADC DMA interrupt callbacks


#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "app.h"
#include "ring_buffer.h"




#define BLK_COUNT     (SAMPLING_RATE_HZ/BLK_SAMPLE_COUNT * 1)   // Enought blocks to store one second of sound


// Variables that are accessed by both the ISRs and foreground are declared volatile
// to prevent them from being cached in a register, etc
static volatile int next_write_blk ;    // Next available block for ADC write
static volatile int next_read_blk;      // Next available block for DSP read
static float ring_buf[BLK_SAMPLE_COUNT * BLK_COUNT] __attribute__((section(".d1_axisram_dsp_buffers"), aligned(32)));


static inline void inc_read_blk(void)
{
   if (++next_read_blk >= BLK_COUNT)                // Increment read block index
      next_read_blk = 0;
}

static inline void inc_write_blk(void)
{
   if (++next_write_blk >= BLK_COUNT)               // Increment write block index
      next_write_blk = 0;
}



static inline float *getRingBlkAddr(int blk_num)
{
   assert(0<=blk_num && blk_num<BLK_COUNT);
   return &ring_buf[blk_num * BLK_SAMPLE_COUNT];
}



void rb_init(void)
{
#define SAMPLE_COUNT  (BLK_COUNT * BLK_SAMPLE_COUNT)   //Total number of samples in whole ring buf

   printf("Ring buffer SAMPLE_COUNT =%d\r\n", SAMPLE_COUNT);
   printf("Ring buffer BLK_COUNT    =%d\r\n", BLK_COUNT);
   printf("Ring buffer size         =%d\r\n", sizeof(ring_buf));

   //Start the ring buffer with the ADC sample block the same as the DAC block. (This only works
   //because we know the ADC ISR will write a block first, then read a block.  This is done to
   //minimize latency)  Eventually may need to start with a bigger gap between ADC and DAC, to
   //have time to do DSP
   next_write_blk=0;       // Next available block for ADC write to dry ring
   next_read_blk=0;        // Next available block for dry read (start with one block of silence so there are no errors)

   //Initialize entire ring buffer to silence
   for (int i=0; i<SAMPLE_COUNT; i++) 
      ring_buf[i] = 0;        // Start ring buffer with silence  (samples are stored as unbiased signed floats)
}





//Assumes block size is BLK_SIZE bytes, tries to write from ADC buf to next available block in the ring buffer,
// No check for overrun, since ring buffer is read and written on the same ISR
void rb_write_adc_blk(const uint16_t *p_adc_blk)
{
   float* p_ring_blk = getRingBlkAddr(next_write_blk);

   for (int i=0; i<BLK_SAMPLE_COUNT; i++) {
#ifndef USE_TEST_PULSE

      *p_ring_blk++ = (int)*p_adc_blk++ - ADC_MID;      // Converts from uint16_t to float, removes DC offset

#else
      //HACK, for testing with the single GPIO pulse, we want anything below 0 clamped to 0 in the float samples
      int s = *p_adc_blk++ - ADC_MID;
      if (s<0)
         s=0;
      *p_ring_blk++ = s;
#endif
   }

   inc_write_blk();
}



// Returns ptr to the next read block
// Must mark block read with rb_inc_dsp_blk when finished with this one
// For in place DSP and DAC write
float* rb_get_dsp_blk(void)
{
   return getRingBlkAddr(next_read_blk);
}


// Mark the current block read and increment to the next one
void rb_inc_dsp_blk(void)
{
   inc_read_blk();
}



//Get address for the block that is blk_offset before the next_read_blk
float* rb_get_previous_dsp_blk(int blk_offset)
{
   assert(0<=blk_offset && blk_offset<BLK_COUNT);

   int prev_blk = next_read_blk - blk_offset;
   if (prev_blk < 0)
      prev_blk += BLK_COUNT;

   return getRingBlkAddr(prev_blk);
}



//Increment a pointer within the ring buffer, taking wrap into account
float* rb_inc_ptr_with_wrap(float *p)
{
   assert(&ring_buf[0]<=p && p<=&ring_buf[BLK_COUNT*BLK_SAMPLE_COUNT - 1]);

   if (++p > &ring_buf[BLK_COUNT*BLK_SAMPLE_COUNT - 1])    //Check for wrap around
      p = &ring_buf[0];

   return p;
}



//Get address for the sample that is samp_offset samples before the next_read_blk's 1st sample
float* rb_get_previous_sample(int samp_offset)
{
   assert(0<=samp_offset && samp_offset<ELEMENTS_OF(ring_buf));

   int next_samp_index = next_read_blk*BLK_SAMPLE_COUNT;
   int prev_samp_index = next_samp_index - samp_offset;
   if (prev_samp_index < 0)
      prev_samp_index += ELEMENTS_OF(ring_buf);

   return &ring_buf[prev_samp_index];
}
