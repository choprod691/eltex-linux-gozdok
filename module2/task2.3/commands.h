#ifndef COMMANDS_H
#define COMMANDS_H

#include <stddef.h>

// указатель на функцию, принимающую два вещественных и возвращающую вещественное
typedef double (*BinaryOperation)(double, double);

// описание команды калькулятора
typedef struct {
    const char *symbol;
    const char *description;
    BinaryOperation operation;
    int requires_nonzero_second;
} Command;

// возвращаем массив команд и записываем его размер
const Command *commands_get(size_t *count);

#endif
