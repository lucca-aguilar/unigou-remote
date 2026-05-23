#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include "runtime_stats.h"

// Protótipo da Task
static void runtime_task(void* pvParameters);

// Buffer estático na memória global para evitar estouro de Stack
static char stats_buffer[2048]; 

void runtime_stats_init() {
    // Aloca 512 palavras de Stack (2048 bytes em ARM 32-bits), espaço de sobra para o Uno R4
    if (xTaskCreate(runtime_task, "RunStats", 512, NULL, 1, NULL) != pdPASS) {
        Serial.println("FATAL: runtime stats task failed");
        while(1);
    }
    Serial.println("Runtime stats task OK");
}

static void runtime_task(void* pvParameters) {
    TickType_t LastWakeTime = xTaskGetTickCount();

    while(1) {
        // Aguarda 5 segundos de forma eficiente
        vTaskDelayUntil(&LastWakeTime, pdMS_TO_TICKS(5000));
        
        // Limpa o buffer antes do preenchimento por segurança
        memset(stats_buffer, 0, sizeof(stats_buffer));

        Serial.println("\n=================================");
        Serial.println("   ESTATÍSTICAS DE USO DA CPU   ");
        Serial.println("=================================");
        
        // Coleta e formata os dados diretamente no buffer global
        vTaskGetRunTimeStats(stats_buffer);
        
        // Imprime o resultado formatado na Serial
        Serial.print(stats_buffer);
    }
}
