//
// APP.H
//

#pragma once

#include <stdint.h>
#include <stdbool.h>



//Useful constants

// Use a sampling rate that divides nicely into the 84Mhz timer 2 clock
// The higher the sampling rate, the nicer the higher frequency waveforms
// will look (before filtering)   The max effective DAC conversion rate
// is something like 1M
// (Does not need to be a "standard sampling rate")
#define SAMPLING_RATE_HZ   400000

// Number of samples in each DAC DMA ping-pong buffer block
// At 400Khz samping rate, a 512 sample block gives a 1.28ms ISR window
// At 48Khz sampling rate, a  64 sample block gives a 1.33ms ISR window
// (The sine wave generation is currently using about 50% of the ISR window)
#define BLK_SAMPLE_COUNT     512


//For 12 bit internal DAC 
#define DAC_MAX       ((1<<12)-1)
#define DAC_MID       (DAC_MAX/2)



//Debug/timing pins for use with scope.  On STM32F4 DISCO:
//PD15 is blue LED
//PD14 is red LED 
#define SET_BLUE()    HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_SET)
#define RESET_BLUE()  HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET)
#define SET_RED()     HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_SET)
#define RESET_RED()   HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_RESET)



#define ELEMENTS_OF(arr)  (sizeof(arr)/sizeof(arr[0]))



void app_init1(void);
void app_init2(void);

void app_loop_bottom(void);

void CDC_Receive_FS_hook(uint8_t* Buf, uint32_t Len);
bool is_usb_read_ready(void);
int _read(int file, char *ptr, int len);



