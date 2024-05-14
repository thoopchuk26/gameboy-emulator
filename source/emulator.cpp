#include <thread>

#include <emulator.hpp>
#include <windows.h>

int Emulator::emulator_start(std::string rom)
{
  if (!cart.cart_load(rom)) {
    printf("Failed to load ROM file: %s\n", rom.c_str());
    return -1;
  }

  printf("Cart loaded..\n");

  ui.ui_init();

  cpu.cpu_init();

  std::thread t1(&Emulator::cpu_run, this);

  while (!context.die) {
    Sleep(1);
    ui.ui_handle_events();
  }

  t1.detach();

  return 0;
}

void Emulator::cpu_run()
{
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
      exit(-3);
    }
    context.ticks++;
  }
}

void Emulator::emulator_cycles(int cpu_cycles) {}

void Emulator::delay(u32 ms) {}

EmulatorContext* Emulator::get_context()
{
  return &context;
}
