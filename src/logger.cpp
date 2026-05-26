#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "logger.h"

static QueueHandle_t messages;
static const int max_messages = 10;
static void log_task(void* pvParameters);

struct Log {
    char task[configMAX_TASK_NAME_LEN];
    char message[MAX_LOG_MSG_LEN];
    uint32_t timestamp;
    Level level;
};

void log_init() {
messages = xQueueCreate(max_messages, sizeof(Log));
    if (messages == NULL) {
        Serial.println("FATAL: queue NULL - heap insuficiente");
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
    xQueueSend(messages, &log, pdMS_TO_TICKS(10));
}

void serial_print_guarded(const char* line) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        vTaskSuspendAll();
        Serial.print(line);
        xTaskResumeAll();
    } else {
        Serial.print(line);
    }
}

void serial_println_guarded(const char* line) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        vTaskSuspendAll();
        Serial.println(line);
        xTaskResumeAll();
    } else {
        Serial.println(line);
    }
}

static void log_task(void* pvParameters) {
    static char buffer[256];
    const char* level_names[] = {"DEBUG", "OPERATION", "WARNING", "ERROR"};

    while(1) {
        Log log;
        xQueueReceive(messages, &log, portMAX_DELAY);
        snprintf(buffer, sizeof(buffer), "%lu,%s,%s,%s\n", log.timestamp, log.task, log.message, level_names[log.level]);
        serial_print_guarded(buffer);
    }
}
