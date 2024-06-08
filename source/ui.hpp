#pragma once

#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <common.hpp>

class Emulator;

class UserInterface
{
public:
  SDL_Window* sdlWindow;
  SDL_Renderer* sdlRenderer;
  SDL_Texture* sdlTexture;
  SDL_Surface* screen;

  int screenWidth = 1024, screenHeight = 720;

  UserInterface(Emulator& emu)
      : emulator(emu) {};

  void ui_init();
  void ui_handle_events();
  void delay(u32 ms);

private:
  Emulator& emulator;
};
