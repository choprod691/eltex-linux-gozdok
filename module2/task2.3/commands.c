#include "commands.h"
#include "calculator.h"

// для добавления операции - добавить сюда строку
static const Command commands[] = {
    {"+", "Сложение", add, 0},
    {"-", "Вычитание", subtract, 0},
    {"*", "Умножение", multiply, 0},
    {"/", "Деление", divide, 1}
};

const Command *commands_get(size_t *count) {
    *count = sizeof(commands) / sizeof(commands[0]);
    return commands;
}
