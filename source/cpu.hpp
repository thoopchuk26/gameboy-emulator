#pragma once
#include <common.hpp>
#include <instructions.hpp>

const u8 ZERO_FLAG_BYTE_POSITION = 7;
const u8 SUBTRACT_FLAG_BYTE_POSITION = 6;
const u8 HALF_CARRY_FLAG_BYTE_POSITION = 5;
const u8 CARRY_FLAG_BYTE_POSITION = 4;

struct cpu_registers
{
  u8 a;
  u8 b;
  u8 c;
  u8 d;
  u8 e;
  u8 f;
  u8 g;
  u8 h;
  u8 l;
  u16 pc;
  u16 sp;

  u16 get_af()
  {
    u16 res = (u16)a << 8 | (u16)f;
    return res;
  };

  void set_af(u16 value)
  {
    a = ((value & 0xFF00)) >> 8;
    f = ((value & 0xFF));
  };

  u16 get_bc()
  {
    u16 res = (u16)b << 8 | (u16)c;
    return res;
  };

  void set_bc(u16 value)
  {
    b = ((value & 0xFF00)) >> 8;
    c = ((value & 0xFF));
  };

  u16 get_de()
  {
    u16 res = (u16)d << 8 | (u16)e;
    return res;
  };

  void set_de(u16 value)
  {
    d = ((value & 0xFF00)) >> 8;
    e = ((value & 0xFF));
  };

  u16 get_hl()
  {
    u16 res = (u16)h << 8 | (u16)l;
    return res;
  };

  void set_hl(u16 value)
  {
    h = ((value & 0xFF00)) >> 8;
    l = ((value & 0xFF));
  };

  FlagRegister convert_from_u8(u8 byte)
  {
    bool zero = ((byte << ZERO_FLAG_BYTE_POSITION) & 1) != 0;
    bool subtract = ((byte << SUBTRACT_FLAG_BYTE_POSITION) & 1) != 0;
    bool half_carry = ((byte << HALF_CARRY_FLAG_BYTE_POSITION) & 1) != 0;
    bool carry = ((byte << CARRY_FLAG_BYTE_POSITION) & 1) != 0;

    FlagRegister reg = {zero, subtract, half_carry, carry};
    return reg;
  }
};

struct FlagRegister
{
  bool zero;
  bool subtract;
  bool half_carry;
  bool carry;

  explicit operator u8()
  {
    u8 res = 0;
    if (zero) {
      res = (res << ZERO_FLAG_BYTE_POSITION) | 1;
      res >>= ZERO_FLAG_BYTE_POSITION;
    }
    if (subtract) {
      res = (res << SUBTRACT_FLAG_BYTE_POSITION) | 1;
      res >>= SUBTRACT_FLAG_BYTE_POSITION;
    }
    if (half_carry) {
      res = (res << HALF_CARRY_FLAG_BYTE_POSITION) | 1;
      res >>= HALF_CARRY_FLAG_BYTE_POSITION;
    }
    if (carry) {
      res = (res << CARRY_FLAG_BYTE_POSITION) | 1;
      res >>= CARRY_FLAG_BYTE_POSITION;
    }
    return res;
  };
};

struct MemoryBus
{
  u8 memory[0xFFFF];

  u8 read_byte(uint16_t address) { return memory[address]; }
};

struct cpu_context
{
  cpu_registers registers;

  u16 fetched_data;
  u16 mem_destination;
  bool dest_is_mem;
  u8 curr_opcode;
  instruction* cur_instruction;

  bool halted;
  bool stepping;

  bool int_master_enabled;
  bool enabling_ime;
  u8 ie_register;
  u8 int_flags;
};
