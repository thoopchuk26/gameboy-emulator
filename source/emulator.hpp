#pragma once
#include <cart.hpp>
#include <cpu.hpp>

struct EmulatorContext
{
  bool paused;
  bool running;
  u64 ticks;
};

class Emulator
{
public:
  Emulator()
      : cpu(*this)
  {
  }

  Cartridge cart;

  int emulator_start(std::string rom);
  void emulator_cycles(int cpu_cycles);
  EmulatorContext* get_context();
  void delay(u32 ms);

private:
  CPU cpu;
  EmulatorContext context;
};
