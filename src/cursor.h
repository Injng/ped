#ifndef CURSOR_H
#define CURSOR_H

#include <stdbool.h>

#include <SDL3/SDL_render.h>

#include "glyph.h"

typedef struct Cursor {
  int line;
  int idx;
} Cursor;

bool render_cursor(SDL_Renderer *renderer, Cursor *cursor, Glyphs *glyphs);

#endif // CURSOR_H
