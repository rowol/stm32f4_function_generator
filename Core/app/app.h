//
// APP.H
//

#pragma once

#include <stdint.h>
#include <stdbool.h>



//Useful constants
#define SAMPLING_RATE_HZ   48000

// Number of samples in each ADC/DAC DMA and ring buffer block
// If this number is increased, also be sure to update the MPU region 0
// size setting in CubeMX and regenerate code (for MPU_Config.)  Currently
// set to 1K, so count could grow to 128 before there are any problems
#define BLK_SAMPLE_COUNT      64



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



