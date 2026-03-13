CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = main.c interpreter.c parser.c tokenizer.c utils.c types.c
OBJ = $(SRC:.c=.o)
TARGET = interpreter

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
