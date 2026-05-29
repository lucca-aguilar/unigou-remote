/**
 * @file logger.cpp
 * @brief Implementation of the asynchronous logger module using FreeRTOS.
 */

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "logger.h"

static QueueHandle_t logs;
static const int max_logs = 40;
static void log_task(void* pvParameters);

/**
 * @brief Internal structure representing a single log entry packet.
 */
struct Log {
    char task[configMAX_TASK_NAME_LEN]; 
    char message[MAX_LOG_MSG_LEN];      
    uint32_t timestamp;                 
    Level level;                        
};

void log_init() {
logs = xQueueCreate(max_logs, sizeof(Log));
    if (logs == NULL) {
        Serial.println("FATAL: queue NULL");
        while(1);
    }
    Serial.println("Queue OK");

    if (xTaskCreate(log_task, "Logger", 256, NULL, 1, NULL) != pdPASS) {
        Serial.println("FATAL: logger task failed");
        while(1);
    }
    Serial.println("Logger task OK");
}

void log_write(Level level, const char* msg) {
    Log log;
    log.level = level;
    log.timestamp = xTaskGetTickCount();
    strncpy(log.message, msg, MAX_LOG_MSG_LEN - 1);
    log.message[MAX_LOG_MSG_LEN - 1] = '\0';
    strncpy(log.task, pcTaskGetName(NULL), configMAX_TASK_NAME_LEN - 1);
    log.task[configMAX_TASK_NAME_LEN - 1] = '\0';
    xQueueSend(logs, &log, pdMS_TO_TICKS(10));
}

/**
 * @brief Dedicated FreeRTOS task that consumes and outputs logs. This task runs in an infinite loop and remains in the Blocked state as long as the log queue is empty, consuming 0% CPU. When a log arrives, it wakes up, formats the data into a CSV-like string, and streams it over the hardware Serial interface.
 */
static void log_task(void* pvParameters) {
    static char buffer[256];
    const char* level_names[] = {"DEBUG", "OPERATION", "WARNING", "ERROR"};

    while(1) {
        Log log;
        xQueueReceive(logs, &log, portMAX_DELAY);
        snprintf(buffer, sizeof(buffer), "%lu,%s,%s,%s\n", log.timestamp, log.task, log.message, level_names[log.level]);
        Serial.print(buffer);
    }
}
