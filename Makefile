CFLAGS = -pedantic -Wall -Wextra -g -fsanitize=address
LDLIBS = -lSDL3_ttf -lSDL3

pedit: src/main.c
	cc $(CFLAGS) src/main.c src/glyph.c src/rope.c -o main.o $(LDLIBS)
