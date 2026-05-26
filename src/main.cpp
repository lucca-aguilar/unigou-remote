#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "logger.h"
#include "mem_monitor.h"
#include "task_tracer.h"
#include "runtime_stats.h"
#include "context_switches.h"

static void Task1(void* pvParameters);
static void Task2(void* pvParameters);

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);          
    }

    log_init();
    /*mem_monitor_init();
    tracer_init();
    runtime_stats_init();*/
    context_switches_init();

    Serial.println("BOOT");

    if (xTaskCreate(Task1, "Task1", 100, NULL, 2, NULL) != pdPASS) {
        Serial.println("FATAL: Task1 create failed");
        while (1);
    }

    if (xTaskCreate(Task2, "Task2", 100, NULL, 2, NULL) != pdPASS) {
        Serial.println("FATAL: Task2 create failed");
        while (1);
    }

    vTaskStartScheduler();
}

void loop() {}

static void Task1(void* pvParameters) {
    while (1) {
        tracer_event("Task1", TASK_RUNNING);
        log_write(OPERATION, "Task1 operating.");
        tracer_event("Task1", TASK_BLOCKED);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

static void Task2(void* pvParameters) {
    while (1) {
        tracer_event("Task2", TASK_RUNNING);
        log_write(OPERATION, "Task2 operating.");
        tracer_event("Task2", TASK_BLOCKED);
        vTaskDelay(pdMS_TO_TICKS(600));
    }
}