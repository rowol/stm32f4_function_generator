//
// SIG_GEN.C
// Signal generator functions
// Sine, triange, sawtooth, and square
//


#include <math.h>

#include "app.h"
#include "sig_gen.h"



static const float F_DAC_MAX = (1<<12) - 1;
static const float F_DAC_MID = F_DAC_MAX/2.0f;
static const float F_SCALE   = 0.92f;               // To prevent clipping (was seeing some due to loading from
                                                    // my 1M scope probe?) slightly scale the output

static float freq = 440.0f;



//Generates 440Hz sine wave, for testing
void generate_sine(uint16_t *target_sub_buffer, uint16_t count)
{
   static float phase = 0.0f;

   // Calculate increment: 440Hz wave assuming a standard 48kHz trigger clock
   float phase_increment = 2.0f * 3.14159265f * freq / (float)SAMPLING_RATE_HZ;

   for (int i=0; i < count; i++) {
      // 12-bit DAC mid-point is 2048. Amplitude of 1500 keeps it from clipping.
      float sample = F_DAC_MID + (F_DAC_MID * sinf(phase) * F_SCALE);

      target_sub_buffer[i] = (uint16_t)sample;

      phase += phase_increment;
      if (phase >= 2.0f * 3.14159265f)
         phase -= 2.0f * 3.14159265f;
   }
}



//Generates 440Hz sine wave, for testing
void generate_sawtooth(uint16_t *target_sub_buffer, uint16_t count)
{
   static float phase = 0.0f;

   // Calculate increment: 440Hz wave assuming a standard 48kHz trigger clock
   float phase_increment = freq / (float)SAMPLING_RATE_HZ;

   for (int i=0; i < count; i++) {
      float sample = F_DAC_MAX * phase * F_SCALE;

      target_sub_buffer[i] = (uint16_t)sample;

      phase += phase_increment;
      if (phase >= 1.0f)
         phase -= 1.0f;
   }
}



void generate_triangle(uint16_t *target_sub_buffer, uint16_t count)
{
   static float phase = 0.0f;

   // Calculate increment: 440Hz wave assuming a standard 48kHz trigger clock
   float phase_increment = freq / (float)SAMPLING_RATE_HZ;

   for (int i=0; i < count; i++) {
      float sample;
      if (phase < 0.5f)
         sample = F_DAC_MAX * phase*2 * F_SCALE;
      else 
         sample = F_DAC_MAX * (1.0f - (phase-0.5f)*2) * F_SCALE;

      target_sub_buffer[i] = (uint16_t)sample;

      phase += phase_increment;
      if (phase >= 1.0f)
         phase -= 1.0f;
   }
}



void generate_square(uint16_t *target_sub_buffer, uint16_t count)
{
   static float phase = 0.0f;

   // Calculate increment: 440Hz wave assuming a standard 48kHz trigger clock
   float phase_increment = freq / (float)SAMPLING_RATE_HZ;

   for (int i=0; i < count; i++) {
      float sample;
      if (phase < 0.5f)
         sample = F_DAC_MAX *F_SCALE;
      else
         sample = 0;

      target_sub_buffer[i] = (uint16_t)sample;

      phase += phase_increment;
      if (phase >= 1.0f)
         phase -= 1.0f;
   }
}

