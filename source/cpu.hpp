#pragma once
#include <map>
#include <string>

#include <bus.hpp>
#include <common.hpp>
#include <instructions.hpp>

class Emulator;

#define CPU_FLAG_Z BIT(ctx->regs.f, 7)
#define CPU_FLAG_C BIT(ctx->regs.f, 4)

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
  Bus memory_bus;

  CPU(Emulator& emu)
      : emulator(emu)
      , memory_bus(emu)
  {
  }

  void cpu_init();
  bool cpu_step();
  void fetch_instruction();
  void fetch_data();
  void execute();

private:
  CPUContext cpu_context;
  Emulator& emulator;

  u16 reverse(u16 n);
  u16 cpu_read_reg(register_type rt);
  void cpu_set_reg(register_type rt, u16 val);
  u8 cpu_read_reg8(register_type rt);
  void cpu_set_reg8(register_type rt, u8 val);
  CPURegisters* cpu_get_registers();
  u8 cpu_get_int_flags();
  void cpu_set_int_flags(u8 value);
  void cpu_set_flags(char z, char n, char h, char c);

  void proc_ld();
  void proc_di();
  void proc_jp();
  bool check_cond();
  void proc_xor();
  void proc_nop();
  void proc_none();
};
