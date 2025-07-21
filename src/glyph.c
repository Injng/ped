#include <stdlib.h>

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "stb_ds.h"
#include "glyph.h"

Glyphs *init_glyphs(TTF_Font *font, SDL_Renderer *renderer, SDL_Color color)
{
  // initialize hash map of glyphs and allocate memory for our glyphs struct
  Encoding *textures = NULL;
  Glyphs *glyphs = malloc(sizeof(Glyphs));
  if (glyphs == NULL) {
    SDL_SetError("Failed to allocate memory for Glyphs struct");
    free(glyphs);
    return NULL;
  }

  // check if font is fixed width
  if (!TTF_FontIsFixedWidth(font)) {
    SDL_SetError("Font must be fixed width");
    free(glyphs);
    return NULL;
  }

  // iterate through the unicode codepoints and generate textures for the font
  for (int i = 48; i < GLYPHS_SIZE; i++) {
    // render a blended glyph surface
    SDL_Surface *gs = TTF_RenderGlyph_Blended(font, i, color);
    if (gs == NULL) {
      free(glyphs);
      return NULL;
    }

    // create a texture from the surface
    SDL_Texture *glyph = SDL_CreateTextureFromSurface(renderer, gs);
    SDL_DestroySurface(gs);
    if (glyph  == NULL) {
      free(glyphs);
      return NULL;
    }

    // add to the hash map with the unicode codepoint as the key
    hmput(textures, i, glyph);
  }

  // set properties and return pointer to glyphs
  glyphs->glyphs = textures;
  glyphs->font = font;
  glyphs->width = textures[0].value->w;
  glyphs->height = TTF_GetFontHeight(font);

  return glyphs;
}

void free_glyphs(Glyphs *text)
{
  if (text == NULL) return;
  for (int i = 0; i < GLYPHS_SIZE; i++) {
    SDL_DestroyTexture(text->glyphs[i].value);
  }
  hmfree(text->glyphs);
  free(text);
}

bool render_text(Glyphs *glyphs, SDL_Renderer *renderer, uint32_t *text)
{
  // exit early if no text to render
  if (arrlen(text) == 0) return true;

  // check if font is fixed width
  if (!TTF_FontIsFixedWidth(glyphs->font)) {
    SDL_SetError("Font must be fixed width");
    return false;
  }

  // render each character in the dynamic array
  for (int i = 0; i < arrlen(text); i++) {
    // destination rectangle always starts from offest and is calculated from the start
    SDL_FRect dst = {
      .x = PADDING + glyphs->width * i,
      .y = PADDING,
      .w = glyphs->width,
      .h = glyphs->height
    };

    // check if text is in glyphs
    if (hmgeti(glyphs->glyphs, text[i]) == -1) continue;

    // render the texture to the destination rectangle
    if (!SDL_RenderTexture(renderer, hmget(glyphs->glyphs, text[i]), NULL, &dst)) {
      return false;
    }
  }

  return true;
}
