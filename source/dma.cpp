#include <cpu.hpp>
#include <dma.hpp>
#include <windows.h>

void DMA::dma_start(u8 start)
{
  context.active = true;
  context.byte = 0;
  context.start_delay = 2;
  context.value = start;
}

void DMA::dma_tick()
{
  if (!context.active) {
    return;
  }

  if (context.start_delay) {
    context.start_delay--;
    return;
  }

  cpu.ppu.ppu_oam_write(
      context.byte, cpu.bus.bus_read((context.value * 0x100) + context.byte));

  context.byte++;

  context.active = context.byte < 0xA0;

  if (!context.active) {
    fmt::print("DMA DONE!\n");
    Sleep(2000);
  }
}

bool DMA::dma_transferring()
{
  return context.active;
}
