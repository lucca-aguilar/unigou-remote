#ifndef TASK_TRACER_H
#define TASK_TRACER_H

enum TaskState {
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_READY
};

void tracer_init();
void tracer_event(const char* taskName, TaskState state);

#endif