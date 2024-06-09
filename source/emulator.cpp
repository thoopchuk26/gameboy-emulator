#include <thread>

#include <emulator.hpp>
#include <windows.h>

int Emulator::emulator_start(std::string rom)
{
  if (!cart.cart_load(rom)) {
    fmt::print("Failed to load ROM file: {:s}\n", rom.c_str());
    return -1;
  }

  fmt::print("Cart loaded..\n");

  ui.ui_init();

  std::thread t1(&Emulator::cpu_run, this);

  while (!context.die) {
    Sleep(1000);
    ui.ui_handle_events();

    ui.ui_update();
  }

  t1.join();

  return 0;
}

void Emulator::cpu_run()
{
  timer.timer_init();
  cpu.cpu_init();

  context.running = true;
  context.paused = false;
  context.ticks = 0;

  while (context.running && !context.die) {
    if (context.paused) {
      delay(10);
      continue;
    }
    if (!cpu.cpu_step()) {
      fmt::print("CPU Stopped\n");
      return;
    }
  }
}

void Emulator::emulator_cycles(int cpu_cycles)
{
  for (int i = 0; i < cpu_cycles; i++) {
    for (int n = 0; n < 4; n++) {
      context.ticks++;
      timer.timer_tick();
    }
    cpu.dma.dma_tick();
  }
}

void Emulator::delay(u32 ms) {}

EmulatorContext* Emulator::get_context()
{
  return &context;
}
