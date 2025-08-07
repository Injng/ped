CFLAGS = -pedantic -Wall -Wextra -g -fsanitize=address
SRC = src/main.c src/glyph.c src/rope.c src/buffer.c src/cursor.c
LDLIBS = -lSDL3_ttf -lSDL3

pedit: src/main.c
	cc $(CFLAGS) ${SRC} -o main.o $(LDLIBS)
