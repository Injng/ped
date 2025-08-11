#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL_error.h>

#include "buffer.h"
#include "cursor.h"
#include "glyph.h"
#include "rope.h"
#include "stb_ds.h"

Buffer *buffer_init(void)
{
  // allocate and initialize the buffer
  Buffer *buffer = malloc(sizeof(Buffer));
  if (buffer == NULL) {
    SDL_SetError("Failed to allocate memory for buffer");
    return NULL;
  }
  buffer->ropes = NULL;
  buffer->text = NULL;
  arrput(buffer->ropes, NULL);

  // create empty rope as the default first rope
  RopeNode *empty_rope = rope_build(NULL, 0);
  if (empty_rope == NULL) {
    buffer_free(buffer);
    return NULL;
  }
  arrput(buffer->ropes[0], empty_rope);
  return buffer;
}

void buffer_free(Buffer *buffer)
{
  // guard against null
  if (buffer == NULL) return;

  // free the ropes
  if (buffer->ropes != NULL) {
    for (int i = 0; i < arrlen(buffer->ropes); i++) {
      if (buffer->ropes[i] != NULL) {
        for (int j = 0; j < arrlen(buffer->ropes[i]); j++) {
          rope_deref(buffer->ropes[i][j]);
        }
      }
      arrfree(buffer->ropes[i]);
    }
    arrfree(buffer->ropes);
  }

  // free the cached text
  if (buffer->text != NULL) {
    for (int i = 0; i < arrlen(buffer->text); i++) {
      arrfree(buffer->text[i]);
    }
    arrfree(buffer->text);
  }
  free(buffer);
}

bool buffer_validate(Buffer *buffer, int line)
{
  // guard against passing in null
  if (buffer == NULL) {
    SDL_SetError("Buffer cannot be NULL");
    return false;
  }

  // make sure the 2d array of ropes is initialized
  if (buffer->ropes == NULL) {
    SDL_SetError("Buffer is not initialized properly");
    return false;
  }

  // make sure the line doesn't exceed the total number of lines in the buffer
  if (arrlen(buffer->ropes) <= line) {
    SDL_SetError("Line exceeds buffer size");
    return false;
  }
  
  return true;
}

bool buffer_newline(Buffer *buffer, struct Cursor *cursor)
{
  // make sure the 2d array of ropes is initialized
  if (buffer->ropes == NULL) {
    SDL_SetError("Buffer is not initialized properly");
    return false;
  }

  // split the rope at the current cursor index
  int line_size = arrlen(buffer->ropes[cursor->line]);
  RopeNode **roots = rope_split(buffer->ropes[cursor->line][line_size - 1], cursor->idx);
  if (roots == NULL) return false;

  // add pre-split rope to current line, and post-split rope to new line
  arrput(buffer->ropes[cursor->line], roots[0]);
  stbds_arrinsn(buffer->ropes, (size_t)(cursor->line + 1), (size_t)1);
  buffer->ropes[cursor->line + 1] = NULL;
  arrput(buffer->ropes[cursor->line + 1], roots[1]);

  // rebuild text cache
  for (int i = 0; i < arrlen(buffer->ropes); i++) {
    buffer_text(buffer, i);
  }

  // update cursor location
  cursor->line++;
  cursor->idx = -1;
  free(roots);
  return true;
}

bool buffer_insert(Buffer *buffer, struct Cursor *cursor, uint32_t c)
{
  // get line and idx from cursor
  int line = cursor->line;
  int idx = cursor->idx;
  
  // check if buffer is valid with the parameters given
  if (!buffer_validate(buffer, line)) return false;

  // create new rope by inserting character at given index on the line
  int line_size = arrlen(buffer->ropes[line]);
  RopeNode *new_rope = rope_insert(buffer->ropes[line][line_size - 1], c, idx);
  if (new_rope == NULL) return false;

  // add to the line array and update cursor
  arrput(buffer->ropes[line], new_rope);
  cursor->idx++;
  return true;
}

bool buffer_delete(Buffer *buffer, struct Cursor *cursor)
{
  // get line and idx from cursor
  int line = cursor->line;
  int idx = cursor->idx;
  
  // check if buffer is valid with the parameters given
  if (!buffer_validate(buffer, line)) return false;

  // create new rope by deleting the character at the given index and line
  int line_size = arrlen(buffer->ropes[line]);
  RopeNode *new_rope = rope_delete(buffer->ropes[line][line_size - 1], idx);
  if (new_rope == NULL) return false;

  // add to the line array and update cursor
  arrput(buffer->ropes[line], new_rope);
  cursor->idx--;
  return true;
}

bool buffer_text(Buffer *buffer, int line)
{
  // append to text array if line exceeds bounds
  if (line >= arrlen(buffer->text)) {
    arrput(buffer->text, NULL);
  }

  // update the given line in the cache
  int line_size = arrlen(buffer->ropes[line]);
  uint32_t *text = rope_text(buffer->ropes[line][line_size - 1]);
  arrfree(buffer->text[line]);
  buffer->text[line] = text;
  
  return true;
}
