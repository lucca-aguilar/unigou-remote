#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "runtime_stats.h"

void vConfigureTimerForRunTimeStats(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT    = 0;
    DWT->CTRL      |= DWT_CTRL_CYCCNTENA_Msk;
}