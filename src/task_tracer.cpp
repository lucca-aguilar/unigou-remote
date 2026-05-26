#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "logger.h"
#include "task_tracer.h"

#define TRACER_QUEUE_SIZE  100
#define TRACER_NAME_LEN    configMAX_TASK_NAME_LEN

struct TraceEvent {
    char      taskName[TRACER_NAME_LEN];
    uint32_t  timestamp;
    TaskState state;
};

static QueueHandle_t traceQueue;
static void tracer_task(void* pvParameters);

static const char* state_to_string(TaskState state) {
    switch (state) {
        case TASK_RUNNING: return "RUNNING";
        case TASK_BLOCKED: return "BLOCKED";
        case TASK_READY:   return "READY";
        default:           return "UNKNOWN";
    }
}

void tracer_init() {
    traceQueue = xQueueCreate(TRACER_QUEUE_SIZE, sizeof(TraceEvent));
    if (traceQueue == NULL) {
        Serial.println("FATAL: tracer queue failed");
        while(1);
    }
    if (xTaskCreate(tracer_task, "Tracer task", 256, NULL, 3, NULL) != pdPASS) {
        Serial.println("FATAL: tracer task failed");
        while(1);
    }
    Serial.println("Tracer OK");
}

void tracer_event(const char* taskName, TaskState state) {
    if (traceQueue == NULL) return;
    TraceEvent event;
    event.timestamp = xTaskGetTickCount();
    event.state     = state;
    strncpy(event.taskName, taskName, TRACER_NAME_LEN - 1);
    event.taskName[TRACER_NAME_LEN - 1] = '\0';
    xQueueSend(traceQueue, &event, 0);
}

static void tracer_task(void* pvParameters) {
    TraceEvent event;
    serial_println_guarded("=== TRACER START ===");
    serial_println_guarded("timestamp_ms,task,state");

    while (1) {
        if (xQueueReceive(traceQueue, &event, portMAX_DELAY) == pdPASS) {
            char line[80];
            snprintf(line, sizeof(line), "%lu,%s,%s",
                event.timestamp,
                event.taskName,
                state_to_string(event.state));
            serial_println_guarded(line);
        }
    }
}