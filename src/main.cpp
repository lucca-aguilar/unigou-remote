#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "logger.h"
#include "mem_monitor.h"
#include "runtime_stats.h"

static void Task1(void* pvParameters);
static void Task2(void* pvParameters);

void setup() {
    while (!Serial) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    log_init();
    mem_monitor_init();
    runtime_stats_init();

    Serial.println("BOOT");

    xTaskCreate(Task1, "Task1", 192, NULL, 2, NULL);
    xTaskCreate(Task2, "Task2", 192, NULL, 2, NULL);
}

void loop() {}

static void Task1(void* pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(200));
        log_write(OPERATION, "Task1 operating.");
    }
}

static void Task2(void* pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(600));
        log_write(OPERATION, "Task2 operating.");
    }
}