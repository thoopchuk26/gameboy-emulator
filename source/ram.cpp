#include <ram.hpp>

u8 Ram::wram_read(u16 address)
{
  address -= 0xC000;

  if (address >= 0x2000) {
    printf("INVALID WRAM ADDR %08X\n", address + 0xC000);
    exit(-1);
  }

  return ram_context.wram[address];
}

void Ram::wram_write(u16 address, u8 value)
{
  address -= 0xC000;

  ram_context.wram[address] = value;
}

u8 Ram::hram_read(u16 address)
{
  address -= 0xFF80;

  return ram_context.hram[address];
}

void Ram::hram_write(u16 address, u8 value)
{
  address -= 0xFF80;

  ram_context.hram[address] = value;
}
