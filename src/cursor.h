#ifndef CURSOR_H
#define CURSOR_H

#include <stdbool.h>

#include <SDL3/SDL_render.h>

#include "glyph.h"

struct Buffer;

typedef struct Cursor {
  int line;
  int idx;
} Cursor;

bool render_cursor(SDL_Renderer *renderer, Cursor *cursor, Glyphs *glyphs);

void move_cursor(Cursor *cursor, struct Buffer *buffer, SDL_Keycode key);

#endif // CURSOR_H
