#include <emulator.hpp>

int Emulator::emulator_start(std::string rom)
{
  if (!cart.cart_load(rom)) {
    printf("Failed to load ROM file: %s\n", rom.c_str());
    return -1;
  }

  printf("Cart loaded..\n");

  ui.ui_init();

  cpu.cpu_init();

  context.running = true;
  context.paused = false;
  context.ticks = 0;

  while (context.running) {
    if (context.paused) {
      delay(10);
      continue;
    }
    if (!cpu.cpu_step()) {
      printf("CPU Stopped\n");
      return -3;
    }
    context.ticks++;
  }
  return 0;
}

void Emulator::emulator_cycles(int cpu_cycles) {}

void Emulator::delay(u32 ms) {}

EmulatorContext* Emulator::get_context()
{
  return &context;
}
