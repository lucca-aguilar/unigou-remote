#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <string.h>
#include "logger.h"
#include "context_switches.h"

#define SWITCHES_QUEUE_SIZE 30
extern "C" void* pxCurrentTCB;
static QueueHandle_t switchesQueue;
static void switches_task(void* pvParameters);

struct SwitchEvent {
    char      taskName[configMAX_TASK_NAME_LEN];
    uint32_t  timestamp;
    TaskSwitch task_switch;
};

extern "C" void log_context_switch_out(void) {
    TaskHandle_t leaving_task = xTaskGetCurrentTaskHandle(); 
    const char* task_name = pcTaskGetName((TaskHandle_t)pxCurrentTCB);
    const unsigned long current_time = micros();

    if(strcmp(task_name, "Switches task") == 0 || strcmp(task_name, "Logger") == 0) {
        return;
    }

    SwitchEvent event;
    event.task_switch = TASK_OFF;
    strncpy(event.taskName, task_name, configMAX_TASK_NAME_LEN - 1);
    event.taskName[configMAX_TASK_NAME_LEN - 1] = '\0';
    event.timestamp = current_time;

    xQueueSendFromISR(switchesQueue, &event, NULL);
}

extern "C" void log_context_switch_in(void) {
    TaskHandle_t coming_task = xTaskGetCurrentTaskHandle(); 
    const char* task_name = pcTaskGetName((TaskHandle_t)pxCurrentTCB);
    const unsigned long current_time = micros();

    if(strcmp(task_name, "Switches task") == 0 || strcmp(task_name, "Logger") == 0) {
        return;
    }

    SwitchEvent event;
    event.task_switch = TASK_ON;
    strncpy(event.taskName, task_name, configMAX_TASK_NAME_LEN - 1);
    event.taskName[configMAX_TASK_NAME_LEN - 1] = '\0';
    event.timestamp = current_time;

    xQueueSendFromISR(switchesQueue, &event, NULL);
}

static const char* switch_to_string(TaskSwitch task_switch) {
    switch (task_switch) {
        case TASK_OFF: return "OFF";
        case TASK_ON: return "ON";
        default:           return "UNKNOWN";
    }
}

void context_switches_init() {
    switchesQueue = xQueueCreate(SWITCHES_QUEUE_SIZE, sizeof(SwitchEvent));
    if (switchesQueue == NULL) {
        Serial.println("FATAL: switches queue failed");
        while(1);
    }
    if (xTaskCreate(switches_task, "Switches task", 256, NULL, 3, NULL) != pdPASS) {
        Serial.println("FATAL: switches task failed");
        while(1);
    }
    Serial.println("Switches OK");
}

static void switches_task(void* pvParameters) {
    SwitchEvent event;

    while (1) {
        if (xQueueReceive(switchesQueue, &event, portMAX_DELAY) == pdPASS) {
            char line[80];
            snprintf(line, sizeof(line), "%lu,%s,%s",
                event.timestamp,
                event.taskName,
                switch_to_string(event.task_switch));
            log_write(OPERATION, line);
        }
    }
}

