CFLAGS  := -Wall -Wextra -O2 $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS	:= $(shell pkg-config --libs gtk+-3.0)
CC      := gcc

all: chip8

chip8: chip8.o chip8_instructions.o main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.o: .c
	$(CC) $(CFLAGS) -c $^

clean :
	rm -f *.o */*.o chip8
