#pragma once

#include <cart.hpp>
#include <cpu.hpp>
#include <ui.hpp>

struct EmulatorContext
{
  bool paused;
  bool running;
  bool die;
  u64 ticks;
};

class Emulator
{
public:
  Emulator()
      : cpu(*this)
      , ui(*this)
  {
  }

  Cartridge cart;
  CPU cpu;
  UserInterface ui;

  int emulator_start(std::string rom);
  void emulator_cycles(int cpu_cycles);
  EmulatorContext* get_context();
  void delay(u32 ms);

private:
  EmulatorContext context;
};
