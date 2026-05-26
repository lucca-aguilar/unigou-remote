#ifndef RUNTIME_STATS_H
#define RUNTIME_STATS_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

void runtime_stats_init();
void runtime_stats_timer_init();
uint32_t runtime_stats_timer_get_value();

#ifdef __cplusplus
}
#endif

#endif
#