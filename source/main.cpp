#include <emulator.hpp>

int main(int argc, char* argv[])
{
  Emulator emu;
  return emu.emulator_start(
      "C:/programming_projects/cpp_projects/gameboy-emulator/roms/"
      "03-op_sphl"
      ".gb");
}
