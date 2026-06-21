//
// SIG_GEN.C
// Signal generator functions
// Sine, triange, sawtooth, square, silence, and custom sequences
//


#include <stdio.h>
#include <math.h>

#include "main.h"
#include "app.h"
#include "sig_gen.h"



//For 12 bit internal DAC
static const float F_DAC_MAX = (float)DAC_MAX;
static const float F_DAC_MID = (float)DAC_MID;





// Module parameters (modified with fg_set_ functions below)
static float    g_freq  = 880.0f;          // Startup frequency
static FG_SHAPE g_shape = FG_SINE;         // Startup shape

// To prevent clipping (was seeing some due to loading from my 1M scope probe?)
// slightly scale the output.   The signal generation functions generate as
// an offset from F_DAC_MID, so the peak and the base are scaled by this factor.
// In addition to the output peak limit, I also noticed with scope probe attached,
// signal doesn't go down much below 63mV... setting the scale also prevents having
// flat spots on the bottom of the triangle, sine, etc..
static float g_scale = 0.92f;




//Phase ranges from 0 - 2Pi
static void generate_sine(uint16_t *target_sub_buffer, uint16_t count)
{
   static float phase = 0.0f;

   if (!target_sub_buffer) {   //Reset the state machine
      phase = 0.0f;
      return;
   }

   
   // Calculate increment: 440Hz wave assuming a standard 48kHz trigger clock
   float phase_increment = 2.0f * 3.14159265f * g_freq / (float)SAMPLING_RATE_HZ;

   for (int i=0; i < count; i++) {
      // 12-bit DAC mid-point is 2048. Amplitude of 1500 keeps it from clipping.
      float sample = F_DAC_MID + (F_DAC_MID * sinf(phase) * g_scale);

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

   if (!target_sub_buffer) {   //Reset state 
      phase = -0.5;
      return;
   }

   
   for (int i=0; i < count; i++) {
      float sample = F_DAC_MID + F_DAC_MAX * phase * g_scale;

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

   if (!target_sub_buffer) {   //Reset state 
      phase = -2.0f;
      return;
   }

   
   for (int i=0; i < count; i++) {
      float sample;
      if (phase < 0.0f)
         sample = F_DAC_MID + F_DAC_MID * (1.0f + phase) * g_scale;   // Upward half
      else 
         sample = F_DAC_MID + F_DAC_MID * (1.0f - phase) * g_scale;   // Downward half

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


   if (!target_sub_buffer) {   //Reset state 
      phase = 0;
      return;
   }

   
   for (int i=0; i < count; i++) {
      float sample;
      if (phase < 0.5f)
         sample = F_DAC_MID + F_DAC_MID*g_scale;
      else
         sample = F_DAC_MID - F_DAC_MID*g_scale;

      target_sub_buffer[i] = (uint16_t)sample;

      phase += phase_increment;
      if (phase >= 1.0f)
         phase -= 1.0f;
   }
}



// "Phase" ranges from 0 to 1
static void generate_silence(uint16_t *target_sub_buffer, uint16_t count)
{
   if (!target_sub_buffer)               //Reset state 
      return;

   for (int i=0; i < count; i++) 
      target_sub_buffer[i] = DAC_MID;
}




//Custom sequence
// call with buffer null ptr to reset state
static void generate_sequence1(uint16_t *target_sub_buffer, uint16_t count)
{
   // Wait 100ms, generate a 5ms sine modulated pulse, then silence.
   // After one second, repeat
   static uint32_t pulse_tmr_start;
   static int state = 0;

   if (!target_sub_buffer) {   //Reset the state machine
      state = 0;
      return;
   }
   
   switch (state) {
      case 0:
         generate_sine(NULL, 0);                        // Reset sine state
         generate_silence(target_sub_buffer, count);
         pulse_tmr_start = HAL_GetTick();               // 1ms timer
         state++;
         break;

      case 1:
         generate_silence(target_sub_buffer, count);
         if (HAL_GetTick() - pulse_tmr_start > 100) 
            state++;
         break;

      case 2:
         generate_sine(target_sub_buffer, count);
         if (HAL_GetTick() - pulse_tmr_start > 105) 
            state++;
         break;

      case 3:   //Restart after 1 second
         generate_silence(target_sub_buffer, count);         
         if (HAL_GetTick() - pulse_tmr_start > 1000) 
            state=0;
         break;

   }
}



bool fg_set_freq(int freq_new)
{
   // The upper frequency limit is somewhat arbitrary, that's about where "audio" ends.
   // The sampling rate is high enough that you could probably run higher frequencies
   // if desired.
   if (1 > freq_new || freq_new > 22000) {
      printf("ERR: Illegal frequency, try 1 - 22000 hz\r\n");
      return false;
   }

   //HACK, should maybe reset the phase, etc...  not sure we care
   g_freq = freq_new;
   return true;
}



bool fg_set_shape(FG_SHAPE shape_new)
{
   if (0 > shape_new || shape_new > FG_SHAPE_MAX) {
      printf("ERR: Illegal shape, try 0 - %d\r\n", FG_SHAPE_MAX);
      return false;
   }
   
   switch (shape_new) {
      case FG_SINE:      generate_sine(NULL, 0);      break;   //Reset state
      case FG_SQUARE:    generate_square(NULL, 0);    break;   //Reset state
      case FG_TRIANGLE:  generate_triangle(NULL, 0);  break;   //Reset state
      case FG_SAWTOOTH:  generate_sawtooth(NULL, 0);  break;   //Reset state         
      case FG_SEQUENCE1: generate_sequence1(NULL, 0); break;   //Reset the sequence state machine
         
      default:
         break;
   }  
   
   g_shape = shape_new;
   return true;
}



bool fg_set_scale(float scale)
{
   if (0.0f > scale || scale > 1.0f) {
      printf("ERR: Illegal scale %0.3f, try 0.0 - 1.0\r\n", (double)scale);
      return false;
   }

   g_scale = scale;
   return true;
}



void fg_fill_dac_buffer(uint16_t *buf, uint16_t count)
{
   // Choose the correct fill routine based on the current shape setting
   switch (g_shape) { 
      case FG_SILENCE:   generate_silence(buf, count);         break;
      case FG_SINE:      generate_sine(buf, count);            break;
      case FG_SQUARE:    generate_square(buf, count);          break;
      case FG_TRIANGLE:  generate_triangle(buf, count);        break;
      case FG_SAWTOOTH:  generate_sawtooth(buf, count);        break;
      case FG_SEQUENCE1: generate_sequence1(buf, count);       break;

      default:
         printf("ERR: illegal wave shape\r\n");
         break;
   }
}
