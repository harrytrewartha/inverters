CC = gcc
CFLAGS = -Wall -Wextra -g -O3

LIBS = -lm -lraylib

SRCS = main.c
OBJS = main.o

TARGET = main

all: clean run

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
