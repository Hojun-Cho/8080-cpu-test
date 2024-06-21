#include "game.h"
#include <SDL2/SDL.h>
#define ROM_1 "roms/invaders.h"
#define ROM_2 "roms/invaders.g"
#define ROM_3 "roms/invaders.f"
#define ROM_4 "roms/invaders.e"

SDL_Window* win;
SDL_Renderer* renderer;
SDL_Texture* texture;
Machine m;

static void
sdl_init()
{
  if (SDL_Init(SDL_INIT_VIDEO))
    panic("Fail SDL_Init %s", SDL_GetError());
  if ((win = SDL_CreateWindow("Game",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH * 2,
                              SCREEN_HEIGHT * 2,
                              SDL_WINDOW_RESIZABLE |
                                SDL_WINDOW_ALLOW_HIGHDPI)) == 0) {
    panic("Can't create win\n%s", SDL_GetError());
  }
  if ((renderer = SDL_CreateRenderer(
         win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) == 0) {
    panic("Can't create renderer\n%s", SDL_GetError());
  }

  SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
  if ((texture = SDL_CreateTexture(renderer,
                                   SDL_PIXELFORMAT_RGBA32,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   SCREEN_WIDTH,
                                   SCREEN_HEIGHT)) == 0) {
    panic("Can't create Texture\n%s", SDL_GetError());
  }
}

static void
update_screen(Machine* mach)
{
  int pitch;
  void* pixel;

  if (SDL_LockTexture(texture, 0, &pixel, &pitch))
    error("Can't Lock Texture: %s", SDL_GetError());
  else
    memcpy(pixel, mach->buf, pitch * SCREEN_HEIGHT);
  SDL_UnlockTexture(texture);
}

static void
game_init()
{
  mach_init(&m, update_screen);
  m.update_screen(&m);
  mach_load(&m, ROM_1, 0x0000);
  mach_load(&m, ROM_1, 0x0800);
  mach_load(&m, ROM_1, 0x1000);
  mach_load(&m, ROM_1, 0x1800);
}

int
main()
{
  sdl_init();
  game_init();
}
