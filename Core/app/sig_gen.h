//
// SIG_GEN.H
// Signal generator functions
//

#pragma once

#include <stdint.h>
#include <stdbool.h>


bool fg_set_freq(int freq_new);


void generate_sine(uint16_t *target_sub_buffer, uint16_t count);
void generate_sawtooth(uint16_t *target_sub_buffer, uint16_t count);
void generate_triangle(uint16_t *target_sub_buffer, uint16_t count);
void generate_square(uint16_t *target_sub_buffer, uint16_t count);



