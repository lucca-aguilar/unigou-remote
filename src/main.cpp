#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "trace.h"

void TaskBlink(void *pvParameters);
void TaskSensor(void *pvParameters);
void TaskMonitor(void *pvParameters);
void TaskHighPriority(void *pvParameters);

TaskHandle_t xSensorHandle = NULL;

void setup() {
    Serial.begin(115200);
    uint32_t timeout = millis();

    while (!Serial && millis() - timeout < 3000); 

    Serial.println("--- System Initialized ---");
    
    trace_init();
    Serial.println("Tracer initializing...");

    xTaskCreate(TaskBlink, "Blink", 128, NULL, 2, NULL);
    xTaskCreate(TaskSensor, "Sensor", 128, NULL, 1, NULL);
    xTaskCreate(TaskMonitor, "Monitor", 256, NULL, 3, NULL);
    xTaskCreate(TaskHighPriority, "Heavy", 128, NULL, 3, NULL);
 
    vTaskStartScheduler(); 
}

void loop() {
    trace_record("IDLE", STATE_RUNNING);
}

void TaskMonitor(void *pvParameters) {
    static bool printed = false;
    for (;;) {
        if (traceIndex >= MAX_TRACES && !printed) {
            Serial.println("### FULL BUFFER - EXPORTING CSV ###");
            vTaskSuspendAll(); 
            trace_print_csv();
            printed = true;
            xTaskResumeAll();
            Serial.println("### END OF EXPORTATION ###");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void TaskBlink(void *pvParameters) {
    pinMode(LED_BUILTIN, OUTPUT);
    int cycleCount = 0;
    for (;;) {
        trace_record("Blink", STATE_RUNNING);
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        cycleCount++;
        if (cycleCount >= 10) {
            trace_record("Sensor", STATE_SUSPENDED); 
            vTaskSuspend(xSensorHandle);
            vTaskDelay(pdMS_TO_TICKS(2000));  
            vTaskResume(xSensorHandle);
            trace_record("Sensor", STATE_READY); 
            cycleCount = 0;
        }
        trace_record("Blink", STATE_BLOCKED);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void TaskSensor(void *pvParameters) {
    for (;;) {
        trace_record("Sensor", STATE_RUNNING);
        delayMicroseconds(500); 
        trace_record("Sensor", STATE_BLOCKED);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void TaskHighPriority(void *pvParameters) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000)); 
        trace_record("Heavy", STATE_RUNNING);
        trace_record("Blink", STATE_READY);  
        trace_record("Sensor", STATE_READY);
        delay(50); 
        trace_record("Heavy", STATE_BLOCKED);
    }
}

