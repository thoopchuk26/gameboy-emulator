#include "cpu.hpp"

#include <cpu.hpp>
#include <emulator.hpp>

#include "cpu.hpp"

register_type rt_lookup[] = {RT_B, RT_C, RT_D, RT_E, RT_H, RT_L, RT_HL, RT_A};

void CPU::cpu_init()
{
  cpu_context.registers.pc = 0x100;
  cpu_context.registers.sp = 0xFFFE;
  cpu_context.registers.a = 0x01;
  cpu_context.registers.f = 0xB0;
  cpu_context.registers.b = 0x00;
  cpu_context.registers.c = 0x13;
  cpu_context.registers.d = 0x00;
  cpu_context.registers.e = 0xD8;
  cpu_context.registers.h = 0x01;
  cpu_context.registers.l = 0x4D;
  cpu_context.ie_register = 0;
  cpu_context.int_flags = 0;
  cpu_context.int_master_enabled = false;
  cpu_context.enabling_ime = false;

  emulator.timer.timer_get_context()->div = 0xABCC;
}

CPUContext* CPU::cpu_get_context()
{
  return &cpu_context;
}

void CPU::fetch_instruction()
{
  cpu_context.cur_opcode = bus.bus_read(cpu_context.registers.pc++);
  cpu_context.cur_instruction = instruction_by_opcode(cpu_context.cur_opcode);
}

void CPU::fetch_data()
{
  cpu_context.mem_destination = 0;
  cpu_context.dest_is_mem = false;

  if (cpu_context.cur_instruction == NULL) {
    return;
  }

  switch (cpu_context.cur_instruction->mode) {
    case AM_IMP:
      return;

    case AM_R:
      cpu_context.fetched_data =
          cpu_read_reg(cpu_context.cur_instruction->reg_1);
      return;

    case AM_R_R:
      cpu_context.fetched_data =
          cpu_read_reg(cpu_context.cur_instruction->reg_2);
      return;

    case AM_R_D8:
      cpu_context.fetched_data = bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);
      cpu_context.registers.pc++;
      return;

    case AM_R_D16:
    case AM_D16: {
      u16 lo = bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);

      u16 hi = bus.bus_read(cpu_context.registers.pc + 1);
      emulator.emulator_cycles(1);

      cpu_context.fetched_data = lo | (hi << 8);

      cpu_context.registers.pc += 2;

      return;
    }

    case AM_MR_R:
      cpu_context.fetched_data =
          cpu_read_reg(cpu_context.cur_instruction->reg_2);
      cpu_context.mem_destination =
          cpu_read_reg(cpu_context.cur_instruction->reg_1);
      cpu_context.dest_is_mem = true;

      if (cpu_context.cur_instruction->reg_1 == RT_C) {
        cpu_context.mem_destination |= 0xFF00;
      }

      return;

    case AM_R_MR: {
      u16 addr = cpu_read_reg(cpu_context.cur_instruction->reg_2);

      if (cpu_context.cur_instruction->reg_2 == RT_C) {
        addr |= 0xFF00;
      }

      cpu_context.fetched_data = bus.bus_read(addr);
      emulator.emulator_cycles(1);
    }
      return;

    case AM_R_HLI:
      cpu_context.fetched_data =
          bus.bus_read(cpu_read_reg(cpu_context.cur_instruction->reg_2));
      emulator.emulator_cycles(1);
      cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) + 1);
      return;

    case AM_R_HLD:
      cpu_context.fetched_data =
          bus.bus_read(cpu_read_reg(cpu_context.cur_instruction->reg_2));
      emulator.emulator_cycles(1);
      cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) - 1);
      return;

    case AM_HLI_R:
      cpu_context.fetched_data =
          cpu_read_reg(cpu_context.cur_instruction->reg_2);
      cpu_context.mem_destination =
          cpu_read_reg(cpu_context.cur_instruction->reg_1);
      cpu_context.dest_is_mem = true;
      cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) + 1);
      return;

    case AM_HLD_R:
      cpu_context.fetched_data =
          cpu_read_reg(cpu_context.cur_instruction->reg_2);
      cpu_context.mem_destination =
          cpu_read_reg(cpu_context.cur_instruction->reg_1);
      cpu_context.dest_is_mem = true;
      cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) - 1);
      return;

    case AM_R_A8:
      cpu_context.fetched_data = bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);
      cpu_context.registers.pc++;
      return;

    case AM_A8_R:
      cpu_context.mem_destination =
          bus.bus_read(cpu_context.registers.pc) | 0xFF00;
      cpu_context.dest_is_mem = true;
      emulator.emulator_cycles(1);
      cpu_context.registers.pc++;
      return;

    case AM_HL_SPR:
      cpu_context.fetched_data = bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);
      cpu_context.registers.pc++;
      return;

    case AM_D8:
      cpu_context.fetched_data = bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);
      cpu_context.registers.pc++;
      return;

    case AM_A16_R:
    case AM_D16_R: {
      u16 lo = bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);

      u16 hi = bus.bus_read(cpu_context.registers.pc + 1);
      emulator.emulator_cycles(1);

      cpu_context.mem_destination = lo | (hi << 8);
      cpu_context.dest_is_mem = true;

      cpu_context.registers.pc += 2;
      cpu_context.fetched_data =
          cpu_read_reg(cpu_context.cur_instruction->reg_2);
    }
      return;

    case AM_MR_D8:
      cpu_context.fetched_data = bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);
      cpu_context.registers.pc++;
      cpu_context.mem_destination =
          cpu_read_reg(cpu_context.cur_instruction->reg_1);
      cpu_context.dest_is_mem = true;
      return;

    case AM_MR:
      cpu_context.mem_destination =
          cpu_read_reg(cpu_context.cur_instruction->reg_1);
      cpu_context.dest_is_mem = true;
      cpu_context.fetched_data =
          bus.bus_read(cpu_read_reg(cpu_context.cur_instruction->reg_1));
      emulator.emulator_cycles(1);
      return;

    case AM_R_A16: {
      u16 lo = bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);

      u16 hi = bus.bus_read(cpu_context.registers.pc + 1);
      emulator.emulator_cycles(1);

      u16 addr = lo | (hi << 8);

      cpu_context.registers.pc += 2;
      cpu_context.fetched_data = bus.bus_read(addr);
      emulator.emulator_cycles(1);

      return;
    }

    default:
      printf("Unknown Addressing Mode! %d (%02X)\n",
             cpu_context.cur_instruction->mode,
             cpu_context.cur_opcode);
      exit(-7);
      return;
  }
}

bool CPU::cpu_step()
{
  if (!cpu_context.halted) {
    u16 pc = cpu_context.registers.pc;

    fetch_instruction();
    emulator.emulator_cycles(1);
    fetch_data();

    std::string flags;
    flags += (cpu_context.registers.f & (1 << 7) ? 'Z' : '-');
    flags += (cpu_context.registers.f & (1 << 6) ? 'N' : '-');
    flags += (cpu_context.registers.f & (1 << 5) ? 'H' : '-');
    flags += (cpu_context.registers.f & (1 << 4) ? 'C' : '-');

    std::string inst = "";
    inst = instruction_to_str(&cpu_context);
    printf(
        "%08lX - %04X: %-12s (%02X %02X %02X) A: %02X F:%s BC: %02X%02X DE: "
        "%02X%02X HL: %02X%02X\n",
        emulator.get_context()->ticks,
        pc,
        inst.c_str(),
        cpu_context.cur_opcode,
        bus.bus_read(pc + 1),
        bus.bus_read(pc + 2),
        cpu_context.registers.a,
        flags.c_str(),
        cpu_context.registers.b,
        cpu_context.registers.c,
        cpu_context.registers.d,
        cpu_context.registers.e,
        cpu_context.registers.h,
        cpu_context.registers.l);

    if (cpu_context.cur_instruction == NULL) {
      printf("Unknown Instruction! %02X\n", cpu_context.cur_opcode);
      return false;
    }

    debug_update();
    debug_print();

    execute();
  } else {
    emulator.emulator_cycles(1);

    if (cpu_context.int_flags) {
      cpu_context.halted = false;
    }
  }

  if (cpu_context.int_master_enabled) {
    cpu_handle_interrupts();
    cpu_context.enabling_ime = false;
  }

  if (cpu_context.enabling_ime) {
    cpu_context.int_master_enabled = true;
  }

  return true;
}

void CPU::execute()
{
  // execute the correct instruction function
  switch (cpu_context.cur_instruction->type) {
    case IN_NOP:
      proc_nop();
      break;
    case IN_LD:
      proc_ld();
      break;
    case IN_INC:
      proc_inc();
      break;
    case IN_DEC:
      proc_dec();
      break;
    case IN_RLCA:
      proc_rlca();
      break;
    case IN_ADD:
      proc_add();
      break;
    case IN_RRCA:
      proc_rrca();
      break;
    case IN_STOP:
      proc_stop();
      break;
    case IN_RLA:
      proc_rla();
      break;
    case IN_JR:
      proc_jr();
      break;
    case IN_RRA:
      proc_rra();
      break;
    case IN_DAA:
      proc_daa();
      break;
    case IN_CPL:
      proc_cpl();
      break;
    case IN_SCF:
      proc_scf();
      break;
    case IN_CCF:
      proc_ccf();
      break;
    case IN_HALT:
      proc_halt();
      break;
    case IN_ADC:
      proc_adc();
      break;
    case IN_SUB:
      proc_sub();
      break;
    case IN_SBC:
      proc_sbc();
      break;
    case IN_AND:
      proc_and();
      break;
    case IN_XOR:
      proc_xor();
      break;
    case IN_OR:
      proc_or();
      break;
    case IN_CP:
      proc_cp();
      break;
    case IN_POP:
      proc_pop();
      break;
    case IN_JP:
      proc_jp();
      break;
    case IN_PUSH:
      proc_push();
      break;
    case IN_RET:
      proc_ret();
      break;
    case IN_CB:
      proc_cb();
      break;
    case IN_CALL:
      proc_call();
      break;
    case IN_RETI:
      proc_reti();
      break;
    case IN_LDH:
      proc_ldh();
      break;
    case IN_DI:
      proc_di();
      break;
    case IN_EI:
      proc_ei();
      break;
    case IN_RST:
      proc_rst();
      break;
    case IN_NONE:
      proc_none();
      break;
  }
}

u16 CPU::reverse(u16 n)
{
  return ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
}

u16 CPU::cpu_read_reg(register_type rt)
{
  switch (rt) {
    case RT_A:
      return cpu_context.registers.a;
    case RT_F:
      return cpu_context.registers.f;
    case RT_B:
      return cpu_context.registers.b;
    case RT_C:
      return cpu_context.registers.c;
    case RT_D:
      return cpu_context.registers.d;
    case RT_E:
      return cpu_context.registers.e;
    case RT_H:
      return cpu_context.registers.h;
    case RT_L:
      return cpu_context.registers.l;

    case RT_AF:
      return reverse(*((u16*)&cpu_context.registers.a));
    case RT_BC:
      return reverse(*((u16*)&cpu_context.registers.b));
    case RT_DE:
      return reverse(*((u16*)&cpu_context.registers.d));
    case RT_HL:
      return reverse(*((u16*)&cpu_context.registers.h));

    case RT_PC:
      return cpu_context.registers.pc;
    case RT_SP:
      return cpu_context.registers.sp;
    default:
      return 0;
  }
}

void CPU::cpu_set_reg(register_type rt, u16 val)
{
  switch (rt) {
    case RT_A:
      cpu_context.registers.a = val & 0xFF;
      break;
    case RT_F:
      cpu_context.registers.f = val & 0xFF;
      break;
    case RT_B:
      cpu_context.registers.b = val & 0xFF;
      break;
    case RT_C: {
      cpu_context.registers.c = val & 0xFF;
    } break;
    case RT_D:
      cpu_context.registers.d = val & 0xFF;
      break;
    case RT_E:
      cpu_context.registers.e = val & 0xFF;
      break;
    case RT_H:
      cpu_context.registers.h = val & 0xFF;
      break;
    case RT_L:
      cpu_context.registers.l = val & 0xFF;
      break;

    case RT_AF:
      *((u16*)&cpu_context.registers.a) = reverse(val);
      break;
    case RT_BC:
      *((u16*)&cpu_context.registers.b) = reverse(val);
      break;
    case RT_DE:
      *((u16*)&cpu_context.registers.d) = reverse(val);
      break;
    case RT_HL: {
      *((u16*)&cpu_context.registers.h) = reverse(val);
      break;
    }

    case RT_PC:
      cpu_context.registers.pc = val;
      break;
    case RT_SP:
      cpu_context.registers.sp = val;
      break;
    case RT_NONE:
      break;
  }
}

u8 CPU::cpu_read_reg8(register_type rt)
{
  switch (rt) {
    case RT_A:
      return cpu_context.registers.a;
    case RT_F:
      return cpu_context.registers.f;
    case RT_B:
      return cpu_context.registers.b;
    case RT_C:
      return cpu_context.registers.c;
    case RT_D:
      return cpu_context.registers.d;
    case RT_E:
      return cpu_context.registers.e;
    case RT_H:
      return cpu_context.registers.h;
    case RT_L:
      return cpu_context.registers.l;
    case RT_HL: {
      return bus.bus_read(cpu_read_reg(RT_HL));
    }
    default:
      printf("**ERR INVALID REG8: %d\n", rt);
      exit(-5);
  }
}

void CPU::cpu_set_reg8(register_type rt, u8 val)
{
  switch (rt) {
    case RT_A:
      cpu_context.registers.a = val & 0xFF;
      break;
    case RT_F:
      cpu_context.registers.f = val & 0xFF;
      break;
    case RT_B:
      cpu_context.registers.b = val & 0xFF;
      break;
    case RT_C:
      cpu_context.registers.c = val & 0xFF;
      break;
    case RT_D:
      cpu_context.registers.d = val & 0xFF;
      break;
    case RT_E:
      cpu_context.registers.e = val & 0xFF;
      break;
    case RT_H:
      cpu_context.registers.h = val & 0xFF;
      break;
    case RT_L:
      cpu_context.registers.l = val & 0xFF;
      break;
    case RT_HL:
      bus.bus_write(cpu_read_reg(RT_HL), val);
      break;
    default:
      printf("**ERR INVALID REG8: %d\n", rt);
      exit(-5);
  }
}

CPURegisters* CPU::cpu_get_registers()
{
  return &cpu_context.registers;
}

u8 CPU::cpu_get_int_flags()
{
  return cpu_context.int_flags;
}

void CPU::cpu_set_int_flags(u8 value)
{
  cpu_context.int_flags = value;
}

void CPU::cpu_set_flags(char z, char n, char h, char c)
{
  if (z != -1) {
    BIT_SET(cpu_context.registers.f, 7, z);
  }

  if (n != -1) {
    BIT_SET(cpu_context.registers.f, 6, n);
  }

  if (h != -1) {
    BIT_SET(cpu_context.registers.f, 5, h);
  }

  if (c != -1) {
    BIT_SET(cpu_context.registers.f, 4, c);
  }
}

void CPU::cpu_handle_interrupts()
{
  if (interrupt_check(0x40, IT_VBLANK)) {
  } else if (interrupt_check(0x48, IT_LCD_STAT)) {
  } else if (interrupt_check(0x50, IT_TIMER)) {
  } else if (interrupt_check(0x58, IT_SERIAL)) {
  } else if (interrupt_check(0x60, IT_JOYPAD)) {
  }
}

void CPU::cpu_request_interrupts(interrupt_type t)
{
  cpu_context.int_flags |= t;
}

void CPU::interrupt_handle(u16 address)
{
  stack_push16(cpu_context.registers.pc);
  cpu_context.registers.pc = address;
}

bool CPU::interrupt_check(u16 address, interrupt_type it)
{
  if (cpu_context.int_flags & it && cpu_context.ie_register & it) {
    interrupt_handle(address);
    cpu_context.int_flags &= ~it;
    cpu_context.halted = false;
    cpu_context.int_master_enabled = false;

    return true;
  }

  return false;
}

bool CPU::is_16_bit(register_type rt)
{
  return rt >= RT_AF;
}

register_type CPU::decode_reg(u8 reg)
{
  if (reg > 0b111) {
    return RT_NONE;
  }
  return rt_lookup[reg];
}

u8 CPU::cpu_get_ie_register()
{
  return cpu_context.ie_register;
}

void CPU::cpu_set_ie_register(u8 n)
{
  cpu_context.ie_register = n;
}

void CPU::stack_push(u8 data)
{
  cpu_context.registers.sp--;
  bus.bus_write(cpu_context.registers.sp, data);
}

u8 CPU::stack_pop()
{
  return bus.bus_read(cpu_context.registers.sp++);
}

void CPU::stack_push16(u16 data)
{
  stack_push((data >> 8) & 0xFF);
  stack_push(data & 0xFF);
}

u16 CPU::stack_pop16()
{
  u16 lo = stack_pop();
  u16 hi = stack_pop();

  return (hi << 8) | lo;
}

void CPU::proc_none()
{
  printf("INVALID INSTRUCTION!\n");
  exit(-7);
}

void CPU::proc_cb()
{
  u8 op = cpu_context.fetched_data;
  register_type reg = decode_reg(op & 0b111);
  u8 bit = (op >> 3) & 0b111;
  u8 bit_op = (op >> 6) & 0b11;
  u8 reg_val = cpu_read_reg8(reg);

  emulator.emulator_cycles(1);

  if (reg == RT_HL) {
    emulator.emulator_cycles(2);
  }

  switch (bit_op) {
    case 1:
      // BIT
      cpu_set_flags(!(reg_val & (1 << bit)), 0, 1, -1);
      return;

    case 2:
      // RST
      reg_val &= ~(1 << bit);
      cpu_set_reg8(reg, reg_val);
      return;

    case 3:
      // SET
      reg_val |= (1 << bit);
      cpu_set_reg8(reg, reg_val);
      return;
  }

  bool flagC = CPU_FLAG_C(cpu_context);

  switch (bit) {
    case 0: {
      // RLC
      bool setC = false;
      u8 result = (reg_val << 1) & 0xFF;

      if ((reg_val & (1 << 7)) != 0) {
        result |= 1;
        setC = true;
      }

      cpu_set_reg8(reg, result);
      cpu_set_flags(result == 0, false, false, setC);
    }
      return;

    case 1: {
      // RRC
      u8 old = reg_val;
      reg_val >>= 1;
      reg_val |= (old << 7);

      cpu_set_reg8(reg, reg_val);
      cpu_set_flags(!reg_val, false, false, old & 1);
    }
      return;

    case 2: {
      // RL
      u8 old = reg_val;
      reg_val <<= 1;
      reg_val |= flagC;

      cpu_set_reg8(reg, reg_val);
      cpu_set_flags(!reg_val, false, false, !!(old & 0x80));
    }
      return;

    case 3: {
      // RR
      u8 old = reg_val;
      reg_val >>= 1;

      reg_val |= (flagC << 7);

      cpu_set_reg8(reg, reg_val);
      cpu_set_flags(!reg_val, false, false, old & 1);
    }
      return;

    case 4: {
      // SLA
      u8 old = reg_val;
      reg_val <<= 1;

      cpu_set_reg8(reg, reg_val);
      cpu_set_flags(!reg_val, false, false, !!(old & 0x80));
    }
      return;

    case 5: {
      // SRA
      u8 u = (int8_t)reg_val >> 1;
      cpu_set_reg8(reg, u);
      cpu_set_flags(!u, 0, 0, reg_val & 1);
    }
      return;

    case 6: {
      // SWAP
      reg_val = ((reg_val & 0xF0) >> 4) | ((reg_val & 0xF) << 4);
      cpu_set_reg8(reg, reg_val);
      cpu_set_flags(reg_val == 0, false, false, false);
    }
      return;

    case 7: {
      // SRL
      u8 u = reg_val >> 1;
      cpu_set_reg8(reg, u);
      cpu_set_flags(!u, 0, 0, reg_val & 1);
    }
      return;
  }

  fprintf(stderr, "ERROR: INVALID CB: %02X", op);
  exit(-5);
}

void CPU::proc_rlca()
{
  u8 u = cpu_context.registers.a;
  bool c = (u >> 7) & 1;
  u = (u << 1) | c;
  cpu_context.registers.a = u;

  cpu_set_flags(0, 0, 0, c);
}

void CPU::proc_rrca()
{
  u8 b = cpu_context.registers.a & 1;
  cpu_context.registers.a >>= 1;
  cpu_context.registers.a |= (b << 7);

  cpu_set_flags(0, 0, 0, b);
}

void CPU::proc_rla()
{
  u8 u = cpu_context.registers.a;
  u8 cf = CPU_FLAG_C(cpu_context);
  u8 c = (u >> 7) & 1;

  cpu_context.registers.a = (u << 1) | cf;
  cpu_set_flags(0, 0, 0, c);
}

void CPU::proc_stop()
{
  fprintf(stderr, "STOPPING!\n");
  exit(-5);
}

void CPU::proc_daa()
{
  u8 u = 0;
  int fc = 0;

  if (CPU_FLAG_H(cpu_context)
      || (!CPU_FLAG_N(cpu_context) && (cpu_context.registers.a & 0xF) > 9))
  {
    u = 6;
  }

  if (CPU_FLAG_C(cpu_context)
      || (!CPU_FLAG_N(cpu_context) && cpu_context.registers.a > 0x99))
  {
    u |= 0x60;
    fc = 1;
  }

  cpu_context.registers.a += CPU_FLAG_N(cpu_context) ? -u : u;

  cpu_set_flags(cpu_context.registers.a == 0, -1, 0, fc);
}

void CPU::proc_cpl()
{
  cpu_context.registers.a = ~cpu_context.registers.a;
  cpu_set_flags(-1, 1, 1, -1);
}

void CPU::proc_scf()
{
  cpu_set_flags(-1, 0, 0, 1);
}

void CPU::proc_ccf()
{
  cpu_set_flags(-1, 0, 0, CPU_FLAG_C(cpu_context) ^ 1);
}

void CPU::proc_halt()
{
  cpu_context.halted = true;
}

void CPU::proc_rra()
{
  u8 carry = CPU_FLAG_C(cpu_context);
  u8 new_c = cpu_context.registers.a & 1;

  cpu_context.registers.a >>= 1;
  cpu_context.registers.a |= (carry << 7);

  cpu_set_flags(0, 0, 0, new_c);
}

void CPU::proc_and()
{
  cpu_context.registers.a &= cpu_context.fetched_data;
  cpu_set_flags(cpu_context.registers.a == 0, 0, 1, 0);
}

void CPU::proc_xor()
{
  cpu_context.registers.a ^= cpu_context.fetched_data & 0xFF;
  cpu_set_flags(cpu_context.registers.a == 0, 0, 0, 0);
}

void CPU::proc_or()
{
  cpu_context.registers.a |= cpu_context.fetched_data & 0xFF;
  cpu_set_flags(cpu_context.registers.a == 0, 0, 0, 0);
}

void CPU::proc_cp()
{
  int n = (int)cpu_context.registers.a - (int)cpu_context.fetched_data;

  cpu_set_flags(n == 0,
                1,
                ((int)cpu_context.registers.a & 0x0F)
                        - ((int)cpu_context.fetched_data & 0x0F)
                    < 0,
                n < 0);
}

void CPU::proc_di()
{
  cpu_context.int_master_enabled = false;
}

void CPU::proc_ei()
{
  cpu_context.enabling_ime = true;
}

static bool is_16_bit(register_type rt)
{
  return rt >= RT_AF;
}

void CPU::proc_ld()
{
  if (cpu_context.dest_is_mem) {
    // LD (BC), A for instance...

    if (is_16_bit(cpu_context.cur_instruction->reg_2)) {
      // if 16 bit register...
      emulator.emulator_cycles(1);
      bus.bus_write16(cpu_context.mem_destination, cpu_context.fetched_data);
    } else {
      bus.bus_write(cpu_context.mem_destination, (u8)cpu_context.fetched_data);
    }

    emulator.emulator_cycles(1);

    return;
  }

  if (cpu_context.cur_instruction->mode == AM_HL_SPR) {
    u8 hflag = (cpu_read_reg(cpu_context.cur_instruction->reg_2) & 0xF)
            + (cpu_context.fetched_data & 0xF)
        >= 0x10;

    u8 cflag = (cpu_read_reg(cpu_context.cur_instruction->reg_2) & 0xFF)
            + (cpu_context.fetched_data & 0xFF)
        >= 0x100;

    cpu_set_flags(0, 0, hflag, cflag);
    cpu_set_reg(cpu_context.cur_instruction->reg_1,
                cpu_read_reg(cpu_context.cur_instruction->reg_2)
                    + (int8_t)cpu_context.fetched_data);

    return;
  }

  cpu_set_reg(cpu_context.cur_instruction->reg_1, cpu_context.fetched_data);
}

void CPU::proc_ldh()
{
  if (cpu_context.cur_instruction->reg_1 == RT_A) {
    cpu_set_reg(cpu_context.cur_instruction->reg_1,
                (u16)bus.bus_read(0xFF00 | cpu_context.fetched_data));
  } else {
    bus.bus_write(cpu_context.mem_destination, cpu_context.registers.a);
  }

  emulator.emulator_cycles(1);
}

bool CPU::check_cond()
{
  bool z = CPU_FLAG_Z(cpu_context);
  bool c = CPU_FLAG_C(cpu_context);

  switch (cpu_context.cur_instruction->cond) {
    case CT_NONE:
      return true;
    case CT_C:
      return c;
    case CT_NC:
      return !c;
    case CT_Z:
      return z;
    case CT_NZ:
      return !z;
  }

  return false;
}

void CPU::proc_nop() {}

void CPU::goto_addr(u16 addr, bool pushpc)
{
  if (check_cond()) {
    if (pushpc) {
      emulator.emulator_cycles(2);
      stack_push16(cpu_context.registers.pc);
    }

    cpu_context.registers.pc = addr;
    emulator.emulator_cycles(1);
  }
}

void CPU::proc_jp()
{
  goto_addr(cpu_context.fetched_data, false);
}

void CPU::proc_jr()
{
  int8_t rel = (int8_t)(cpu_context.fetched_data & 0xFF);
  u16 addr = cpu_context.registers.pc + (u16)rel;

  goto_addr(addr, false);
}

void CPU::proc_call()
{
  goto_addr(cpu_context.fetched_data, true);
}

void CPU::proc_rst()
{
  goto_addr((u16)cpu_context.cur_instruction->param, true);
}

void CPU::proc_ret()
{
  if (cpu_context.cur_instruction->cond != CT_NONE) {
    emulator.emulator_cycles(1);
  }

  if (check_cond()) {
    u16 lo = (u16)stack_pop();
    emulator.emulator_cycles(1);
    u16 hi = stack_pop();
    emulator.emulator_cycles(1);

    u16 n = (hi << 8) | lo;
    cpu_context.registers.pc = n;

    emulator.emulator_cycles(1);
  }
}

void CPU::proc_reti()
{
  cpu_context.int_master_enabled = true;
  proc_ret();
}

void CPU::proc_pop()
{
  u16 lo = stack_pop();
  emulator.emulator_cycles(1);
  u16 hi = stack_pop();
  emulator.emulator_cycles(1);

  u16 n = (hi << 8) | lo;

  cpu_set_reg(cpu_context.cur_instruction->reg_1, n);

  if (cpu_context.cur_instruction->reg_1 == RT_AF) {
    cpu_set_reg(cpu_context.cur_instruction->reg_1, n & 0xFFF0);
  }
}

void CPU::proc_push()
{
  u16 hi = (cpu_read_reg(cpu_context.cur_instruction->reg_1) >> 8) & 0xFF;
  emulator.emulator_cycles(1);
  stack_push(hi);

  u16 lo = cpu_read_reg(cpu_context.cur_instruction->reg_1) & 0xFF;
  emulator.emulator_cycles(1);
  stack_push(lo);

  emulator.emulator_cycles(1);
}

void CPU::proc_inc()
{
  u16 val = cpu_read_reg(cpu_context.cur_instruction->reg_1) + 1;

  if (is_16_bit(cpu_context.cur_instruction->reg_1)) {
    emulator.emulator_cycles(1);
  }

  if (cpu_context.cur_instruction->reg_1 == RT_HL
      && cpu_context.cur_instruction->mode == AM_MR)
  {
    val = bus.bus_read(cpu_read_reg(RT_HL)) + 1;
    val &= 0xFF;
    bus.bus_write(cpu_read_reg(RT_HL), val);
  } else {
    cpu_set_reg(cpu_context.cur_instruction->reg_1, val);
    val = cpu_read_reg(cpu_context.cur_instruction->reg_1);
  }

  if ((cpu_context.cur_opcode & 0x03) == 0x03) {
    return;
  }

  cpu_set_flags(val == 0, 0, (val & 0x0F) == 0, -1);
}

void CPU::proc_dec()
{
  u16 val = cpu_read_reg(cpu_context.cur_instruction->reg_1) - 1;

  if (is_16_bit(cpu_context.cur_instruction->reg_1)) {
    emulator.emulator_cycles(1);
  }

  if (cpu_context.cur_instruction->reg_1 == RT_HL
      && cpu_context.cur_instruction->mode == AM_MR)
  {
    val = bus.bus_read(cpu_read_reg(RT_HL)) - 1;
    bus.bus_write(cpu_read_reg(RT_HL), val);
  } else {
    cpu_set_reg(cpu_context.cur_instruction->reg_1, val);
    val = cpu_read_reg(cpu_context.cur_instruction->reg_1);
  }

  if ((cpu_context.cur_opcode & 0x0B) == 0x0B) {
    return;
  }

  cpu_set_flags(val == 0, 1, (val & 0x0F) == 0x0F, -1);
}

void CPU::proc_sub()
{
  u16 val = cpu_read_reg(cpu_context.cur_instruction->reg_1)
      - cpu_context.fetched_data;

  int z = val == 0;
  int h = ((int)cpu_read_reg(cpu_context.cur_instruction->reg_1) & 0xF)
          - ((int)cpu_context.fetched_data & 0xF)
      < 0;
  int c = ((int)cpu_read_reg(cpu_context.cur_instruction->reg_1))
          - ((int)cpu_context.fetched_data)
      < 0;

  cpu_set_reg(cpu_context.cur_instruction->reg_1, val);
  cpu_set_flags(z, 1, h, c);
}

void CPU::proc_sbc()
{
  u8 val = cpu_context.fetched_data + CPU_FLAG_C(cpu_context);

  int z = cpu_read_reg(cpu_context.cur_instruction->reg_1) - val == 0;

  int h = ((int)cpu_read_reg(cpu_context.cur_instruction->reg_1) & 0xF)
          - ((int)cpu_context.fetched_data & 0xF)
          - ((int)CPU_FLAG_C(cpu_context))
      < 0;
  int c = ((int)cpu_read_reg(cpu_context.cur_instruction->reg_1))
          - ((int)cpu_context.fetched_data) - ((int)CPU_FLAG_C(cpu_context))
      < 0;

  cpu_set_reg(cpu_context.cur_instruction->reg_1,
              cpu_read_reg(cpu_context.cur_instruction->reg_1) - val);
  cpu_set_flags(z, 1, h, c);
}

void CPU::proc_adc()
{
  u16 u = cpu_context.fetched_data;
  u16 a = cpu_context.registers.a;
  u16 c = CPU_FLAG_C(cpu_context);

  cpu_context.registers.a = (a + u + c) & 0xFF;

  cpu_set_flags(cpu_context.registers.a == 0,
                0,
                ((a & 0xF) + (u & 0xF) + c) > 0xF,
                (a + u + c) > 0xFF);
}

void CPU::proc_add()
{
  u32 val = cpu_read_reg(cpu_context.cur_instruction->reg_1)
      + cpu_context.fetched_data;

  bool is_16bit = is_16_bit(cpu_context.cur_instruction->reg_1);

  if (is_16bit) {
    emulator.emulator_cycles(1);
  }

  if (cpu_context.cur_instruction->reg_1 == RT_SP) {
    val = cpu_read_reg(cpu_context.cur_instruction->reg_1)
        + (int8_t)cpu_context.fetched_data;
  }

  int z = (val & 0xFF) == 0;
  int h = (cpu_read_reg(cpu_context.cur_instruction->reg_1) & 0xF)
          + (cpu_context.fetched_data & 0xF)
      >= 0x10;
  int c = (int)(cpu_read_reg(cpu_context.cur_instruction->reg_1) & 0xFF)
          + (int)(cpu_context.fetched_data & 0xFF)
      >= 0x100;

  if (is_16bit) {
    z = -1;
    h = (cpu_read_reg(cpu_context.cur_instruction->reg_1) & 0xFFF)
            + (cpu_context.fetched_data & 0xFFF)
        >= 0x1000;
    u32 n = ((u32)cpu_read_reg(cpu_context.cur_instruction->reg_1))
        + ((u32)cpu_context.fetched_data);
    c = n >= 0x10000;
  }

  if (cpu_context.cur_instruction->reg_1 == RT_SP) {
    z = 0;
    h = (cpu_read_reg(cpu_context.cur_instruction->reg_1) & 0xF)
            + (cpu_context.fetched_data & 0xF)
        >= 0x10;
    c = (int)(cpu_read_reg(cpu_context.cur_instruction->reg_1) & 0xFF)
            + (int)(cpu_context.fetched_data & 0xFF)
        >= 0x100;
  }

  cpu_set_reg(cpu_context.cur_instruction->reg_1, val & 0xFFFF);
  cpu_set_flags(z, 0, h, c);
}

void CPU::debug_update()
{
  if (bus.bus_read(0xFF02) == 0x81) {
    char c = bus.bus_read(0xFF01);

    dbg_msg += c;

    bus.bus_write(0xFF02, 0);
  }
}

void CPU::debug_print()
{
  if (!dbg_msg.empty()) {
    printf("DBG: %s\n", dbg_msg.c_str());
  }
}
