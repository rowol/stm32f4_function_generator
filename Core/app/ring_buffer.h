//
// RING_BUFFER.H
//

#pragma once

void rb_init(void);

void rb_write_adc_blk(const uint16_t *p_blk);

float* rb_get_dsp_blk(void);    //For in place DSP and DAC write
void rb_inc_dsp_blk(void);

float* rb_get_previous_dsp_blk(int blk_offset);

float* rb_inc_ptr_with_wrap(float *p);
float* rb_get_previous_sample(int samp_offset);

