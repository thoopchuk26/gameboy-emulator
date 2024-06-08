#pragma once

#include <common.hpp>

class Emulator;

class IO
{
public:
  IO(Emulator& emu)
      : emulator(emu) {};

  std::array<u8, 2> serial_data;

  void io_write(u16 address, u8 value);
  u8 io_read(u16 address);

private:
  Emulator& emulator;
};
