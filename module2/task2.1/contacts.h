#ifndef CONTACTS_H
#define CONTACTS_H

// ограничения книги
#define MAX_CONTACTS 100
#define MAX_VALUES 5
#define TEXT_LEN 100

// список однотипных строк (телефон, адрес и т.д.)
typedef struct {
    char items[MAX_VALUES][TEXT_LEN];
    int count;
} ValueList;

// всё одного контакта
typedef struct {
    char last_name[TEXT_LEN];
    char first_name[TEXT_LEN];
    char middle_name[TEXT_LEN];
    char workplace[TEXT_LEN];
    char position[TEXT_LEN];

    ValueList phones;
    ValueList emails;
    ValueList social_links;
    ValueList messengers;
} Contact;

// массив контактов и количество занятых элементов.
typedef struct {
    Contact items[MAX_CONTACTS];
    int count;
} PhoneBook;

// список строк
int values_add(ValueList *list, const char *value);
void values_clear(ValueList *list);

// контакты и книга
void contact_init(Contact *contact);
void book_init(PhoneBook *book);
int book_add(PhoneBook *book, const Contact *contact);
int book_remove(PhoneBook *book, int index);
Contact *book_get(PhoneBook *book, int index);

#endif
