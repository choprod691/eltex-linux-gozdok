#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contacts.h"

// вспомогательные функции

// читает строку и удаляет переход на новую строку
static void read_line(const char *prompt, char buffer[], int size) {
    int ch;

    printf("%s", prompt);

    if (fgets(buffer, size, stdin) == NULL) {
        printf("\nВвод завершён.\n");
        exit(0);
    }

    // если строка не поместилась - удаляем остаток
    if (strchr(buffer, '\n') == NULL) {
        while ((ch = getchar()) != '\n' && ch != EOF) {
            // пропускаем лишнее
        }
    }

    buffer[strcspn(buffer, "\r\n")] = '\0';
}

// просит обязательное поле
static void read_required_field(const char *prompt, char field[], int size) {
    do {
        read_line(prompt, field, size);

        if (field[0] == '\0') {
            printf("Это поле обязательно. Пожалуйста, введите значение.\n");
        }
    } while (field[0] == '\0');
}

// читает число в пределах
static int read_int(const char *prompt, int min, int max) {
    char line[TEXT_LEN];
    char extra;
    int value;

    while (1) {
        read_line(prompt, line, TEXT_LEN);

        if (sscanf(line, "%d %c", &value, &extra) == 1 &&
            value >= min && value <= max) {
            return value;
        }

        printf("Введите целое число от %d до %d.\n", min, max);
    }
}

// заполняет строки (пустая строка завершает ввод)
static void read_list(const char *title, ValueList *list) {
    char value[TEXT_LEN];

    values_clear(list);
    printf("\n%s. Можно ввести до %d значений.\n", title, MAX_VALUES);

    while (list->count < MAX_VALUES) {
        printf("Значение %d (Enter — закончить ввод списка): ",
               list->count + 1);
        read_line("", value, TEXT_LEN);

        if (value[0] == '\0') {
            break;
        }

        if (values_add(list, value) != 0) {
            printf("Не удалось добавить значение: список заполнен.\n");
            break;
        }
    }

    if (list->count == MAX_VALUES) {
        printf("Достигнуто максимальное количество значений.\n");
    }
}

// ввод

static void print_list(const char *title, const ValueList *list) {
    printf("%s: ", title);

    if (list->count == 0) {
        printf("не указано");
    } else {
        for (int i = 0; i < list->count; i++) {
            printf("%s", list->items[i]);

            if (i + 1 < list->count) {
                printf("; ");
            }
        }
    }

    printf("\n");
}

static void print_contact(const Contact *contact, int number) {
    printf("\nКонтакт №%d\n", number);
    printf("Фамилия: %s\n", contact->last_name);
    printf("Имя: %s\n", contact->first_name);

    if (contact->middle_name[0] != '\0') {
        printf("Отчество: %s\n", contact->middle_name);
    }
    if (contact->workplace[0] != '\0') {
        printf("Место работы: %s\n", contact->workplace);
    }
    if (contact->position[0] != '\0') {
        printf("Должность: %s\n", contact->position);
    }

    print_list("Телефоны", &contact->phones);
    print_list("Электронная почта", &contact->emails);
    print_list("Социальные сети", &contact->social_links);
    print_list("Мессенджеры", &contact->messengers);
}

static void print_all_contacts(const PhoneBook *book) {
    if (book->count == 0) {
        printf("\nТелефонная книга пуста.\n");
        return;
    }

    for (int i = 0; i < book->count; i++) {
        print_contact(&book->items[i], i + 1);
    }
}

// добавляем контакт

static void add_contact(PhoneBook *book) {
    Contact contact;

    if (book->count >= MAX_CONTACTS) {
        printf("Невозможно добавить контакт: книга заполнена.\n");
        return;
    }

    contact_init(&contact);

    printf("\nДобавление контакта\n");
    printf("Фамилия и имя обязательны. Остальные поля можно пропустить,\n");
    printf("нажав Enter.\n");

    read_required_field("Фамилия: ", contact.last_name, TEXT_LEN);
    read_required_field("Имя: ", contact.first_name, TEXT_LEN);
    read_line("Отчество: ", contact.middle_name, TEXT_LEN);
    read_line("Место работы: ", contact.workplace, TEXT_LEN);
    read_line("Должность: ", contact.position, TEXT_LEN);

    read_list("Номера телефонов", &contact.phones);
    read_list("Адреса электронной почты", &contact.emails);
    read_list("Ссылки на страницы в социальных сетях",
              &contact.social_links);
    read_list("Профили в мессенджерах", &contact.messengers);

    if (book_add(book, &contact) != 0) {
        printf("Не удалось добавить контакт.\n");
        return;
    }

    printf("Контакт добавлен под номером %d.\n", book->count);
}

// редактируем контакт

// пустой ввод сохраняет старое значение обязательного поля
static void edit_required_field(const char *title, char field[], int size) {
    char value[TEXT_LEN];

    printf("\n%s. Текущее значение: %s\n", title, field);
    read_line("Новое значение (Enter — оставить без изменений): ",
              value, TEXT_LEN);

    if (value[0] != '\0') {
        snprintf(field, size, "%s", value);
    }
}

// "enter" сохраняет значение, а "-" очищает необязательное поле
static void edit_optional_field(const char *title, char field[], int size) {
    char value[TEXT_LEN];

    printf("\n%s. Текущее значение: %s\n", title,
           field[0] == '\0' ? "не указано" : field);
    read_line("Новое значение (Enter — оставить, '-' — очистить): ",
              value, TEXT_LEN);

    if (value[0] == '\0') {
        return;
    }

    if (strcmp(value, "-") == 0) {
        field[0] = '\0';
    } else {
        snprintf(field, size, "%s", value);
    }
}

static void edit_list(const char *title, ValueList *list) {
    int action;

    printf("\n");
    print_list(title, list);
    action = read_int("1 — оставить, 2 — заменить список, 3 — очистить: ",
                      1, 3);

    if (action == 2) {
        read_list(title, list);
    } else if (action == 3) {
        values_clear(list);
        printf("Список очищен.\n");
    }
}

// спрашиваем номер контакта и возвращаем индекс (-1 при отмене)
static int choose_contact(const PhoneBook *book) {
    int choice;

    if (book->count == 0) {
        printf("\nТелефонная книга пуста.\n");
        return -1;
    }

    printf("\nДоступные контакты:\n");
    for (int i = 0; i < book->count; i++) {
        printf("%d. %s %s\n", i + 1,
               book->items[i].last_name, book->items[i].first_name);
    }

    choice = read_int("Номер контакта (0 — отмена): ", 0, book->count);

    if (choice == 0) {
        return -1;
    }

    return choice - 1;
}

static void edit_contact(PhoneBook *book) {
    int index = choose_contact(book);
    Contact *contact;

    if (index == -1) {
        return;
    }

    contact = book_get(book, index);

    if (contact == NULL) {
        printf("Не удалось получить контакт.\n");
        return;
    }

    printf("\nРедактирование контакта №%d\n", index + 1);

    edit_required_field("Фамилия", contact->last_name, TEXT_LEN);
    edit_required_field("Имя", contact->first_name, TEXT_LEN);
    edit_optional_field("Отчество", contact->middle_name, TEXT_LEN);
    edit_optional_field("Место работы", contact->workplace, TEXT_LEN);
    edit_optional_field("Должность", contact->position, TEXT_LEN);

    edit_list("Телефоны", &contact->phones);
    edit_list("Электронная почта", &contact->emails);
    edit_list("Социальные сети", &contact->social_links);
    edit_list("Мессенджеры", &contact->messengers);

    printf("Изменения сохранены.\n");
}

// удаляем контакт

static void delete_contact(PhoneBook *book) {
    int index = choose_contact(book);

    if (index == -1) {
        return;
    }

    if (book_remove(book, index) != 0) {
        printf("Не удалось удалить контакт.\n");
        return;
    }

    printf("Контакт удалён.\n");
}

// мейн

int main(void) {
    PhoneBook book;
    int choice;

    book_init(&book);

    do {
        printf("\nТЕЛЕФОННАЯ КНИГА\n");
        printf("Контактов: %d из %d\n", book.count, MAX_CONTACTS);
        printf("1. Показать все контакты\n");
        printf("2. Добавить контакт\n");
        printf("3. Редактировать контакт\n");
        printf("4. Удалить контакт\n");
        printf("0. Выйти\n");

        choice = read_int("Выберите действие: ", 0, 4);

        switch (choice) {
            case 1:
                print_all_contacts(&book);
                break;
            case 2:
                add_contact(&book);
                break;
            case 3:
                edit_contact(&book);
                break;
            case 4:
                delete_contact(&book);
                break;
            case 0:
                printf("Программа завершена.\n");
                break;
        }
    } while (choice != 0);

    return 0;
}
