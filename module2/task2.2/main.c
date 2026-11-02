#include <stdio.h>

#include "calculator.h"

// проверяем ввод

// запрашиваем целое число ("1" - успех, "0" - ввод закончился)
static int read_int(const char *prompt, int *value) {
    char line[100];
    char extra;

    while (1) {
        printf("%s", prompt);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            return 0;
        }

        // "%c" не даёт ввод "12abc"
        if (sscanf(line, "%d %c", value, &extra) == 1) {
            return 1;
        }

        printf("Ошибка: нужно ввести целое число.\n");
    }
}

// запрашиваем вещественное число
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
    int choice;
    double a;
    double b;
    double result;

    while (1) {
        printf("\nКАЛЬКУЛЯТОР\n");
        printf("1. Сложение\n");
        printf("2. Вычитание\n");
        printf("3. Умножение\n");
        printf("4. Деление\n");
        printf("0. Выход\n");

        if (!read_int("Выберите действие: ", &choice)) {
            printf("\nВвод завершён.\n");
            break;
        }

        if (choice == 0) {
            printf("Программа завершена.\n");
            break;
        }

        if (choice < 1 || choice > 4) {
            printf("Ошибка: такого пункта меню нет.\n");
            continue;
        }

        if (!read_double("Введите первый аргумент: ", &a) ||
            !read_double("Введите второй аргумент: ", &b)) {
            printf("\nВвод завершён.\n");
            break;
        }

        // проверка до деления
        if (choice == 4 && b == 0.0) {
            printf("Ошибка: деление на ноль невозможно.\n");
            continue;
        }

        // вызываем операцию
        switch (choice) {
            case 1:
                result = add(a, b);
                break;
            case 2:
                result = subtract(a, b);
                break;
            case 3:
                result = multiply(a, b);
                break;
            case 4:
                result = divide(a, b);
                break;
        }

        printf("Результат: %.10g\n", result);
    }

    return 0;
}
