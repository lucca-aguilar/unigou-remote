/**
 * @file mem_monitor.h
 * @brief Memory monitoring utility for heap and task stack tracking. This module tracks and reports system-wide heap statistics alongside individual task stack usage to detect memory leaks and prevent stack overflows.
 */

#ifndef MEM_MONITOR_H
#define MEM_MONITOR_H

/**
 * @brief Initializes the memory monitor module and spawns its reporting task.
 * @warning This function should only be called once, during system initialization and after the logging subsystem has been initialized. If there is a failure in creating the task for processing memory data, the system will display an error message and enter an infinite loop.
 */
void mem_monitor_init();

#endif // MEM_MONITOR_H