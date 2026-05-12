#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "trace.h"

void TaskBlink(void *pvParameters);
void TaskSensor(void *pvParameters);
void TaskMonitor(void *pvParameters);
void TaskHighPriority(void *pvParameters);
void TaskMemoryStress(void *pvParameters);

TaskHandle_t xSensorHandle = NULL;

void setup() {
    Serial.begin(115200);
    uint32_t timeout = millis();
    while (!Serial && millis() - timeout < 3000); 

    Serial.println("--- Sistema Iniciado ---");
    
    trace_init();
    Serial.println("Tracer inicializado...");

    xTaskCreate(TaskBlink, "Blink", 128, NULL, 2, NULL);
    xTaskCreate(TaskSensor, "Sensor", 128, NULL, 1, NULL);
    xTaskCreate(TaskMonitor, "Monitor", 256, NULL, 3, NULL);
    xTaskCreate(TaskHighPriority, "Heavy", 128, NULL, 3, NULL);
    xTaskCreate(TaskMemoryStress, "Stress", 128, NULL, 1, NULL);

    Serial.println("Scheduler vai iniciar...");
 
    vTaskStartScheduler(); 
}

void loop() {
    trace_record("IDLE", STATE_RUNNING);
}

void TaskMonitor(void *pvParameters) {
    static bool printed = false;
    for (;;) {
        trace_record_ram();

        if (traceIndex >= MAX_TRACES && !printed) {
            Serial.println("### BUFFER CHEIO - EXPORTANDO CSV ###");
            vTaskSuspendAll(); 
            trace_print_csv();
            printed = true;
            xTaskResumeAll();
            Serial.println("### FIM DA EXPORTACAO ###");
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
        // A cada 10 ciclos, suspende a tarefa Sensor por 2 segundos
        if (cycleCount >= 10) {
            trace_record("Sensor", STATE_SUSPENDED); // Registra que a OUTRA suspendeu
            vTaskSuspend(xSensorHandle);
            
            vTaskDelay(pdMS_TO_TICKS(2000));
            
            vTaskResume(xSensorHandle);
            trace_record("Sensor", STATE_READY); // Volta para Ready após o resume
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
        // Esta tarefa "acorda" e fica usando a CPU por 50ms sem parar
        // Como a prioridade dela é 3, Blink(2) e Sensor(1) ficarão em READY
        vTaskDelay(pdMS_TO_TICKS(1000)); 
        
        trace_record("Heavy", STATE_RUNNING);
        trace_record("Blink", STATE_READY);  // Notifica que as outras foram preteridas
        trace_record("Sensor", STATE_READY);
        
        delay(50); // Uso intensivo de CPU (bloqueia o core)
        
        trace_record("Heavy", STATE_BLOCKED);
    }
}

void TaskMemoryStress(void *pvParameters) {
    for (;;) {
        // 1. Registra a RAM antes da alocação
        trace_record_ram();
        
        // 2. Aloca um bloco de 1KB (1024 bytes)
        // Usamos uint8_t para garantir que cada posição seja 1 byte
        uint8_t* dummyBuffer = new uint8_t[1024];

        if (dummyBuffer != nullptr) {
            // Preenche com lixo para garantir que a RAM foi tocada
            memset(dummyBuffer, 0xAA, 1024);
            
            trace_record_ram(); // Registra a queda na RAM livre
            
            // Segura a memória por 2 segundos para dar tempo do tracer capturar
            vTaskDelay(pdMS_TO_TICKS(2000));

            // 3. Libera a memória
            delete[] dummyBuffer;
            
            trace_record_ram(); // Registra a subida da RAM livre
        }

        // Espera um pouco antes do próximo ciclo
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}