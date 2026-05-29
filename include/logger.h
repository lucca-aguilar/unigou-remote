/**
 * @file logger.h
 * @brief Asynchronous logging system based on queues and dedicated tasks. This module manages the creation, queuing, and processing of system logs in a non-blocking manner for the main tasks.
 */

#ifndef LOGGER_H
#define LOGGER_H

/**
 * @brief Defines the maximum length of log messages.
 */
#define MAX_LOG_MSG_LEN 100

/**
 * @brief Levels of log severity.
 * @note It does not affect the order in which the logs are printed.
 */
enum Level {
    DEBUG,
    OPERATION,
    WARNING,
    ERROR
};

/**
 * @brief Initializes the log queue and the log processing task.
 * @warning This function should only be called once, during system initialization and before any call to `log_write()`. If there is a failure in allocating memory for the log queue or in creating the task for processing the logs, the system will display an error message and enter an infinite loop.
 */
void log_init();

/**
 * @brief Creates a log with required fields and sends it to the log queue.
 * @param[in] level Severity of the log.
 * @param[in] message String containing message.
 * @pre The log_init() function should have been executed successfully.
 */
void log_write(Level, const char*);

#endif