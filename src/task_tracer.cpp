/**
 * @file task_tracer.cpp
 * @brief Implementation of the lock-free kernel event tracer.
 */

#include "task_tracer.h"
#include "logger.h"
#include <Arduino_FreeRTOS.h>

#define TRACER_BUFFER_SIZE 128

/**
 * @brief Data packet encapsulating a single captured kernel trace event.
 */
struct TraceEvent {
    const char* taskName; 
    uint32_t     timestamp;
    int          state;  
};

static TraceEvent traceBuffer[TRACER_BUFFER_SIZE];

static volatile uint32_t head = 0;

static volatile uint32_t tail = 0;

static void tracer_task(void* pvParameters);

/**
 * @brief Internal translation utility converting state integer IDs into strings.
 * @param[in] state The integer value of the state transition.
 * @return const char* A pointer to the corresponding static flash string literal.
 */
static const char* state_to_string(int state) {
    switch (state) {
        case 0:  return "RUNNING";
        case 1:  return "READY";
        case 2:  return "BLOCKED";
        case 3:  return "SUSPENDED";
        case 4:  return "DELETED";
        default: return "UNKNOWN";
    }
}

void tracer_init() {
    if (xTaskCreate(tracer_task, "Tracer", 256, NULL, configMAX_PRIORITIES - 1, NULL) != pdPASS) {
        Serial.println("FATAL: tracer task failed");
        while(1); 
    }
    Serial.println("Tracer OK");
}

void tracer_log_from_hook(const char* taskName, int state) {
    uint32_t next_head = (head + 1) % TRACER_BUFFER_SIZE;

    if (next_head != tail) { 
        traceBuffer[head].timestamp = xTaskGetTickCountFromISR(); 
        traceBuffer[head].taskName = taskName;
        traceBuffer[head].state = state;

        head = next_head;
    }
}

/**
 * @brief High-priority background FreeRTOS task processing trace logs. This task wakes up periodically every 50ms, evaluates if any new events have been pushed into the ring buffer, formats them, and forwards them to the centralized system logger module.
 */
static void tracer_task(void* pvParameters) {
    (void)pvParameters; 
    
    while (1) {
        while (tail != head) {
            TraceEvent event = traceBuffer[tail];

            tail = (tail + 1) % TRACER_BUFFER_SIZE;

            char line[80];
            snprintf(line, sizeof(line), "%lu,%s,%s",
                event.timestamp,
                event.taskName,
                state_to_string(event.state));

            log_write(OPERATION, line);
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}