/**
 * @file task_tracer.h
 * @brief High-frequency task tracing and context switch monitoring utility.
 */

#ifndef TASK_TRACER_H
#define TASK_TRACER_H

#include <Arduino.h>

/**
 * @brief Initializes the task tracer subsystem and its reporting mechanism.
 * @warning This function should only be called once, during system initialization and after the logging subsystem has been initialized. If there is a failure in creating the task for task tracing, the system will display an error message and enter an infinite loop.
 */
void tracer_init();

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Low-level hook function called by the FreeRTOS kernel to log state changes. This function acts as the bridge between FreeRTOS internal kernel events and user-space tracing logic. 
 * @param[in] taskName The string pointer containing the name of the tracked task.
 * @param[in] state    The integer ID representing the target state transition.
 * @note Implements C-linkage so it can be called seamlessly from inside the native C files of the FreeRTOS core.
 */
void tracer_log_from_hook(const char* taskName, int state);

#ifdef __cplusplus
}
#endif

#endif // TASK_TRACER_H