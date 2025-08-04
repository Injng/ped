#include <stdbool.h>
#include <stdlib.h>

#include <SDL3/SDL_error.h>

#include "buffer.h"
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
  if (buffer == NULL) return;
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

bool buffer_insert(Buffer *buffer, int line, int idx, uint32_t c)
{
  // check if buffer is valid with the parameters given
  if (!buffer_validate(buffer, line)) return false;

  // create new rope by inserting character at given index on the line
  int line_size = arrlen(buffer->ropes[line]);
  RopeNode *new_rope = rope_insert(buffer->ropes[line][line_size - 1], c, idx);
  if (new_rope == NULL) return false;

  // add to the line array and return
  arrput(buffer->ropes[line], new_rope);
  return true;
}

bool buffer_delete(Buffer *buffer, int line, int idx)
{
  // check if buffer is valid with the parameters given
  if (!buffer_validate(buffer, line)) return false;

  // create new rope by deleting the character at the given index and line
  int line_size = arrlen(buffer->ropes[line]);
  RopeNode *new_rope = rope_delete(buffer->ropes[line][line_size - 1], idx);
  if (new_rope == NULL) return false;

  // add to the line array and return
  arrput(buffer->ropes[line], new_rope);
  return true;
}

uint32_t *buffer_text(Buffer *buffer, int line)
{
  // check if buffer is valid with the parameters given
  if (!buffer_validate(buffer, line)) return false;

  // get text from the last rope in the line
  int line_size = arrlen(buffer->ropes[line]);
  uint32_t *text = rope_text(buffer->ropes[line][line_size-1]);
  return text;
}
