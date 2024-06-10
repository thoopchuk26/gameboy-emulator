#include <emulator.hpp>
#include <lcd.hpp>
#include <ppu.hpp>

void PPU::ppu_init()
{
  context.current_frame = 0;
  context.line_ticks = 0;
  context.video_buffer.resize(YRES * XRES * sizeof(u32));

  emu.cpu.lcd.lcd_init();
  LCDS_MODE_SET(emu.cpu.lcd.context, MODE_OAM);
}

void PPU::ppu_tick()
{
  context.line_ticks++;

  switch (LCDS_MODE(emu.cpu.lcd.context)) {
    case MODE_OAM:
      ppu_mode_oam();
      break;
    case MODE_TRANSFER:
      ppu_mode_transfer();
      break;
    case MODE_VBLANK:
      ppu_mode_vblank();
      break;
    case MODE_HBLANK:
      ppu_mode_hblank();
      break;
  }
}

void PPU::ppu_oam_write(u16 address, u8 value)
{
  if (address >= 0xFE00) {
    address -= 0xFE00;
  }

  u8* p = static_cast<u8*>(static_cast<void*>(&context.vram));
  p[address] = value;
}

u8 PPU::ppu_oam_read(u16 address)
{
  if (address >= 0xFE00) {
    address -= 0xFE00;
  }

  u8* p = static_cast<u8*>(static_cast<void*>(&context.oam_ram));
  return p[address];
}

void PPU::ppu_vram_write(u16 address, u8 value)
{
  context.vram[address - 0x8000] = value;
}

u8 PPU::ppu_vram_read(u16 address)
{
  return context.vram[address - 0x8000];
}

void PPU::increment_ly()
{
  emu.cpu.lcd.context.ly++;

  if (emu.cpu.lcd.context.ly == emu.cpu.lcd.context.ly_compare) {
    LCDS_LYC_SET(emu.cpu.lcd.context, 1);

    if (LCDS_STAT_INT(emu.cpu.lcd.context, SS_LYC)) {
      emu.cpu.cpu_request_interrupts(IT_LCD_STAT);
    }
  } else {
    LCDS_LYC_SET(emu.cpu.lcd.context, 0);
  }
}

void PPU::ppu_mode_oam()
{
  if (context.line_ticks >= 80) {
    LCDS_MODE_SET(emu.cpu.lcd.context, MODE_TRANSFER);
  }
}

void PPU::ppu_mode_transfer()
{
  if (context.line_ticks >= 80 + 172) {
    LCDS_MODE_SET(emu.cpu.lcd.context, MODE_HBLANK);
  }
}

void PPU::ppu_mode_vblank()
{
  if (context.line_ticks >= TICKS_PER_LINE) {
    increment_ly();

    if (emu.cpu.lcd.context.ly >= LINES_PER_FRAME) {
      LCDS_MODE_SET(emu.cpu.lcd.context, MODE_OAM);
      emu.cpu.lcd.context.ly = 0;
    }

    context.line_ticks = 0;
  }
}

void PPU::ppu_mode_hblank()
{
  if (context.line_ticks >= TICKS_PER_LINE) {
    increment_ly();

    if (emu.cpu.lcd.context.ly >= YRES) {
      LCDS_MODE_SET(emu.cpu.lcd.context, MODE_VBLANK);

      emu.cpu.cpu_request_interrupts(IT_VBLANK);

      if (LCDS_STAT_INT(emu.cpu.lcd.context, SS_VBLANK)) {
        emu.cpu.cpu_request_interrupts(IT_LCD_STAT);
      }

      context.current_frame++;

      u32 end = emu.ui.get_ticks();
      u32 frame_time = end - frames.prev_frame_time;

      if (frame_time < target_frame_time) {
        emu.ui.delay((target_frame_time - frame_time));
      }

      if (end - frames.start_timer >= 1000) {
        u32 fps = frames.frame_count;
        frames.start_timer = end;
        frames.frame_count = 0;

        printf("FPS: %d\n", fps);
      }

      frames.frame_count++;
      frames.prev_frame_time = emu.ui.get_ticks();

    } else {
      LCDS_MODE_SET(emu.cpu.lcd.context, MODE_OAM);
    }

    context.line_ticks = 0;
  }
}
