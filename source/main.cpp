#include <iostream>
#include <string>

#include "lib.hpp"

int main()
{
  auto const lib = library {};
  auto const message = "Hello from " + lib.name + "!";
  std::cout << message << '\n';
  return 0;
}
