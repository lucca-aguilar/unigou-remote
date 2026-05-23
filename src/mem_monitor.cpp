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
    xTaskCreate(mem_task, "Memory Monitor", 256, NULL, 1, NULL);
}

static void mem_task(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    char buf[80];

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, sampling_time);

        free_heap    = xPortGetFreeHeapSize();
        minimum_heap = xPortGetMinimumEverFreeHeapSize();

        snprintf(buf, sizeof(buf), "Free: %u B | Min ever: %u B",
            (unsigned)free_heap, (unsigned)minimum_heap);
        log_write(OPERATION, buf);

        UBaseType_t count = uxTaskGetSystemState(task_status, max_tasks, NULL);
        for (UBaseType_t i = 0; i < count; i++) {
            snprintf(buf, sizeof(buf), "Task %-12s | Stack HWM: %u words",
                task_status[i].pcTaskName,
                (unsigned)task_status[i].usStackHighWaterMark);
            log_write(OPERATION, buf);
        }
    }
}