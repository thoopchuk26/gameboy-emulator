#include <emulator.hpp>
#include <io.hpp>

void IO::io_write(u16 address, u8 value)
{
  if (address == 0xFF01) {
    serial_data[0] = value;
    return;
  }
  if (address == 0xFF02) {
    serial_data[1] = value;
    return;
  }
  if (BETWEEN(address, 0xFF04, 0xFF07)) {
    emulator.timer.timer_write(address, value);
    return;
  }
  if (address == 0xFF0F) {
    emulator.cpu.cpu_set_int_flags(value);
    return;
  }
  if (address == 0xFF46) {
    emulator.cpu.dma.dma_start(value);
    fmt::print("DMA start\n");
  }

  fmt::print("UNSUPPORTED bus_write({:04X})\n", address);
}

u8 IO::io_read(u16 address)
{
  if (address == 0xFF01) {
    return serial_data[0];
  }
  if (address == 0xFF02) {
    return serial_data[1];
  }
  if (BETWEEN(address, 0xFF04, 0xFF07)) {
    return emulator.timer.timer_read(address);
  }
  if (address == 0xFF0F) {
    return emulator.cpu.cpu_get_int_flags();
  }

  if (address == 0xFF44) {
    return ly++;
  }

  fmt::print("UNSUPPORTED bus_read({:04X})\n", address);
  return 0;
}
