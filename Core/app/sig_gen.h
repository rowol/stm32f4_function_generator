//
// SIG_GEN.H
// Signal generator functions
//

#pragma once

#include <stdint.h>
#include <stdbool.h>


typedef enum {
   FG_SINE=0,
   FG_SQUARE=1,
   FG_TRIANGLE=2,
   FG_SAWTOOTH=3,
} FG_SHAPE;

#define FG_SHAPE_MAX FG_SAWTOOTH



bool fg_set_freq(int freq_new);
bool fg_set_shape(FG_SHAPE shape_new);
void fg_fill_dac_buffer(uint16_t *buf, uint16_t count);


