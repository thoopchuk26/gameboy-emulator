#pragma once
#include <map>
#include <string>

#include <bus.hpp>
#include <common.hpp>
#include <instructions.hpp>

#define CPU_FLAG_Z(a) (BIT(a.registers.f, 7))
#define CPU_FLAG_N(a) (BIT(a.registers.f, 6))
#define CPU_FLAG_H(a) (BIT(a.registers.f, 5))
#define CPU_FLAG_C(a) (BIT(a.registers.f, 4))

class Emulator;

struct CPURegisters
{
  u8 a;
  u8 f;
  u8 b;
  u8 c;
  u8 d;
  u8 e;
  u8 g;
  u8 h;
  u8 l;
  u16 pc;
  u16 sp;
};

enum interrupt_type
{
  IT_VBLANK = 1,
  IT_LCD_STAT = 2,
  IT_TIMER = 4,
  IT_SERIAL = 8,
  IT_JOYPAD = 16
};

struct CPUContext
{
  CPURegisters registers;

  u16 fetched_data;
  u16 mem_destination;
  bool dest_is_mem;
  u8 cur_opcode;
  instruction* cur_instruction;

  bool halted;
  bool stepping;

  bool int_master_enabled;
  bool enabling_ime;
  u8 ie_register;
  u8 int_flags;
};

class CPU
{
public:
  CPU(Emulator& emu)
      : emulator(emu)
      , memory_bus(emu)
  {
  }

  Bus memory_bus;

  void cpu_init();
  bool cpu_step();
  void fetch_instruction();
  void fetch_data();
  void execute();

  u8 cpu_get_ie_register();
  void cpu_set_ie_register(u8 n);
  CPURegisters* cpu_get_registers();

private:
  CPUContext cpu_context;
  Emulator& emulator;

  // get/set functions for the cpu
  u16 reverse(u16 n);
  u16 cpu_read_reg(register_type rt);
  void cpu_set_reg(register_type rt, u16 val);
  u8 cpu_read_reg8(register_type rt);
  void cpu_set_reg8(register_type rt, u8 val);
  u8 cpu_get_int_flags();
  void cpu_set_int_flags(u8 value);
  void cpu_set_flags(char z, char n, char h, char c);

  // functions for handling instruction logic
  void proc_ld();
  void proc_ldh();
  void proc_di();
  void proc_jp();
  bool check_cond();
  void proc_nop();
  void proc_none();
  void proc_pop();
  void proc_push();
  void proc_call();
  void proc_jr();
  void proc_ret();
  void proc_reti();
  void proc_rst();
  void proc_inc();
  void proc_dec();
  void proc_add();
  void proc_adc();
  void proc_sub();
  void proc_sbc();
  void proc_and();
  void proc_or();
  void proc_xor();
  void proc_cp();
  void proc_rlca();
  void proc_rrca();
  void proc_rla();
  void proc_rra();
  void proc_stop();
  void proc_daa();
  void proc_cpl();
  void proc_scf();
  void proc_ccf();
  void proc_halt();
  void proc_ei();

  // handles all cb instructions
  void proc_cb();

  void goto_addr(u16 address, bool pushpc);
  bool is_16_bit(register_type rt);
  register_type decode_reg(u8 reg);

  // stack logic functions
  void stack_push(u8 data);
  u8 stack_pop();
  void stack_push16(u16 data);
  u16 stack_pop16();

  // interrupt logic functions
  void cpu_handle_interrupts();
  void cpu_request_interrupts(interrupt_type t);
  void interrupt_handle(u16 address);
  bool interrupt_check(u16 address, interrupt_type t);
};
