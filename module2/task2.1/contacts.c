#include <stdio.h>
#include <string.h>

#include "contacts.h"

// добавляет строку в список, "0" - успех, "-1" - список заполнен
int values_add(ValueList *list, const char *value) {
    if (list->count >= MAX_VALUES) {
        return -1;
    }

    snprintf(list->items[list->count], TEXT_LEN, "%s", value);
    list->count++;

    return 0;
}

// после обнуления счётчика старые строки больше не часть списка
void values_clear(ValueList *list) {
    list->count = 0;
}

// обнуляет все поля контакта
void contact_init(Contact *contact) {
    memset(contact, 0, sizeof(*contact));
}

// сначала книга пустая
void book_init(PhoneBook *book) {
    book->count = 0;
}

// добавляет копию контакта в свободный элемент массива
int book_add(PhoneBook *book, const Contact *contact) {
    if (book->count >= MAX_CONTACTS) {
        return -1;
    }

    book->items[book->count] = *contact;
    book->count++;

    return 0;
}

// удаляет контакт и двигает элементы на один шаг влево
int book_remove(PhoneBook *book, int index) {
    if (index < 0 || index >= book->count) {
        return -1;
    }

    for (int i = index; i < book->count - 1; i++) {
        book->items[i] = book->items[i + 1];
    }

    book->count--;

    return 0;
}

// возвращает указатель на контакт ("NULL" - неправильный индекс)
Contact *book_get(PhoneBook *book, int index) {
    if (index < 0 || index >= book->count) {
        return NULL;
    }

    return &book->items[index];
}
