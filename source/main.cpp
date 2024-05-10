#include <iostream>
#include <string>

#include "emulator.hpp"

int main()
{
  Emulator emu;
  return emu.emulator_start(
      "C:/programming_projects/cpp_projects/gameboy-emulator/roms/"
      "dmg-acid2."
      "gb");
}
