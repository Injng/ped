#include <stdbool.h>

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include "buffer.h"
#include "cursor.h"
#include "glyph.h"
#include "stb_ds.h"

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

void move_cursor(Cursor *cursor, Buffer *buffer, SDL_Keycode key)
{
  // handle cursor move to the left
  if (key == SDLK_LEFT) {
    // stop movement if cursor is at the beginning of the line
    if (cursor->idx != -1) cursor->idx--;
  }

  // handle cursor move to the right
  else if (key == SDLK_RIGHT) {
    // stop movement if cursor is at the end of the line
    int line_length = arrlen(buffer->text[cursor->line]);
    if (cursor->idx != line_length - 1) cursor->idx++;
  }

  // handle cursor move upwards if current line is not the top line
  else if (key == SDLK_UP && cursor->line != 0) {
    // move the cursor up by a line
    int line_length = arrlen(buffer->text[cursor->line]);
    cursor->line--;

    // if the cursor was originally at the end of the line, preserve that
    int new_length = arrlen(buffer->text[cursor->line]);
    if (cursor->idx == line_length - 1) {
      cursor->idx = new_length - 1;
    }

    // otherwise, if cursor's index is greater than line length, move to end of line
    else if (new_length <= cursor->idx) {
      cursor->idx = new_length - 1;
    }
  }

  // handle cursor move downwards if current line is not the bottom line
  else if (key == SDLK_DOWN && cursor->line != arrlen(buffer->text) - 1) {
    // move the cursor down by a line
    int line_length = arrlen(buffer->text[cursor->line]);
    cursor->line++;

    // if the cursor was originally at the end of the line, preserve that
    int new_length = arrlen(buffer->text[cursor->line]);
    if (cursor->idx == line_length - 1) {
      cursor->idx = new_length - 1;
    }

    // otherwise, if cursor's index is greater than line length, move to end of line
    else if (new_length <= cursor->idx) {
      cursor->idx = new_length - 1;
    }
  }
}
