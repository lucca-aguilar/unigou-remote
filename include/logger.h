#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#define MAX_LOG_MSG_LEN 100

enum Level {
    DEBUG,
    OPERATION,
    WARNING,
    ERROR
};

void log_init();
void log_write(Level, const char*);
void serial_print_guarded(const char*);
void serial_println_guarded(const char*);

#endif