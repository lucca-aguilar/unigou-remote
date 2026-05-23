#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "trace.h"

void Task1(void *pvParameters);
void Task2(void *pvParameters);
void TaskMonitor(void *pvParameters);

TaskHandle_t TaskHandle_1;
TaskHandle_t TaskHandle_2;

void setup() {
    Serial.begin(115200);

    Serial.println("--- System Initialized ---");

    trace_init();

    Serial.println("Tracer initializing...");

    xTaskCreate(Task1, "Task 1", 128, NULL, 1, NULL);
    xTaskCreate(Task2, "Task 2", 128, NULL, 1, NULL);
    xTaskCreate(TaskMonitor, "Monitor", 256, NULL, 3, NULL); // highest priority

    vTaskStartScheduler();
}

void loop() {

}

void TaskMonitor(void *pvParameters)
{
    static bool printed = false;
    while (1)
    {
        if (traceIndex >= MAX_TRACES && !printed)
        {
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

void Task1(void *pvParameters)
{
    while (1)
    {
        trace_record("Task 1", STATE_RUNNING);
        vTaskDelay(pdMS_TO_TICKS(200));
        trace_record("Task 1", STATE_READY);
        vTaskDelay(pdMS_TO_TICKS(100));
        ram_record(xPortGetFreeHeapSize());
    }
}

void Task2(void *pvParameters)
{
    while (1)
    {
        trace_record("Task 2", STATE_RUNNING);
        vTaskDelay(pdMS_TO_TICKS(600));
        trace_record("Task 2", STATE_READY);
        vTaskDelay(pdMS_TO_TICKS(100));
        ram_record(xPortGetFreeHeapSize());
    }
}