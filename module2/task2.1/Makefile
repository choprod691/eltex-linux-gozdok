CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g

TARGET = phone_book
OBJECTS = main.o contacts.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

main.o: main.c contacts.h
	$(CC) $(CFLAGS) -c main.c -o main.o

contacts.o: contacts.c contacts.h
	$(CC) $(CFLAGS) -c contacts.c -o contacts.o

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean