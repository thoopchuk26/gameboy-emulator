#pragma once

#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <common.hpp>

class Emulator;

static unsigned long tile_colors[4] = {
    0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};

class UserInterface
{
public:
  SDL_Window* sdlWindow;
  SDL_Renderer* sdlRenderer;
  SDL_Texture* sdlTexture;
  SDL_Surface* screen;

  SDL_Window* sdlDebugWindow;
  SDL_Renderer* sdlDebugRenderer;
  SDL_Texture* sdlDebugTexture;
  SDL_Surface* debugScreen;

  int screenWidth = 1024, screenHeight = 768;
  int scale = 4;

  UserInterface(Emulator& emu)
      : emulator(emu) {};

  void ui_init();
  void ui_handle_events();
  void delay(u32 ms);
  void update_debug_window();
  void display_tile(
      SDL_Surface* surface, u16 startLocation, u16 tileNum, int x, int y);
  u32 get_ticks();

  void ui_update();
  void ui_on_key(bool down, u32 key_code);


private:
  Emulator& emulator;
};
