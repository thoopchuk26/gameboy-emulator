#include <bus.hpp>
#include <cart.hpp>
#include <emulator.hpp>

// 0x0000 - 0x3FFF : ROM Bank 0
// 0x4000 - 0x7FFF : ROM Bank 1 - Switchable
// 0x8000 - 0x97FF : CHR RAM
// 0x9800 - 0x9BFF : BG Map 1
// 0x9C00 - 0x9FFF : BG Map 2
// 0xA000 - 0xBFFF : Cartridge RAM
// 0xC000 - 0xCFFF : RAM Bank 0
// 0xD000 - 0xDFFF : RAM Bank 1-7 - switchable - Color only
// 0xE000 - 0xFDFF : Reserved - Echo RAM
// 0xFE00 - 0xFE9F : Object Attribute Memory
// 0xFEA0 - 0xFEFF : Reserved - Unusable
// 0xFF00 - 0xFF7F : I/O Registers
// 0xFF80 - 0xFFFE : Zero Page

u8 Bus::bus_read(u16 address)
{
  if (address < 0x8000) {
    // ROM Data
    return emulator.cart.cart_read(address);
  } else if (address < 0xA000) {
    // Char/Map Data
    return emulator.cpu.ppu.ppu_vram_read(address);
  } else if (address < 0xC000) {
    // Cartridge RAM
    return emulator.cart.cart_read(address);
  } else if (address < 0xE000) {
    // WRAM (Working RAM)
    return ram.wram_read(address);
  } else if (address < 0xFE00) {
    // reserved echo ram...
    return 0;
  } else if (address < 0xFEA0) {
    // OAM
    if (emulator.cpu.dma.dma_transferring()) {
      return 0xFF;
    }
    return emulator.cpu.ppu.ppu_oam_read(address);
  } else if (address < 0xFF00) {
    // reserved unusable...
    return 0;
  } else if (address < 0xFF80) {
    // IO Registers...
    return emulator.io.io_read(address);
  } else if (address == 0xFFFF) {
    // CPU ENABLE REGISTER...
    return emulator.cpu.cpu_get_ie_register();
  }

  return ram.hram_read(address);
}

void Bus::bus_write(u16 address, u8 value)
{
  if (address < 0x8000) {
    // ROM Data
    emulator.cart.cart_write(address, value);
  } else if (address < 0xA000) {
    // Char/Map Data
    emulator.cpu.ppu.ppu_vram_write(address, value);
  } else if (address < 0xC000) {
    // EXT-RAM
    emulator.cart.cart_write(address, value);
  } else if (address < 0xE000) {
    // WRAM
    ram.wram_write(address, value);
  } else if (address < 0xFE00) {
    // reserved echo ram
  } else if (address < 0xFEA0) {
    // OAM
    // Don't write oam if currently in the middle of a dma transfer
    if (emulator.cpu.dma.dma_transferring()) {
      return;
    }
    emulator.cpu.ppu.ppu_oam_write(address, value);
  } else if (address < 0xFF00) {
    // unusable reserved
  } else if (address < 0xFF80) {
    // IO Registers...
    emulator.io.io_write(address, value);
  } else if (address == 0xFFFF) {
    // CPU SET ENABLE REGISTER
    emulator.cpu.cpu_set_ie_register(value);
  } else {
    ram.hram_write(address, value);
  }
}

u16 Bus::bus_read16(u16 address)
{
  u16 lo = bus_read(address);
  u16 hi = bus_read(address + 1);

  return lo | (hi << 8);
}

void Bus::bus_write16(u16 address, u16 value)
{
  bus_write(address + 1, (value >> 8) & 0xFF);
  bus_write(address, value & 0xFF);
}
