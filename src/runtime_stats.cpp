#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "runtime_stats.h"

static void runtime_task(void* pvParameters);

extern "C" {
    void vConfigureTimerForRunTimeStats(void) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT    = 0;
        DWT->CTRL      |= DWT_CTRL_CYCCNTENA_Msk;
    }

    uint32_t ulGetRunTimeCounterValue(void) {
        return DWT->CYCCNT;
    }
}

void runtime_stats_init() {
    xTaskCreate(runtime_task, "Runtime Stats Task", 512, NULL, 1, NULL);
}

static void runtime_task(void* pvParameters) {
    TickType_t LastWakeTime = xTaskGetTickCount();

    while(1) {
        vTaskDelayUntil(&LastWakeTime, pdMS_TO_TICKS(5000));
        static char buffer[1000];
        vTaskGetRunTimeStats(buffer);
        Serial.print(buffer);
    }
}