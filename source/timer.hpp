#pragma once

#include <common.hpp>

class Emulator;

struct TimerContext
{
  u16 div;
  u8 tima;
  u8 tma;
  u8 tac;
};

class Timer
{
public:
  Timer(Emulator& emu)
      : emulator(emu) {};

  void timer_init();

  void timer_tick();
  void timer_write(u16 address, u8 value);
  u8 timer_read(u16 address);

  TimerContext* timer_get_context();

private:
  TimerContext timer_context;

  Emulator& emulator;
};
