#include <Arduino_FreeRTOS.h>
#include "trace.h"
#include "C:\Users\sophi\.platformio\packages\framework-arduinorenesas-uno\libraries\Arduino_FreeRTOS\src\lib\FreeRTOS-Kernel-v10.5.1\task.h" 

TraceEntry traceBuffer[MAX_TRACES];

int traceIndex = 0;
bool bufferFull = false;
const char* lastTask = "";
TaskState lastState = (TaskState)-1;

void trace_init() {
    traceIndex = 0;
    bufferFull = false;
}

void trace_record(const char* name, TaskState state) {
    if (traceIndex < MAX_TRACES) {
        if (strcmp(name, lastTask) != 0 || state != lastState) {
            traceBuffer[traceIndex].taskName = name;
            traceBuffer[traceIndex].timestamp = micros();
            traceBuffer[traceIndex].value = (uint32_t)state; 
            traceBuffer[traceIndex].type = TYPE_STATE;       
            traceIndex++;
            lastTask = name;
            lastState = state;
        }
    }
}

void trace_record_ram() {
    if (traceIndex < MAX_TRACES) {
        traceBuffer[traceIndex].taskName = "SYSTEM_RAM";
        traceBuffer[traceIndex].timestamp = micros();
        traceBuffer[traceIndex].value = xPortGetFreeHeapSize();
        traceBuffer[traceIndex].type = TYPE_RAM;
        traceIndex++;
    }
}

void trace_print_csv() {
    Serial.println("task_name,timestamp_us,state_or_value,type"); 
    
    for (int i = 0; i < traceIndex; i++) {
        Serial.print(traceBuffer[i].taskName);
        Serial.print(",");
        Serial.print(traceBuffer[i].timestamp);
        Serial.print(",");
        
        if (traceBuffer[i].type == TYPE_RAM) {
            Serial.print(traceBuffer[i].value);
            Serial.println(",RAM");
        } else {
            switch(traceBuffer[i].value) {
                case STATE_RUNNING:   Serial.print("RUNNING"); break;
                case STATE_READY:     Serial.print("READY"); break;
                case STATE_BLOCKED:   Serial.print("BLOCKED"); break;
                case STATE_SUSPENDED: Serial.print("SUSPENDED"); break;
                default:              Serial.print("UNKNOWN"); break;
            }
            Serial.println(",STATE");
        }
    }
}