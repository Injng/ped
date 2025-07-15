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
  code = 1;                                                                    \
  goto cleanup;

TTF_Font *font = NULL;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
TTF_TextEngine *text_engine = NULL;
TTF_Text *text = NULL;

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

  // create a text engine to render text to the SDL renderer
  text_engine = TTF_CreateRendererTextEngine(renderer);
  if (text_engine == NULL) {
    pse();
  }

  // create a text to be rendered on the window
  text = TTF_CreateText(text_engine, font, "Hello, World!", 0);
  if (text == NULL) {
    pse();
  }

  // set text color to be black
  if (!TTF_SetTextColor(text, 0, 0, 0, 255)) {
    pse();
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
    }

    // clear window background with drawing color
    if (!SDL_RenderClear(renderer)) {
      pse();
    }
   
    // render the text on the window
    if (!TTF_DrawRendererText(text, 100, 100)) {
      pse();
    }

    // present the screen
    if (!SDL_RenderPresent(renderer)) {
      pse();
    }
  }

  // cleanup
 cleanup:
  if (text != NULL) TTF_DestroyText(text);
  if (text_engine != NULL) TTF_DestroyRendererTextEngine(text_engine);
  if (renderer != NULL) SDL_DestroyRenderer(renderer);
  if (window != NULL) SDL_DestroyWindow(window);
  if (font != NULL) TTF_CloseFont(font);
  TTF_Quit();
  SDL_Quit();
  
  return code;
}
