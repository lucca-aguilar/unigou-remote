#ifndef TASK_TRACER_H
#define TASK_TRACER_H

#include <Arduino.h>

void tracer_init();

// Força o compilador a usar a ligação (linkage) do C
#ifdef __cplusplus
extern "C" {
#endif

void tracer_log_from_hook(const char* taskName, int state);

#ifdef __cplusplus
}
#endif

#endif