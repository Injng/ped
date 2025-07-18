#ifndef GLYPH_H
#define GLYPH_H

#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>

/**
 * struct Encoding - Defines a key-value pair with the key as the unicode
 * codepoint and the value as the glyph texture.
 *
 * @key: The unicode codepoint corresponding to the character.
 * @value: The corresponding glyph texture.
 *
 * The encoding struct is meant to store information linking a unicode
 * codepoint to its respective glyph texture. It is used as a key-value pair
 * in the stb hash map implementation, in order to quickly retrieve the
 * corresponding texture to be rendered once the codepoint is known.
 */
typedef struct Encoding {
  uint32_t key;
  SDL_Texture *value;
} Encoding;

/**
 * struct Glyphs - Defines a struct that contains all the information for
 * a font's glyphs and their codepoints.
 *
 * @glyphs: A hash map with the codepoint as the key and the texture as the
 * value.
 * @font: The TTF_Font struct used to generate the glyphs.
 * @width: The width of each glyph.
 * @height: The height of the font.
 *
 * This struct stores athe hash map for codepoint-based glyph texture lookup
 * as well as the corresponding information, such as the font used for the
 * glyphs as well as their dimensions. The dimensions assume that the font
 * is monospaced, or fixed-width. Non-monospace fonts should not attempt
 * to use functions related to this struct.
 */
typedef struct Glyphs {
  Encoding *glyphs;
  TTF_Font *font;
  int width;
  int height;
} Glyphs;

/**
 * init_glyphs() - Initializes a Glyphs struct using the given parameters.
 *
 * @font: The font used to generate the glyphs.
 * @renderer: The renderer used to generate the textures.
 * @color: The color of the glyphs.
 *
 * This function returns a pointer to a Glyphs struct with its hash map
 * populated with codepoints and their corresponding glyph textures. The
 * textures are set according to the passed in color and font. free_glyphs()
 * must be called before the program exits. This function returns NULL if
 * it fails. For error information, use SDL_GetError().
 */
Glyphs *init_glyphs(TTF_Font *font, SDL_Renderer *renderer, SDL_Color color);

/**
 * free_glyphs() - Frees a Glyphs struct from memory.
 *
 * @text: The Glyphs struct to be freed.
 *
 * This function frees a Glyphs struct by first iterating through the hash
 * map and freeing all of the textures, the hash map, and then freeing the
 * memory allocated for the struct itself.
 */
void free_glyphs(Glyphs *text);

/**
 * render_text() - Uses a populated Glyphs struct to render text.
 *
 * @glyphs: The Glyphs struct to be used.
 * @renderer: The renderer used to render the text on.
 * @text: The text to be rendered as a dynamic array of unicode codepoints.
 *
 * This function utilizes the textures within the Glyphs struct to render
 * text on the screen using the SDL_Renderer. It iterates through the given
 * dynamic array text, passing each codepoint to the hash map. If the
 * codepoint does not have a corresponding texture, the function will
 * automatically skip it, which will show up as a blank space. This
 * function returns true if successful, and false if there are errors. Use
 * SDL_GetError() for more information.
 */
bool render_text(Glyphs *glyphs, SDL_Renderer *renderer, uint32_t *text);

#endif // GLYPH_H
