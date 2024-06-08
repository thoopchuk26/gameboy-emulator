#pragma once

#include <cart.hpp>
#include <common.hpp>
#include <cpu.hpp>
#include <io.hpp>
#include <timer.hpp>
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
      , timer(*this)
      , io(*this) {};

  Cartridge cart;
  CPU cpu;
  UserInterface ui;
  Timer timer;
  IO io;

  int emulator_start(std::string rom);
  void emulator_cycles(int cpu_cycles);
  EmulatorContext* get_context();
  void cpu_run();
  void delay(u32 ms);

private:
  EmulatorContext context;
};
