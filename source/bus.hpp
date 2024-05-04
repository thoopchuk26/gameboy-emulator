#pragma once
#include <common.hpp>

class Bus
{
public:
  u8 bus_read(u16 address);
  void bus_write(u8 value, u16 address);

private:

};