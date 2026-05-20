#ifndef TRACE_H
#define TRACE_H

#include <Arduino.h>

extern int traceIndex; 
extern bool bufferFull;
const int MAX_TRACES = 300; 

enum TaskState {
    STATE_RUNNING,
    STATE_READY,
    STATE_BLOCKED,
    STATE_SUSPENDED
};

enum EventType { 
    TYPE_STATE, 
    TYPE_RAM 
};

struct TraceEntry {
    const char* taskName;
    uint32_t timestamp;
    TaskState state;
    uint32_t value;
    uint8_t type;
};

void trace_init();
void trace_record_ram();
void trace_record(const char* name, TaskState state);
void trace_print_csv();

#endif