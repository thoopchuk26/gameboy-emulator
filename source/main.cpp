#include <iostream>
#include <string>

#include "emulator.hpp"

int main()
{
  Emulator emu;
  auto const message = "Hello from gameboy!";
  std::cout << message << '\n';
  return 0;
}
