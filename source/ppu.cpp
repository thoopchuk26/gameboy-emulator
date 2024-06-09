#include <ppu.hpp>

void PPU::ppu_init() {}

void PPU::ppu_tick() {}

void PPU::ppu_oam_write(u16 address, u8 value)
{
  if (address >= 0xFE00) {
    address -= 0xFE00;
  }

  u8* p = static_cast<u8*>(static_cast<void*>(&context.vram));
  p[address] = value;
}

u8 PPU::ppu_oam_read(u16 address)
{
  if (address >= 0xFE00) {
    address -= 0xFE00;
  }

  u8* p = static_cast<u8*>(static_cast<void*>(&context.oam_ram));
  return p[address];
}

void PPU::ppu_vram_write(u16 address, u8 value)
{
  context.vram[address - 0x8000] = value;
}

u8 PPU::ppu_vram_read(u16 address)
{
  return context.vram[address - 0x8000];
}
