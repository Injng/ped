ped: src/main.c
	cc -g -pedantic -Wall -Wextra src/main.c -o main.o -fsanitize=address -lSDL3
