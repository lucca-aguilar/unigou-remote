/**
 * @file mem_monitor.cpp
 * @brief Implementation of the periodic memory and task stack monitoring subsystem.
 */

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "logger.h"
#include "mem_monitor.h"

static const TickType_t sampling_time = pdMS_TO_TICKS(200);
static const int max_tasks = 10;

static size_t free_heap = 0;
static size_t minimum_heap = 0;
static TaskStatus_t task_status[max_tasks];

static void mem_task(void* pvParameters);

void mem_monitor_init() {
    if (xTaskCreate(mem_task, "Memory", 256, NULL, 1, NULL) != pdPASS) {
        Serial.println("FATAL: memory task failed");
        while(1);
    }
    Serial.println("Memory task OK");
}

/**
 * @brief FreeRTOS task that periodically profiles memory and task stack safety. Running as a strict periodic task via `vTaskDelayUntil`, this function captures the current heap status and uses `uxTaskGetSystemState` to dynamically list all operational tasks, printing their remaining stack space.
 */
static void mem_task(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    char buffer[80];

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, sampling_time);

        free_heap    = xPortGetFreeHeapSize();
        minimum_heap = xPortGetMinimumEverFreeHeapSize();

        snprintf(buffer, sizeof(buffer), "Free: %u B | Min ever: %u B",
            (unsigned)free_heap, (unsigned)minimum_heap);
        log_write(OPERATION, buffer);

        UBaseType_t count = uxTaskGetSystemState(task_status, max_tasks, NULL);
        if (count > max_tasks) {
            count = max_tasks;
        }

        for (UBaseType_t i = 0; i < count; i++) {
            snprintf(buffer, sizeof(buffer), "Task %-12s | Stack HWM: %u words",
                task_status[i].pcTaskName,
                (unsigned)task_status[i].usStackHighWaterMark);
            log_write(OPERATION, buffer);
        }
    }
}