#include <SDL3/SDL_rect.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define INIT_WIDTH 1080
#define INIT_HEIGHT 720
#define GLYPHS_SIZE 123
#define FONT_FILE "/usr/share/fonts/TTF/JetBrainsMonoNerdFontMono-Regular.ttf"
#define pse()                                                                  \
  printf("Error: %s", SDL_GetError());                                         \
  code = 1;                                                                    \
  goto cleanup;

typedef struct Encoding {
  uint32_t key;
  SDL_Texture *value;
} Encoding;

typedef struct Glyphs {
  Encoding *glyphs;
  TTF_Font *font;
  int width;
  int height;
} Glyphs;

TTF_Font *font = NULL;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
TTF_TextEngine *text_engine = NULL;
TTF_Text *text = NULL;
Glyphs *glyphs = NULL;

Glyphs *init_glyphs(TTF_Font *font, SDL_Renderer *renderer, SDL_Color color) {
  // initialize hash map of glyphs and allocate memory for our glyphs struct
  Encoding *textures = NULL;
  Glyphs *glyphs = malloc(sizeof(Glyphs));
  if (glyphs == NULL) {
    SDL_SetError("Failed to allocate memory for Glyphs struct");
    return NULL;
  }

  // check if font is fixed width
  if (!TTF_FontIsFixedWidth(font)) {
    SDL_SetError("Font must be fixed width");
    return NULL;
  }

  // iterate through the unicode codepoints and generate textures for the font
  for (int i = 48; i < GLYPHS_SIZE; i++) {
    // render a blended glyph surface
    SDL_Surface *gs = TTF_RenderGlyph_Blended(font, i, color);
    if (gs == NULL) return NULL;

    // create a texture from the surface
    SDL_Texture *glyph = SDL_CreateTextureFromSurface(renderer, gs);
    if (glyph  == NULL) return NULL;

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

void free_glyphs(Glyphs *text) {
  for (int i = 0; i < GLYPHS_SIZE; i++) {
    SDL_DestroyTexture(text->glyphs[i].value);
  }
  free(text);
}

bool render_text(Glyphs *glyphs, SDL_Renderer *renderer, uint32_t *text) {
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
      .x = 100 + glyphs->width * i,
      .y = 100,
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

int main(void) {
  // code to return from the program with
  int code = 0; 
  
  // initialize SDL with video subsystem
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    pse();
  }

  // initialize SDL_ttf
  if (!TTF_Init()) {
    pse();
  }

  // open our font
  font = TTF_OpenFont(FONT_FILE, 16);
  if (font == NULL) {
    pse();
  }

  // create a resizable window
  window = SDL_CreateWindow("ped", INIT_WIDTH, INIT_HEIGHT, SDL_WINDOW_RESIZABLE);
  if (window == NULL) {
    pse();
  }

  // create a renderer
  renderer = SDL_CreateRenderer(window, NULL);
  printf("%s", SDL_GetError());
  if (renderer == NULL) {
    pse();
  }

  // create a glyphs structure to cache the font glyphs
  SDL_Color color = { .r = 0, .g = 0, .b = 0, .a = 255 };
  glyphs = init_glyphs(font, renderer, color);
  if (glyphs == NULL) {
    pse();
  }

  // dynamic array to keep track of text that is typed on the screen
  uint32_t *text = NULL;
  
  // event loop with quit event state
  bool quit = false;
  while (!quit) {    
    // handle events by repeatedly polling from event queue
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        quit = true;
        break;
      case SDL_EVENT_KEY_DOWN:
        arrput(text, event.key.key);
        break;
      }
    }

    // set drawing color to white
    if (!SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255)) {
      pse();
    }

    // clear window background with drawing color
    if (!SDL_RenderClear(renderer)) {
      pse();
    }

    // render typed text
    if (!render_text(glyphs, renderer, text)) {
      pse();
    }

    // present the screen
    if (!SDL_RenderPresent(renderer)) {
      pse();
    }
  }

  // cleanup
 cleanup:
  if (glyphs != NULL) free_glyphs(glyphs);
  if (renderer != NULL) SDL_DestroyRenderer(renderer);
  if (window != NULL) SDL_DestroyWindow(window);
  if (font != NULL) TTF_CloseFont(font);
  TTF_Quit();
  SDL_Quit();
  
  return code;
}
