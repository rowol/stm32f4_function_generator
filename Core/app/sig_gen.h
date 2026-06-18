//
// SIG_GEN.H
// Signal generator functions
//

#pragma once

#include <stdint.h>
#include <stdbool.h>


typedef enum {
   FG_SILENCE=0,
   FG_SINE=1,
   FG_SQUARE=2,
   FG_TRIANGLE=3,
   FG_SAWTOOTH=4,
   FG_SEQUENCE1=5,
} FG_SHAPE;

#define FG_SHAPE_MAX FG_SEQUENCE1



bool fg_set_freq(int freq_new);
bool fg_set_shape(FG_SHAPE shape_new);
void fg_fill_dac_buffer(uint16_t *buf, uint16_t count);


