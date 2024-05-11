#include <memory>

#include <cpu.hpp>
#include <emulator.hpp>
#include <instructions.hpp>

void CPU::cpu_init()
{
  cpu_context.registers.pc = 0x100;
  cpu_context.registers.a = 0x01;
  cpu_context.halted = false;
}

void CPU::fetch_instruction()
{
  cpu_context.cur_opcode = memory_bus.bus_read(cpu_context.registers.pc);
  cpu_context.registers.pc++;
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
      break;

    case AM_R:
      cpu_context.fetched_data =
          cpu_read_reg(cpu_context.cur_instruction->reg_1);
      break;

    case AM_R_R:
      cpu_context.fetched_data =
          cpu_read_reg(cpu_context.cur_instruction->reg_2);
      break;

    case AM_R_D8:
      cpu_context.fetched_data = memory_bus.bus_read(cpu_context.registers.pc);

      cpu_context.registers.pc++;
      break;

    case AM_R_D16:
    case AM_D16: {
      u16 lo = memory_bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);

      u16 hi = memory_bus.bus_read(cpu_context.registers.pc + 1);
      emulator.emulator_cycles(1);

      cpu_context.fetched_data = lo | (hi << 8);

      cpu_context.registers.pc += 2;
    } break;

    case AM_MR_R:
      cpu_context.fetched_data =
          cpu_read_reg(cpu_context.cur_instruction->reg_2);
      cpu_context.mem_destination =
          cpu_read_reg(cpu_context.cur_instruction->reg_1);
      cpu_context.dest_is_mem = true;

      if (cpu_context.cur_instruction->reg_1 == RT_C) {
        cpu_context.mem_destination |= 0xFF00;
      }

      break;

    case AM_R_MR: {
      u16 address = cpu_read_reg(cpu_context.cur_instruction->reg_2);
      if (cpu_context.cur_instruction->reg_1 == RT_C) {
        cpu_context.mem_destination |= 0xFF00;
      }

      cpu_context.fetched_data = memory_bus.bus_read(address);
      emulator.emulator_cycles(1);
    } break;

    case AM_R_HLI:
      cpu_context.fetched_data =
          memory_bus.bus_read(cpu_read_reg(cpu_context.cur_instruction->reg_2));
      emulator.emulator_cycles(1);
      cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) + 1);
      break;

    case AM_R_HLD:
      cpu_context.fetched_data =
          memory_bus.bus_read(cpu_read_reg(cpu_context.cur_instruction->reg_2));
      emulator.emulator_cycles(1);
      cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) - 1);
      break;

    case AM_HLI_R:
      cpu_context.fetched_data =
          cpu_read_reg(cpu_context.cur_instruction->reg_2);
      cpu_context.mem_destination =
          cpu_read_reg(cpu_context.cur_instruction->reg_1);
      cpu_context.dest_is_mem = true;
      cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) + 1);
      break;

    case AM_HLD_R:
      cpu_context.fetched_data =
          cpu_read_reg(cpu_context.cur_instruction->reg_2);
      cpu_context.mem_destination =
          cpu_read_reg(cpu_context.cur_instruction->reg_1);
      cpu_context.dest_is_mem = true;
      cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) - 1);
      break;

    case AM_R_A8:
      cpu_context.fetched_data = memory_bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);
      cpu_context.registers.pc++;
      break;

    case AM_A8_R:
      cpu_context.mem_destination =
          memory_bus.bus_read(cpu_context.registers.pc) | 0xFF00;
      cpu_context.dest_is_mem = true;
      emulator.emulator_cycles(1);
      cpu_context.registers.pc++;
      break;

    case AM_HL_SPR:
      cpu_context.fetched_data = memory_bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);
      cpu_context.registers.pc++;
      break;

    case AM_D8:
      cpu_context.fetched_data = memory_bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);
      cpu_context.registers.pc++;
      break;

    case AM_A16_R:
    case AM_D16_R: {
      u16 lo = memory_bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);

      u16 hi = memory_bus.bus_read(cpu_context.registers.pc + 1);
      emulator.emulator_cycles(1);

      cpu_context.mem_destination = lo | (hi << 8);
      cpu_context.dest_is_mem = true;

      cpu_context.registers.pc += 2;
      cpu_context.fetched_data =
          cpu_read_reg(cpu_context.cur_instruction->reg_2);
    } break;

    case AM_MR_D8:
      cpu_context.fetched_data = memory_bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);
      cpu_context.registers.pc++;
      cpu_context.mem_destination =
          cpu_read_reg(cpu_context.cur_instruction->reg_1);
      cpu_context.dest_is_mem = true;
      break;

    case AM_MR:
      cpu_context.mem_destination =
          cpu_read_reg(cpu_context.cur_instruction->reg_1);
      cpu_context.dest_is_mem = true;
      cpu_context.fetched_data =
          memory_bus.bus_read(cpu_read_reg(cpu_context.cur_instruction->reg_1));
      emulator.emulator_cycles(1);
      break;

    case AM_R_A16: {
      u16 lo = memory_bus.bus_read(cpu_context.registers.pc);
      emulator.emulator_cycles(1);

      u16 hi = memory_bus.bus_read(cpu_context.registers.pc + 1);
      emulator.emulator_cycles(1);

      u16 address = lo | (hi << 8);

      cpu_context.registers.pc += 2;
      cpu_context.fetched_data = memory_bus.bus_read(address);
      emulator.emulator_cycles(1);
    } break;

    default:
      printf("Unknown Addressing Mode! %d (%02X)\n",
             cpu_context.cur_instruction->mode,
             cpu_context.cur_opcode);
      exit(-7);
      break;
  }
}

bool CPU::cpu_step()
{
  if (!cpu_context.halted) {
    u16 pc = cpu_context.registers.pc;

    fetch_instruction();
    fetch_data();

    printf("%04X: %-7s (%02X %02X %02X) A: %02X B: %02X C: %02X\n",
           pc,
           instruction_name(cpu_context.cur_instruction->type),
           cpu_context.cur_opcode,
           memory_bus.bus_read(pc + 1),
           memory_bus.bus_read(pc + 2),
           cpu_context.registers.a,
           cpu_context.registers.b,
           cpu_context.registers.c);

    if (cpu_context.cur_instruction == NULL) {
      printf("Unknown Instruction! %02X\n", cpu_context.cur_opcode);
      exit(-7);
    }

    execute();
  }

  return true;
}

void CPU::execute()
{
  printf("Executing Instruction: %02X   PC: %04X\n",
         cpu_context.cur_opcode,
         cpu_context.registers.pc);

  switch (cpu_context.cur_instruction->type) {
    case IN_NOP:
      proc_nop();
      break;
    case IN_LD:
      proc_ld();
      break;
    case IN_JP:
      proc_jp();
      break;
    case IN_DI:
      proc_di();
      break;
    case IN_XOR:
      proc_xor();
      break;
    case IN_NONE:
      proc_none();
      break;
    default:
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
      return memory_bus.bus_read(cpu_read_reg(RT_HL));
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
      memory_bus.bus_write(cpu_read_reg(RT_HL), val);
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

void CPU::proc_none()
{
  printf("INVALID INSTRUCTION!\n");
  exit(-7);
}

void CPU::proc_nop() {}

void CPU::proc_di()
{
  cpu_context.int_master_enabled = false;
}

void CPU::proc_ld()
{
  if (cpu_context.dest_is_mem) {
    // if it's a 16 bit register
    if (cpu_context.cur_instruction->reg_2 >= RT_AF) {
      emulator.emulator_cycles(1);
      memory_bus.bus_write16(cpu_context.mem_destination,
                             cpu_context.fetched_data);
    } else {
      memory_bus.bus_write(cpu_context.mem_destination,
                           cpu_context.fetched_data);
    }

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
                    + (char)cpu_context.fetched_data);

    return;
  }

  cpu_set_reg(cpu_context.cur_instruction->reg_1, cpu_context.fetched_data);
}

void CPU::proc_xor()
{
  cpu_context.registers.a ^= cpu_context.fetched_data & 0xFF;
  cpu_set_flags(cpu_context.registers.a == 0, 0, 0, 0);
}

bool CPU::check_cond()
{
  bool z = BIT(cpu_context.registers.f, 7);
  bool c = BIT(cpu_context.registers.f, 4);

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

void CPU::proc_jp()
{
  if (check_cond()) {
    cpu_context.registers.pc = cpu_context.fetched_data;
    emulator.emulator_cycles(1);
  }
}
