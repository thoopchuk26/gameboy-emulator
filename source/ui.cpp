#include <emulator.hpp>
#include <ui.hpp>

void UserInterface::ui_init()
{
  SDL_Init(SDL_INIT_VIDEO);
  printf("SDL_INIT\n");
  TTF_Init();
  printf("TTF_INIT\n");

  SDL_CreateWindowAndRenderer(
      screenWidth, screenHeight, 0, &sdlWindow, &sdlRenderer);
}

void UserInterface::ui_handle_events()
{
  SDL_Event e;
  while (SDL_PollEvent(&e) > 0) {
    if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
      emulator.get_context()->die = true;
    }
  }
}
