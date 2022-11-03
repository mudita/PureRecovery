#pragma once

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** Initialize submodule delay */
void delay_init(void);

/** Get jiffies in ms when timer start */
uint32_t get_jiffiess(void);

/** Sleep amount time of milisecons
 * @param delay Amount delay of the ms
 */
void msleep(uint32_t delay);

static inline uint32_t jiffiess_timer_diff(uint32_t t1, uint32_t t2) {
    if (t2 >= t1)  //Not overflow
        return t2 - t1;
    else       //Overflow
        return t1 - t2;
}

#ifdef __cplusplus
}
#endif

