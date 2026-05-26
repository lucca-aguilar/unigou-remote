#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "logger.h"
#include "runtime_stats.h"

static char stats_buffer[2048];

static void runtime_stats_task(void* pvParameters);

static uint32_t start_micros = 0;

extern "C" void runtime_stats_timer_init() {
    start_micros = micros();
}

extern "C" uint32_t runtime_stats_timer_get_value() {
    return micros() - start_micros; 
}

void runtime_stats_init() {
     if (xTaskCreate(runtime_stats_task, "Runtime stats", 256, NULL, 1, NULL) != pdPASS) {
        Serial.println("FATAL: Runtime stats task failed");
        while(1);
    }
    Serial.println("Runtime stats task OK");
}

static void runtime_stats_task(void* pvParameters) {
    TickType_t LastWakeTime = xTaskGetTickCount();
    
    while(1) {
        vTaskDelayUntil(&LastWakeTime, pdMS_TO_TICKS(5000));

        memset(stats_buffer, 0, sizeof(stats_buffer));

        Serial.println("\n=================================");
        Serial.println("        CPU USAGE STATISTICS       ");
        Serial.println("=================================");
        
        vTaskGetRunTimeStats(stats_buffer);
        Serial.print(stats_buffer);
    }
}


