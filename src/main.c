#include <stdbool.h>
#include <stdio.h>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>

#define INIT_WIDTH 1080
#define INIT_HEIGHT 720
#define FONT_FILE "/usr/share/fonts/TTF/JetBrainsMonoNerdFontMono-Regular.ttf"
#define pse()                                                                  \
  printf("Error: %s", SDL_GetError());                                         \
  code = 1;

int main(void) {
  // code to return from the program with
  int code = 0;
  
  // initialize SDL with video subsystem
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    pse();
    goto quit;
  }

  // initialize SDL_ttf
  if (!TTF_Init()) {
    pse();
    goto cleanup_ttf;
  }

  // open our font
  TTF_Font *font = TTF_OpenFont(FONT_FILE, 16);
  if (font == NULL) {
    pse();
    goto cleanup_font;
  }

  // create a resizable window
  SDL_Window *window = SDL_CreateWindow("ped", INIT_WIDTH, INIT_HEIGHT, SDL_WINDOW_RESIZABLE);
  if (window == NULL) {
    pse();
    goto cleanup_window;
  }

  // create a renderer
  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
  printf("%s", SDL_GetError());
  if (renderer == NULL) {
    pse();
    goto cleanup_renderer;
  }

  // create a text engine to render text to the SDL renderer
  TTF_TextEngine *text_engine = TTF_CreateRendererTextEngine(renderer);
  if (text_engine == NULL) {
    pse();
    goto cleanup_text_engine;
  }

  // create a text to be rendered on the window
  TTF_Text *text = TTF_CreateText(text_engine, font, "Hello, World!", 0);
  if (text == NULL) {
    pse();
    goto cleanup_text;
  }

  // set text color to be black
  if (!TTF_SetTextColor(text, 0, 0, 0, 255)) {
    pse();
    goto cleanup_all;
  }
  
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
      }
    }

    // set drawing color to white
    if (!SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255)) {
      pse();
      goto cleanup_all;
    }

    // clear window background with drawing color
    if (!SDL_RenderClear(renderer)) {
      pse();
      goto cleanup_all;
    }
   
    // render the text on the window
    if (!TTF_DrawRendererText(text, 100, 100)) {
      pse();
      goto cleanup_all;
    }

    // present the screen
    if (!SDL_RenderPresent(renderer)) {
      pse();
      goto cleanup_all;
    }
  }

  // cleanup
 cleanup_all:
  TTF_DestroyText(text);
 cleanup_text:
  TTF_DestroyRendererTextEngine(text_engine);
 cleanup_text_engine:
  SDL_DestroyRenderer(renderer);
 cleanup_renderer:
  SDL_DestroyWindow(window);
 cleanup_window:
  TTF_CloseFont(font);
 cleanup_font:
  TTF_Quit();
 cleanup_ttf:
  SDL_Quit();
 quit:
  return code;
}
