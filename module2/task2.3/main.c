#include <stdio.h>

#include "commands.h"

// проверяем ввод ("1" - успешный ввод, "0" - ввод закончился)
static int read_int(const char *prompt, int *value) {
    char line[100];
    char extra;

    while (1) {
        printf("%s", prompt);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            return 0;
        }

        // "%c" отменяет ввод "12abc"
        if (sscanf(line, "%d %c", value, &extra) == 1) {
            return 1;
        }

        printf("Ошибка: нужно ввести целое число.\n");
    }
}

static int read_double(const char *prompt, double *value) {
    char line[100];
    char extra;

    while (1) {
        printf("%s", prompt);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            return 0;
        }

        if (sscanf(line, "%lf %c", value, &extra) == 1) {
            return 1;
        }

        printf("Ошибка: нужно ввести число, например 12 или 3.5.\n");
    }
}

// мейн

int main(void) {
    size_t command_count;
    const Command *commands = commands_get(&command_count);

    while (1) {
        int choice;
        double a;
        double b;
        double result;
        const Command *command;

        // меню строим по массиву команд, а не вручную
        printf("\nКАЛЬКУЛЯТОР\n");

        for (size_t i = 0; i < command_count; i++) {
            printf("%zu. %s (%s)\n",
                   i + 1, commands[i].description, commands[i].symbol);
        }

        printf("0. Выход\n");

        if (!read_int("Выберите действие: ", &choice)) {
            printf("\nВвод завершён.\n");
            break;
        }

        if (choice == 0) {
            printf("Программа завершена.\n");
            break;
        }

        if (choice < 1 || (size_t)choice > command_count) {
            printf("Ошибка: такого пункта меню нет.\n");
            continue;
        }

        if (!read_double("Введите первый аргумент: ", &a) ||
            !read_double("Введите второй аргумент: ", &b)) {
            printf("\nВвод завершён.\n");
            break;
        }

        // берём команду из массива
        command = &commands[choice - 1];

        // для деления
        if (command->requires_nonzero_second && b == 0.0) {
            printf("Ошибка: деление на ноль невозможно.\n");
            continue;
        }

        // вызов функции через указатель
        result = command->operation(a, b);
        printf("Результат: %.10g\n", result);
    }

    return 0;
}
