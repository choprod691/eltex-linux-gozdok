CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g

TARGET = calculator
OBJECTS = main.o calculator.o commands.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

main.o: main.c commands.h
	$(CC) $(CFLAGS) -c main.c -o main.o

calculator.o: calculator.c calculator.h
	$(CC) $(CFLAGS) -c calculator.c -o calculator.o

commands.o: commands.c commands.h calculator.h
	$(CC) $(CFLAGS) -c commands.c -o commands.o

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean