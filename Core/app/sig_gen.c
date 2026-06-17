//
// SIG_GEN.C
// Signal generator functions
// Sine, triange, sawtooth, and square
//


#include <stdio.h>
#include <math.h>

#include "app.h"
#include "sig_gen.h"



//For 12 bit internal DAC
static const float F_DAC_MAX = (float)DAC_MAX;
static const float F_DAC_MID = (float)DAC_MID;

// To prevent clipping (was seeing some due to loading from my 1M scope probe?)
// slightly scale the output.   The signal generation functions generate as
// an offset from F_DAC_MID, so the peak and the base are scaled by this factor.
// In addition to the output peak limit, I also noticed with scope probe attached,
// signal doesn't go down much below 63mV... setting the scale also prevents having
// flat spots on the bottom of the triangle, sine, etc..
static const float F_SCALE   = 0.92f;





// Module parameters
static float    g_freq  = 440.0f;    //Startup frequency
static FG_SHAPE g_shape = FG_SINE;   //Startup shape



bool fg_set_freq(int freq_new)
{
   if (1.0f > freq_new || freq_new > 10000.0f) {
      printf("ERR: Illegal frequency try 1hz - 10Khz\r\n");
      return false;
   }

   //HACK, should should maybe reset the phase, etc...
   g_freq = freq_new;
   return true;
}



bool fg_set_shape(FG_SHAPE shape_new)
{
   if (!(0 <= shape_new && shape_new <= FG_SHAPE_MAX)) {
      printf("ERR: Illegal shape, try 0-%d\r\n", FG_SHAPE_MAX);
      return false;
   }

   //HACK, should should maybe reset the phase, etc...
   g_shape = shape_new;
   return true;
}






//Phase ranges from 0 - 2Pi
static void generate_sine(uint16_t *target_sub_buffer, uint16_t count)
{
   static float phase = 0.0f;

   // Calculate increment: 440Hz wave assuming a standard 48kHz trigger clock
   float phase_increment = 2.0f * 3.14159265f * g_freq / (float)SAMPLING_RATE_HZ;

   for (int i=0; i < count; i++) {
      // 12-bit DAC mid-point is 2048. Amplitude of 1500 keeps it from clipping.
      float sample = F_DAC_MID + (F_DAC_MID * sinf(phase) * F_SCALE);

      target_sub_buffer[i] = (uint16_t)sample;

      phase += phase_increment;
      if (phase >= 2.0f * 3.14159265f)
         phase -= 2.0f * 3.14159265f;
   }
}



// "Phase" ranges from -0.5 - 0.5
static void generate_sawtooth(uint16_t *target_sub_buffer, uint16_t count)
{
   static float phase = -0.5f;

   float phase_increment = g_freq / (float)SAMPLING_RATE_HZ;

   for (int i=0; i < count; i++) {
      float sample = F_DAC_MID + F_DAC_MAX * phase * F_SCALE;

      target_sub_buffer[i] = (uint16_t)sample;

      phase += phase_increment;
      if (phase >= 0.5f)
         phase -= 1.0f;
   }
}



// "Phase" ranges from -2 to 2
static void generate_triangle(uint16_t *target_sub_buffer, uint16_t count)
{
   static float phase = -2.0f;

   float phase_increment = g_freq*4 / (float)SAMPLING_RATE_HZ;        // Because phase range is quadrupled

   for (int i=0; i < count; i++) {
      float sample;
      if (phase < 0.0f)
         sample = F_DAC_MID + F_DAC_MID * (1.0f + phase) * F_SCALE;   // Upward half
      else 
         sample = F_DAC_MID + F_DAC_MID * (1.0f - phase) * F_SCALE;   // Downward half

      target_sub_buffer[i] = (uint16_t)sample;

      phase += phase_increment;
      if (phase >= 2.0f)
         phase -= 4.0f;
   }
}



// "Phase" ranges from 0 to 1
static void generate_square(uint16_t *target_sub_buffer, uint16_t count)
{
   static float phase = 0.0f;

   float phase_increment = g_freq / (float)SAMPLING_RATE_HZ;

   for (int i=0; i < count; i++) {
      float sample;
      if (phase < 0.5f)
         sample = F_DAC_MID + F_DAC_MID*F_SCALE;
      else
         sample = F_DAC_MID - F_DAC_MID*F_SCALE;

      target_sub_buffer[i] = (uint16_t)sample;

      phase += phase_increment;
      if (phase >= 1.0f)
         phase -= 1.0f;
   }
}



void fg_fill_dac_buffer(uint16_t *buf, uint16_t count)
{
   // Choose the correct fill routine based on the current shape setting
   switch (g_shape) { 
      case FG_SINE:      generate_sine(buf, count);      break;
      case FG_SQUARE:    generate_square(buf, count);    break;
      case FG_TRIANGLE:  generate_triangle(buf, count);  break;
      case FG_SAWTOOTH:  generate_sawtooth(buf, count);  break;

      default:
         printf("ERR: illegal wave shape\r\n");
         break;
   }
}
