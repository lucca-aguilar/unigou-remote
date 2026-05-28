#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "logger.h"
#include "mem_monitor.h"
#include "task_tracer.h"
#include "runtime_stats.h"

#define PRIORITY_LOW    1
#define PRIORITY_MED    2
#define PRIORITY_HIGH   3

SemaphoreHandle_t xResourceMutex;
TaskHandle_t xTargetTaskHandle = NULL;

static void TaskA(void* pvParameters);
static void TaskB(void* pvParameters);
static void TaskMemory(void* pvParameters);
static void TaskController(void* pvParameters);

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    log_init();
    mem_monitor_init();
    // tracer_init();
    // runtime_stats_init();
    
    Serial.println("BOOT");
    
    xResourceMutex = xSemaphoreCreateMutex();
    if (xResourceMutex != NULL) {
        xTaskCreate(TaskA, "Task A", 150, NULL, PRIORITY_MED, NULL);
        xTaskCreate(TaskB, "Task B", 150, NULL, PRIORITY_MED, &xTargetTaskHandle); 
        xTaskCreate(TaskMemory, "Memory", 150, NULL, PRIORITY_LOW, NULL);
        xTaskCreate(TaskController, "Controller", 120, NULL, PRIORITY_HIGH, NULL);
    } else {
        Serial.println("FATAL: failed to create the mutex.");
        while(1);
    }
    
    vTaskStartScheduler();

}

void loop() {}

static void TaskA(void* pvParameters) {
    while (1) {
        log_write(OPERATION, "A trying to use the Mutex...");
        if (xSemaphoreTake(xResourceMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            log_write(OPERATION, "A using the Mutex...");
            vTaskDelay(pdMS_TO_TICKS(100)); 
            xSemaphoreGive(xResourceMutex);
            log_write(OPERATION, "A released Mutex.");
        } else {
            log_write(WARNING, "A timed out waiting for Mutex.");
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
}

static void consume_stack(int depth) {
    volatile uint32_t dummy_data[4]; 
    dummy_data[0] = depth; 
    if (depth > 0) {
        consume_stack(depth - 1); 
    }
}

static void TaskB(void* pvParameters) {
    int work_complexity = 0; 

    while (1) {
        log_write(OPERATION, "B trying to use the Mutex...");
        
        if (xSemaphoreTake(xResourceMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            log_write(OPERATION, "B using the Mutex...");

            consume_stack(work_complexity);

            if (work_complexity < 12) {
                work_complexity++;
            }
            
            vTaskDelay(pdMS_TO_TICKS(150)); 
            xSemaphoreGive(xResourceMutex);
            log_write(OPERATION, "B released Mutex.");
        }
        
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

static void TaskMemory(void* pvParameters) {
    void* pointerAllocation = NULL;
    size_t sizeAllocation = 512; 
    
    while (1) {
        if (pointerAllocation == NULL) {
            pointerAllocation = pvPortMalloc(sizeAllocation);
            if (pointerAllocation != NULL) {
                log_write(OPERATION, "RAM: allocated 512 bytes.");
            } else {
                log_write(ERROR, "RAM: no heap memory.");
            }
        } else {
            vPortFree(pointerAllocation);
            pointerAllocation = NULL;
            log_write(OPERATION, "RAM: freed 512 bytes.");
        }
        vTaskDelay(pdMS_TO_TICKS(400)); 
    }
}

static void TaskController(void* pvParameters) {
    bool suspended = false;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(3000)); 
        if (xTargetTaskHandle != NULL) {
            if (!suspended) {
                log_write(WARNING, "Controller: suspending B.");
                suspended = true;
                vTaskSuspend(xTargetTaskHandle); 
            } else {
                log_write(OPERATION, "Controller: resuming B.");
                suspended = false;
                vTaskResume(xTargetTaskHandle); 
            }
        }
    }
}