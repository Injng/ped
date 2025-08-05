#include <SDL3/SDL_keycode.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "buffer.h"
#include "glyph.h"
#include "rope.h"

#define INIT_WIDTH 1080
#define INIT_HEIGHT 720
#define FONT_FILE "/usr/share/fonts/TTF/JetBrainsMonoNerdFontMono-Regular.ttf"
#define pse()                                                                  \
  printf("Error: %s", SDL_GetError());                                         \
  code = 1;                                                                    \
  goto cleanup;

TTF_Font *font = NULL;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
Glyphs *glyphs = NULL;
Buffer *buffer = NULL;

int main(void)
{
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

  // initialize the buffer
  buffer = buffer_init();
  if (buffer == NULL) {
    pse();
  }

  // keep track of what line and index the user is on
  int line = 0;
  int idx = -1;
  
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
        if (event.key.key == SDLK_RETURN) {
          line++;
          idx = -1;
          buffer_newline(buffer);
        } else if (event.key.key == SDLK_BACKSPACE && idx > -1) {
          if (!buffer_delete(buffer, line, idx)) {
            pse();
          }
          idx--;
        } else if (validate_glyphs(event.key.key)) {
          if (!buffer_insert(buffer, line, idx, event.key.key)) {
            pse();
          }
          idx++;
        }
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

    // render the typed text
    uint32_t **input = buffer_text(buffer);
    if (!render_text(glyphs, renderer, input)) {
      pse();
    }
    for (int i = 0; i < arrlen(input); i++) arrfree(input[i]);
    arrfree(input);

    // present the screen
    if (!SDL_RenderPresent(renderer)) {
      pse();
    }
  }

  // cleanup
 cleanup:
  buffer_free(buffer);
  free_glyphs(glyphs);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_Quit();
  
  return code;
}
