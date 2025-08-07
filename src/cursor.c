#include <stdbool.h>

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include "cursor.h"
#include "glyph.h"

bool render_cursor(SDL_Renderer *renderer, Cursor *cursor, Glyphs *glyphs)
{
  // calculate destination rectangle based on glyph dimensions
  SDL_FRect dst = {
    .x = PADDING + glyphs->width * (cursor->idx + 1),
    .y = PADDING + glyphs->height * cursor->line,
    .w = glyphs->width,
    .h = glyphs->height
  };

  // set the drawing color for the cursor rectangle
  if (!SDL_SetRenderDrawColor(renderer, 128, 128, 128, 128)) {
    return false;
  }

  // render the cursor
  if (!SDL_RenderFillRect(renderer, &dst)) {
    return false;
  }
  return true;
}
