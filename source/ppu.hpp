#pragma once

#include <vector>

#include <common.hpp>

class Emulator;

static const int LINES_PER_FRAME = 154;
static const int TICKS_PER_LINE = 456;
static const int YRES = 144;
static const int XRES = 160;

struct OAMEntry
{
  u8 x;
  u8 y;
  u8 tile;
  u8 flags;

  u8 f_cgb_pn = 3;
  u8 f_cgb_vram_bank = 1;
  u8 f_pn = 1;
  u8 f_x_flip = 1;
  u8 f_y_flip = 1;
  u8 f_bgp = 1;
};

/*
 Bit7   BG and Window over OBJ (0=No, 1=BG and Window colors 1-3 over the OBJ)
 Bit6   Y flip          (0=Normal, 1=Vertically mirrored)
 Bit5   X flip          (0=Normal, 1=Horizontally mirrored)
 Bit4   Palette number  **Non CGB Mode Only** (0=OBP0, 1=OBP1)
 Bit3   Tile VRAM-Bank  **CGB Mode Only**     (0=Bank 0, 1=Bank 1)
 Bit2-0 Palette number  **CGB Mode Only**     (OBP0-7)
 */

struct PPUContext
{
  std::array<OAMEntry, 40> oam_ram;
  std::array<u8, 0x2000> vram;

  u32 current_frame;
  u32 line_ticks;
  std::vector<u32> video_buffer;
};

struct FrameData
{
  long prev_frame_time;
  long start_timer;
  long frame_count;
};

static const u32 target_frame_time = 1000 / 60;

class PPU
{
public:
  PPU(Emulator& e)
      : emu(e) {};

  FrameData frames;
  PPUContext context;

  void ppu_init();
  void ppu_tick();

  void ppu_oam_write(u16 address, u8 value);
  u8 ppu_oam_read(u16 address);

  void ppu_vram_write(u16 address, u8 value);
  u8 ppu_vram_read(u16 address);

  void increment_ly();
  void ppu_mode_oam();
  void ppu_mode_transfer();
  void ppu_mode_vblank();
  void ppu_mode_hblank();

private:
  Emulator& emu;
};
