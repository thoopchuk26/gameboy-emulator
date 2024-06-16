#include "gamepad.hpp"

bool Gamepad::gamepad_button_sel()
{
  return context.button_sel;
}

bool Gamepad::gamepad_dir_sel()
{
  return context.direction_sel;
}

void Gamepad::gamepad_set_sel(u8 value)
{
  context.button_sel = value & 0x20;
  context.direction_sel = value & 0x10;
}

u8 Gamepad::gamepad_get_output()
{
  u8 output = 0xCF;

  if (!gamepad_button_sel()) {
    if (context.controller.start) {
      output &= ~(1 << 3);
    }
    if (context.controller.select) {
      output &= ~(1 << 2);
    }
    if (context.controller.a) {
      output &= ~(1 << 0);
    }
    if (context.controller.b) {
      output &= ~(1 << 1);
    }
  }

  if (!gamepad_dir_sel()) {
    if (context.controller.up) {
      output &= ~(1 << 2);
    }
    if (context.controller.down) {
      output &= ~(1 << 3);
    }
    if (context.controller.left) {
      output &= ~(1 << 1);
    }
    if (context.controller.right) {
      output &= ~(1 << 0);
    }
  }

  return output;
}
