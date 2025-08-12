#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>
#include <stdint.h>

#include "rope.h"

struct Cursor;

typedef enum {
  ACTION_INSERT,
  ACTION_DELETE,
  ACTION_NEWLINE,
} ActionType;

/**
 * struct Action - Stores information about a user action.
 *
 * @type: The type of action performed.
 * @line: The line the cursor was on before the action was performed.
 * @idx: The character index the cursor was on before the action.
 *
 * This is a struct to hold information about a particular action that was
 * performed. It is meant to be stored into a dynamic array in order to
 * keep a history of previous actions and allow undo and redo functionality.
 */
typedef struct Action {
  ActionType type;
  int line;
  int idx;
} Action;

/**
 * struct Buffer - Stores rope tree and cached text for the buffer.
 *
 * @ropes: A 2D dynamic array of ropes.
 * @text: A 2D dynamic array of unicode codepoints.
 * @undo: A dynamic array of action history.
 * @redo: A dynamic array of undo history.
 *
 * This is a struct to hold information about a buffer. It holds a 2D
 * dynamic array of ropes, where subarray of ropes represents a different
 * line in the buffer, and the last rope in each subarray is the newest
 * version of that line. It also caches text in each line for efficiency,
 * with each subarray representing a different line.
 */
typedef struct Buffer {
  RopeNode ***ropes;
  uint32_t **text;
  Action *undo;
  Action *redo;
} Buffer;

/**
 * buffer_init() - Initializes a new Buffer struct.
 *
 * This function initializes a new Buffer struct by allocating memory
 * for it and returning the pointer. By default, it initializes the
 * rope array with an empty rope as the first element in the first line.
 * This function returns NULL if it fails. For error information, use
 * SDL_GetError().
 */
Buffer *buffer_init(void);

/**
 * buffer_free() - Frees a Buffer struct.
 *
 * @buffer: The Buffer struct to be freed.
 *
 * This function frees all of the stored rope nodes with rope_deref() and
 * then frees the dynamic arrays holding the nodes. It also frees the
 * dynamic array of text. If NULL is passed, nothing will happen.
 */
void buffer_free(Buffer *buffer);

/**
 * buffer_validate() - Validates a buffer using the given parameters.
 *
 * @buffer: The Buffer struct to be validated.
 * @line: The line to validate for (zero-indexed).
 *
 * This function validates that the buffer has been properly initialized
 * and that the queried line exists. It returns true on success and false
 * on failure. For error information, use SDL_GetError().
 */
bool buffer_validate(Buffer *buffer, int line);

/**
 * buffer_newline() - Inserts a new line in the buffer.
 *
 * @buffer: The Buffer struct to use.
 * @cursor: The Cursor struct to update.
 *
 * This function inserts a new line into the dynamic array of ropes in the
 * Buffer struct. By default, it initializes the first rope of the new line
 * to be an empty rope. It will also automatically update the state of the
 * cursor to be on the new line. It returns true on success and false on
 * failure. For error information, use SDL_GetError().
 */
bool buffer_newline(Buffer *buffer, struct Cursor *cursor);

/**
 * buffer_insert() - Inserts a character into the buffer.
 *
 * @buffer: The Buffer struct to use.
 * @cursor: The Cursor struct to update.
 * @c: The unicode codepoint of the character to insert.
 *
 * This function inserts a character into the buffer at the given line,
 * which is zero-indexed, and at a given index within that line. The index
 * and line is given within the Cursor struct that is passed in. It does
 * this by creating a new rope using rope_insert() and saving it into the
 * array of ropes. It returns true on success and false on failure. For
 * error information, use SDL_GetError().
 */
bool buffer_insert(Buffer *buffer, struct Cursor *cursor, uint32_t c);

/**
 * buffer_delete() - Deletes a character in the buffer.
 *
 * @buffer: The Buffer struct to use.
 * @cursor: The Cursor struct to update.
 *
 * This function deletes a character in a buffer at the given line
 * and index in the Cursor struct. It does this by creating a new rope
 * using rope_delete() and saving it into the array of ropes. It returns
 * true on success and false on failure. For error information, use
 * SDL_GetError().
 */
bool buffer_delete(Buffer *buffer, struct Cursor *cursor);

/**
 * buffer_text() - Update the cache of text in the buffer.
 *
 * @buffer: The Buffer struct to use.
 * @line: The line to update.
 *
 * This function updates the cached text in the buffer for the line
 * given. It does this by calling rope_text() on the given line and
 * replacing the correct element within the text dynamic array. This
 * function returns true on success and false on failure. For error
 * information, use SDL_GetError().
 */
bool buffer_text(Buffer *buffer, int line);

#endif // BUFFER_H
