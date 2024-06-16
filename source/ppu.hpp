#pragma once

#include <queue>
#include <vector>

#include <common.hpp>

class Emulator;

static const int LINES_PER_FRAME = 154;
static const int TICKS_PER_LINE = 456;
static const int YRES = 144;
static const int XRES = 160;

enum FetchState
{
  FS_TILE,
  FS_DATA0,
  FS_DATA1,
  FS_IDLE,
  FS_PUSH
};

struct OAMEntry
{
  u8 y;
  u8 x;
  u8 tile;

  u8 f_cgb_pn : 3;
  u8 f_cgb_vram_bank : 1;
  u8 f_pn : 1;
  u8 f_x_flip : 1;
  u8 f_y_flip : 1;
  u8 f_bgp : 1;
};

struct OAMLineEntry
{
  OAMEntry entry;
  OAMLineEntry* next;
};

/*
 Bit7   BG and Window over OBJ (0=No, 1=BG and Window colors 1-3 over the OBJ)
 Bit6   Y flip          (0=Normal, 1=Vertically mirrored)
 Bit5   X flip          (0=Normal, 1=Horizontally mirrored)
 Bit4   Palette number  **Non CGB Mode Only** (0=OBP0, 1=OBP1)
 Bit3   Tile VRAM-Bank  **CGB Mode Only**     (0=Bank 0, 1=Bank 1)
 Bit2-0 Palette number  **CGB Mode Only**     (OBP0-7)
 */

struct PixelFifoContext
{
  FetchState cur_fetch_state;
  std::queue<u32> pixel_fifo;
  u8 line_x;
  u8 pushed_x;
  u8 fetch_x;
  u8 bgw_fetch_data[3];
  u8 fetch_entry_data[6];
  u8 map_y;
  u8 map_x;
  u8 tile_y;
  u8 fifo_x;
};

struct PPUContext
{
  OAMEntry oam_ram[40];
  u8 vram[0x2000];

  PixelFifoContext pfc;

  u8 line_sprite_count;
  OAMLineEntry* line_sprites;  // linked list of sprites
  OAMLineEntry line_entry_array[10];  // memory to use for list.

  u8 fetched_entry_count;
  OAMEntry fetched_entries[3];
  u8 window_line;

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

  // ppu data functions
  void ppu_oam_write(u16 address, u8 value);
  u8 ppu_oam_read(u16 address);
  void ppu_vram_write(u16 address, u8 value);
  u8 ppu_vram_read(u16 address);

  // ppu state functions
  void increment_ly();
  void ppu_mode_oam();
  void ppu_mode_transfer();
  void ppu_mode_vblank();
  void ppu_mode_hblank();

  // pipeline functions
  void pipeline_process();
  void pipeline_fetch();
  void pipeline_fifo_reset();
  void pixel_fifo_push(u32 value);
  u32 pixel_fifo_pop();
  void pipeline_push_pixel();
  bool pipeline_fifo_add();
  void pipeline_load_window_tile();

  // sprite data functions
  void load_line_sprites();
  u32 fetch_sprite_pixels(int bit, u32 color, u8 bg_color);
  void pipeline_load_sprite_tile();
  void pipeline_load_sprite_data(u8 offset);

  bool window_visible();

private:
  Emulator& emu;
};
