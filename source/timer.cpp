#include <emulator.hpp>
#include <timer.hpp>

void Timer::timer_init()
{
  timer_context.div = 0xAC00;
}

void Timer::timer_tick()
{
  u16 prev_div = timer_context.div;

  timer_context.div++;

  bool timer_update = false;

  switch (timer_context.tac & (0b11)) {
    case 0b00:
      timer_update = (prev_div & (1 << 9)) && (!(timer_context.div & (1 << 9)));
      break;
    case 0b01:
      timer_update = (prev_div & (1 << 3)) && (!(timer_context.div & (1 << 3)));
      break;
    case 0b10:
      timer_update = (prev_div & (1 << 5)) && (!(timer_context.div & (1 << 5)));
      break;
    case 0b11:
      timer_update = (prev_div & (1 << 7)) && (!(timer_context.div & (1 << 7)));
      break;
  }

  if (timer_update && timer_context.tac & (1 << 2)) {
    timer_context.tima++;

    if (timer_context.tima == 0xFF) {
      timer_context.tima = timer_context.tma;

      emulator.cpu.cpu_request_interrupts(IT_TIMER);
    }
  }
}

void Timer::timer_write(u16 address, u8 value)
{
  switch (address) {
    case 0xFF04:
      // DIV
      timer_context.div = 0;
      break;
    case 0xFF05:
      // TIMA
      timer_context.tima = value;
      break;
    case 0xFF06:
      // TMA
      timer_context.tma = value;
      break;
    case 0xFF07:
      // TAC
      timer_context.tac = value;
      break;
  }
}

u8 Timer::timer_read(u16 address)
{
  switch (address) {
    case 0xFF04:
      // DIV
      return timer_context.div >> 8;
    case 0xFF05:
      // TIMA
      return timer_context.tima;
    case 0xFF06:
      // TMA
      return timer_context.tma;
    case 0xFF07:
      // TAC
      return timer_context.tac;
  }
}

TimerContext* Timer::timer_get_context()
{
  return &timer_context;
}
