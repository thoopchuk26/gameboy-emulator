#pragma once
#include <common.hpp>
#include <ram.hpp>

class Emulator;

class Bus
{
public:
  Bus(Emulator& e)
      : emulator(e)
  {
  }

  u8 bus_read(u16 address);
  void bus_write(u16 address, u8 value);
  u16 bus_read16(u16 address);
  void bus_write16(u16 value, u16 address);

private:
  Emulator& emulator;
  Ram ram;
};
