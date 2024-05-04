#pragma once
#include <common.hpp>

struct EmulatorContext
{
  bool paused;
  bool running;
  u64 ticks;
};

class Emulator
{
public:
  void emulator_start();

private:
  EmulatorContext context;

  void delay(u32 ms);
  EmulatorContext get_context();
};
