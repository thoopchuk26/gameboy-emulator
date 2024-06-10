#include "ui.hpp"

#include <emulator.hpp>
#include <ui.hpp>

#include "ui.hpp"

void UserInterface::ui_init()
{
  SDL_Init(SDL_INIT_VIDEO);
  fmt::print("SDL_INIT\n");
  TTF_Init();
  fmt::print("TTF_INIT\n");

  SDL_CreateWindowAndRenderer(
      screenWidth, screenHeight, 0, &sdlWindow, &sdlRenderer);

  SDL_CreateWindowAndRenderer(
      16 * 8 * scale, 32 * 8 * scale, 0, &sdlDebugWindow, &sdlDebugRenderer);

  debugScreen = SDL_CreateRGBSurface(0,
                                     (16 * 8 * scale) + (16 * scale),
                                     (32 * 8 * scale) + (64 * scale),
                                     32,
                                     0x00FF0000,
                                     0x0000FF00,
                                     0x000000FF,
                                     0xFF000000);

  sdlDebugTexture = SDL_CreateTexture(sdlDebugRenderer,
                                      SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      (16 * 8 * scale) + (16 * scale),
                                      (32 * 8 * scale) + (64 * scale));

  int x, y;
  SDL_GetWindowPosition(sdlWindow, &x, &y);
  SDL_SetWindowPosition(sdlDebugWindow, x + screenWidth + 10, y);
}

void UserInterface::ui_handle_events()
{
  SDL_Event e;
  while (SDL_PollEvent(&e) > 0) {
    if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
      emulator.get_context()->die = true;
    }
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
      if (emulator.get_context()->paused) {
        emulator.get_context()->paused = false;
      } else {
        emulator.get_context()->paused = true;
      }
    }
  }
}

void UserInterface::delay(u32 ms)
{
  SDL_Delay(ms);
}

void UserInterface::update_debug_window()
{
  int xDraw = 0;
  int yDraw = 0;
  int tileNum = 0;

  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = debugScreen->w;
  rect.h = debugScreen->h;
  SDL_FillRect(debugScreen, &rect, 0xF111111);

  u16 addr = 0x8000;

  // 384 tiles, 24 x 16
  for (int y = 0; y < 24; y++) {
    for (int x = 0; x < 16; x++) {
      display_tile(
          debugScreen, addr, tileNum, xDraw + (x * scale), yDraw + (y * scale));
      xDraw += (8 * scale);
      tileNum++;
    }

    yDraw += (8 * scale);
    xDraw = 0;
  }

  SDL_UpdateTexture(
      sdlDebugTexture, NULL, debugScreen->pixels, debugScreen->pitch);
  SDL_RenderClear(sdlDebugRenderer);
  SDL_RenderCopy(sdlDebugRenderer, sdlDebugTexture, NULL, NULL);
  SDL_RenderPresent(sdlDebugRenderer);
}

void UserInterface::display_tile(
    SDL_Surface* surface, u16 startLocation, u16 tileNum, int x, int y)
{
  SDL_Rect rc;

  for (int tileY = 0; tileY < 16; tileY += 2) {
    // Grab data for the two bytes representing a line
    u8 b1 = emulator.cpu.bus.bus_read(startLocation + (tileNum * 16) + tileY);
    u8 b2 =
        emulator.cpu.bus.bus_read(startLocation + (tileNum * 16) + tileY + 1);

    for (int bit = 7; bit >= 0; bit--) {
      // Read the bits in order for each line and bitwise or to determine pixel
      // color
      u8 hi = !!(b1 & (1 << bit)) << 1;
      u8 lo = !!(b2 & (1 << bit));

      u8 color = hi | lo;

      rc.x = x + ((7 - bit) * scale);
      rc.y = y + (tileY / 2 * scale);
      rc.w = scale;
      rc.h = scale;

      SDL_FillRect(surface, &rc, tile_colors[color]);
    }
  }
}

u32 UserInterface::get_ticks()
{
  return SDL_GetTicks();
}

void UserInterface::ui_update()
{
  update_debug_window();
}
