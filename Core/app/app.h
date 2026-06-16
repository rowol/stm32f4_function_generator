//
// APP.H
//

#pragma once

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




#define ELEMENTS_OF(arr)  (sizeof(arr)/sizeof(arr[0]))



void app_init1(void);
void app_init2(void);

void app_loop_bottom(void);

void CDC_Receive_FS_hook(uint8_t* Buf, uint32_t Len);
bool is_usb_read_ready(void);
int _read(int file, char *ptr, int len);



