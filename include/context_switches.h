#ifndef CONTEXT_SWITCHES_H
#define CONTEXT_SWITCHES_H

enum TaskSwitch {
    TASK_ON,
    TASK_OFF
};

#ifdef __cplusplus
extern "C" {
#endif

void log_context_switch_out(void);
void log_context_switch_in(void);

#ifdef __cplusplus
}
#endif

void context_switches_init();
void switch_event(const char* taskName, TaskSwitch task_switch);

#endif