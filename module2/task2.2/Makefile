CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g

TARGET = calculator
OBJECTS = main.o calculator.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

main.o: main.c calculator.h
	$(CC) $(CFLAGS) -c main.c -o main.o

calculator.o: calculator.c calculator.h
	$(CC) $(CFLAGS) -c calculator.c -o calculator.o

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean