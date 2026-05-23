#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "logger.h"

QueueHandle_t messages;
static const int max_messages = 100;
static void logging(void* pvParameters);

struct Log {
    char task[configMAX_TASK_NAME_LEN];
    char message[MAX_LOG_MSG_LEN];
    uint32_t timestamp;
    Level level;
};

void log_init() {
    Serial.begin(115200);
    messages = xQueueCreate(max_messages, sizeof(Log));
    xTaskCreate(logging, "Logger", 256, NULL, 1, NULL);
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

static void logging(void* pvParameters) {
    static char buffer[256];
    const char* level_names[] = {"DEBUG", "OPERATION", "WARNING", "ERROR"};

    while(1) {
        Log log;
        xQueueReceive(messages, &log, portMAX_DELAY);
        snprintf(buffer, sizeof(buffer), "%lu,%s,%s,%s\n", log.timestamp, log.task, log.message, level_names[log.level]);
    }

    Serial.print(buffer);
}