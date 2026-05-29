/**
 * @file runtime_stats.h
 * @brief This module provides the interface to configure and read high-resolution timer metrics required by FreeRTOS to calculate CPU usage per task. 
 * @note This file utilizes `extern "C"` linkage to ensure compatibility between FreeRTOS kernel hooks and the Arduino framework.
 */

#ifndef RUNTIME_STATS_H
#define RUNTIME_STATS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the runtime statistics subsystem and its reporting mechanism.
 * @warning This function should only be called once, during system initialization and after the logging subsystem has been initialized. If there is a failure in creating the task for processing runtime statistics, the system will display an error message and enter an infinite loop.
 */
void runtime_stats_init();

/**
 * @brief Configures the high-resolution hardware timer for tracking CPU time.
 * @note This function is intended to be mapped directly to the FreeRTOS macro `portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()`.
 */
void runtime_stats_timer_init();

/**
 * @brief Retrieves the current value of the high-resolution runtime timer.
 * @return uint32_t The current high-resolution timer tick count.
 * @note This function is intended to be mapped directly to the FreeRTOS macro `portGET_RUN_TIME_COUNTER_VALUE()`.
 */
uint32_t runtime_stats_timer_get_value();

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_STATS_H