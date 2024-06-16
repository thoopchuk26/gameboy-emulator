#pragma once
#include <common.hpp>
#include <gamepad.hpp>

struct GamepadState
{
  bool start;
  bool select;
  bool a;
  bool b;
  bool up;
  bool down;
  bool left;
  bool right;
};

struct GamepadContext
{
  bool button_sel;
  bool direction_sel;
  GamepadState controller;
};

class Gamepad
{
public:
  GamepadContext context;

  bool gamepad_button_sel();
  bool gamepad_dir_sel();
  void gamepad_set_sel(u8 value);
  u8 gamepad_get_output();

private:
};
