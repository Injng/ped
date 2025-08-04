#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>

#include "rope.h"

typedef struct Buffer {
  RopeNode ***ropes;
} Buffer;

Buffer *buffer_init(void);

void buffer_free(Buffer *buffer);

bool buffer_validate(Buffer *buffer, int line);

bool buffer_insert(Buffer *buffer, int line, int idx, uint32_t c);

bool buffer_delete(Buffer *buffer, int line, int idx);

uint32_t *buffer_text(Buffer *buffer, int line);

#endif // BUFFER_H
