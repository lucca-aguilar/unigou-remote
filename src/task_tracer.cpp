#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "logger.h"
#include "task_tracer.h"

#define TRACER_QUEUE_SIZE  30
#define TRACER_NAME_LEN    16

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
    if (xTaskCreate(tracer_task, "Tracer", 256, NULL, 1, NULL) != pdPASS) {
        Serial.println("FATAL: tracer task failed");
        while(1);
    }
    Serial.println("Tracer OK");
}

void tracer_event(const char* taskName, TaskState state) {
    if (traceQueue == NULL) return;
    TraceEvent ev;
    ev.timestamp = xTaskGetTickCount();
    ev.state     = state;
    strncpy(ev.taskName, taskName, TRACER_NAME_LEN - 1);
    ev.taskName[TRACER_NAME_LEN - 1] = '\0';
    xQueueSend(traceQueue, &ev, 0);
}

static void tracer_task(void* pvParameters) {
    TraceEvent ev;
    serial_println_guarded("=== TRACER START ===");
    serial_println_guarded("timestamp_ms,task,state");

    while (1) {
        if (xQueueReceive(traceQueue, &ev, portMAX_DELAY) == pdPASS) {
            char line[80];
            snprintf(line, sizeof(line), "%lu,%s,%s",
                ev.timestamp,
                ev.taskName,
                state_to_string(ev.state));
            serial_println_guarded(line);
        }
    }
}