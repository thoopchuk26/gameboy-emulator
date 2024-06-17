#include <emulator.hpp>

int main(int argc, char* argv[])
{
  Emulator emu;
  // TODO: Make path relative
  return emu.emulator_start(
      "C:/programming_projects/cpp_projects/gameboy-emulator/roms/"
      "Tetris"
      ".gb");
}
