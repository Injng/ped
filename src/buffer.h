#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>

#include "rope.h"

typedef struct Buffer {
  RopeNode ***ropes;
  uint32_t **text;
} Buffer;

Buffer *buffer_init(void);

void buffer_free(Buffer *buffer);

bool buffer_validate(Buffer *buffer, int line);

bool buffer_newline(Buffer *buffer);

bool buffer_insert(Buffer *buffer, int line, int idx, uint32_t c);

bool buffer_delete(Buffer *buffer, int line, int idx);

bool buffer_text(Buffer *buffer, int line);

#endif // BUFFER_H
