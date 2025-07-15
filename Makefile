CFLAGS = -std=c11 -pedantic -Wall -Wextra -g -fsanitize=address
LDLIBS = -lSDL3_ttf -lSDL3

ped: src/main.c
	cc $(CFLAGS) src/main.c -o main.o $(LDLIBS)
