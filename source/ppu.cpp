#include "ppu.hpp"

#include <emulator.hpp>
#include <lcd.hpp>
#include <ppu.hpp>

#include "ppu.hpp"

void PPU::ppu_init()
{
  context.video_buffer.resize(YRES * XRES * sizeof(u32));

  context.pfc.cur_fetch_state = FS_TILE;

  emu.cpu.lcd.lcd_init();
  LCDS_MODE_SET(emu.cpu.lcd.context, MODE_OAM);

  memset(context.oam_ram, 0, sizeof(context.oam_ram));
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

  u8* p = static_cast<u8*>(static_cast<void*>(&context.oam_ram));
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
  if (window_visible() && emu.cpu.lcd.context.ly >= emu.cpu.lcd.context.win_y
      && emu.cpu.lcd.context.ly < emu.cpu.lcd.context.win_y + YRES)
  {
    context.window_line++;
  }
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

    context.pfc.cur_fetch_state = FS_TILE;
    context.pfc.line_x = 0;
    context.pfc.fetch_x = 0;
    context.pfc.pushed_x = 0;
    context.pfc.fifo_x = 0;
  }

  if (context.line_ticks == 1) {
    context.line_sprites = 0;
    context.line_sprite_count = 0;

    load_line_sprites();
  }
}

void PPU::ppu_mode_transfer()
{
  pipeline_process();
  if (context.pfc.pushed_x >= XRES) {
    pipeline_fifo_reset();
    LCDS_MODE_SET(emu.cpu.lcd.context, MODE_HBLANK);

    if (LCDS_STAT_INT(emu.cpu.lcd.context, SS_HBLANK)) {
      emu.cpu.cpu_request_interrupts(IT_LCD_STAT);
    }
  }
}

void PPU::ppu_mode_vblank()
{
  if (context.line_ticks >= TICKS_PER_LINE) {
    increment_ly();

    if (emu.cpu.lcd.context.ly >= LINES_PER_FRAME) {
      LCDS_MODE_SET(emu.cpu.lcd.context, MODE_OAM);
      emu.cpu.lcd.context.ly = 0;
      context.window_line = 0;
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

void PPU::pipeline_process()
{
  context.pfc.map_y = (emu.cpu.lcd.context.ly + emu.cpu.lcd.context.scroll_y);
  context.pfc.map_x = (context.pfc.fetch_x + emu.cpu.lcd.context.scroll_x);
  context.pfc.tile_y =
      ((emu.cpu.lcd.context.ly + emu.cpu.lcd.context.scroll_y) % 8) * 2;

  if (!(context.line_ticks & 1)) {
    pipeline_fetch();
  }

  pipeline_push_pixel();
}

void PPU::pipeline_fifo_reset()
{
  while (context.pfc.pixel_fifo.size()) {
    pixel_fifo_pop();
  }
}

void PPU::pixel_fifo_push(u32 value)
{
  context.pfc.pixel_fifo.push(value);
}

void PPU::pipeline_fetch()
{
  switch (context.pfc.cur_fetch_state) {
    case FS_TILE: {
      context.fetched_entry_count = 0;
      if (LCDC_BGW_ENABLE(emu.cpu.lcd.context)) {
        context.pfc.bgw_fetch_data[0] = emu.cpu.bus.bus_read(
            LCDC_BG_MAP_AREA(emu.cpu.lcd.context) + (context.pfc.map_x / 8)
            + ((context.pfc.map_y / 8) * 32));

        if (LCDC_BGW_DATA_AREA(emu.cpu.lcd.context) == 0x8800) {
          context.pfc.bgw_fetch_data[0] += 128;
        }

        pipeline_load_window_tile();
      }

      if (LCDC_OBJ_ENABLE(emu.cpu.lcd.context) && context.line_sprites) {
        pipeline_load_sprite_tile();
      }

      context.pfc.cur_fetch_state = FS_DATA0;
      context.pfc.fetch_x += 8;
    } break;
    case FS_DATA0: {
      context.pfc.bgw_fetch_data[1] = emu.cpu.bus.bus_read(
          LCDC_BGW_DATA_AREA(emu.cpu.lcd.context)
          + (context.pfc.bgw_fetch_data[0] * 16) + context.pfc.tile_y);

      pipeline_load_sprite_data(0);

      context.pfc.cur_fetch_state = FS_DATA1;
    } break;
    case FS_DATA1: {
      context.pfc.bgw_fetch_data[2] = emu.cpu.bus.bus_read(
          LCDC_BGW_DATA_AREA(emu.cpu.lcd.context)
          + (context.pfc.bgw_fetch_data[0] * 16) + context.pfc.tile_y + 1);

      pipeline_load_sprite_data(1);

      context.pfc.cur_fetch_state = FS_IDLE;
    } break;
    case FS_IDLE: {
      context.pfc.cur_fetch_state = FS_PUSH;
    } break;
    case FS_PUSH: {
      if (pipeline_fifo_add()) {
        context.pfc.cur_fetch_state = FS_TILE;
      }
    } break;
  }
}

void PPU::pipeline_push_pixel()
{
  if (context.pfc.pixel_fifo.size() > 8) {
    u32 pixel_data = pixel_fifo_pop();

    if (context.pfc.line_x >= (emu.cpu.lcd.context.scroll_x % 8)) {
      context.video_buffer[context.pfc.pushed_x
                           + (emu.cpu.lcd.context.ly * XRES)] = pixel_data;

      context.pfc.pushed_x++;
    }

    context.pfc.line_x++;
  }
}

bool PPU::pipeline_fifo_add()
{
  if (context.pfc.pixel_fifo.size() > 8) {
    return false;
  }

  int x = context.pfc.fetch_x - (8 - (emu.cpu.lcd.context.scroll_x % 8));

  for (int i = 0; i < 8; i++) {
    int bit = 7 - i;
    u8 lo = !!(context.pfc.bgw_fetch_data[1] & (1 << bit));
    u8 hi = !!(context.pfc.bgw_fetch_data[2] & (1 << bit)) << 1;

    u32 color = emu.cpu.lcd.context.bg_colors[hi | lo];

    if (!LCDC_BGW_ENABLE(emu.cpu.lcd.context)) {
      color = emu.cpu.lcd.context.bg_colors[0];
    }

    if (LCDC_OBJ_ENABLE(emu.cpu.lcd.context)) {
      color = fetch_sprite_pixels(bit, color, (hi | lo));
    }

    if (x >= 0) {
      pixel_fifo_push(color);
      context.pfc.fifo_x++;
    }
  }

  return true;
}

void PPU::pipeline_load_window_tile()
{
  if (!window_visible()) {
    return;
  }

  u8 window_y = emu.cpu.lcd.context.win_y;

  if (context.pfc.fetch_x + 7 >= emu.cpu.lcd.context.win_x
      && context.pfc.fetch_x + 7 < emu.cpu.lcd.context.win_x + YRES + 14)
  {
    if (emu.cpu.lcd.context.ly >= window_y
        && emu.cpu.lcd.context.ly < window_y + XRES)
    {
      u8 w_tile_y = context.window_line / 8;

      context.pfc.bgw_fetch_data[0] = emu.cpu.bus.bus_read(
          LCDC_WIN_MAP_AREA(emu.cpu.lcd.context)
          + ((context.pfc.fetch_x + 7 - emu.cpu.lcd.context.win_x) / 8)
          + (w_tile_y * 32));

      if (LCDC_BGW_DATA_AREA(emu.cpu.lcd.context) == 0x8800) {
        context.pfc.bgw_fetch_data[0] += 128;
      }
    }
  }
}

void PPU::load_line_sprites()
{
  int cur_y = emu.cpu.lcd.context.ly;

  u8 sprite_height = LCDC_OBJ_HEIGHT(emu.cpu.lcd.context);
  memset(context.line_entry_array, 0, sizeof(context.line_entry_array));

  for (int i = 0; i < 40; i++) {
    OAMEntry e = context.oam_ram[i];

    if (!e.x) {
      // x = 0 means not visible...
      continue;
    }

    if (context.line_sprite_count >= 10) {
      // max 10 sprites per line...
      break;
    }

    if (e.y <= cur_y + 16 && e.y + sprite_height > cur_y + 16) {
      // this sprite is on the current line.

      OAMLineEntry* entry =
          &context.line_entry_array[context.line_sprite_count++];

      entry->entry = e;
      entry->next = NULL;

      if (!context.line_sprites || context.line_sprites->entry.x > e.x) {
        entry->next = context.line_sprites;
        context.line_sprites = entry;
        continue;
      }

      // do some sorting...

      OAMLineEntry* le = context.line_sprites;
      OAMLineEntry* prev = le;

      while (le) {
        if (le->entry.x > e.x) {
          prev->next = entry;
          entry->next = le;
          break;
        }

        if (!le->next) {
          le->next = entry;
          break;
        }

        prev = le;
        le = le->next;
      }
    }
  }
}

u32 PPU::fetch_sprite_pixels(int bit, u32 color, u8 bg_color)
{
  for (int i = 0; i < context.fetched_entry_count; i++) {
    int sprite_x =
        (context.fetched_entries[i].x - 8) + (emu.cpu.lcd.context.scroll_x % 8);

    if (sprite_x + 8 < context.pfc.fifo_x) {
      // Past pixel location already
      continue;
    }

    int offset = context.pfc.fifo_x - sprite_x;

    if (offset < 0 || offset > 7) {
      // Out of bounds
      continue;
    }

    bit = (7 - offset);

    if (context.fetched_entries[i].f_x_flip) {
      bit = offset;
    }

    u8 lo = !!(context.pfc.fetch_entry_data[i * 2] & (1 << bit));
    u8 hi = !!(context.pfc.fetch_entry_data[(i * 2) + 1] & (1 << bit)) << 1;

    bool bg_priority = context.fetched_entries[i].f_bgp;

    if (!(hi | lo)) {
      // transparent pixel
      continue;
    }

    if (!bg_priority || bg_color == 0) {
      color = (context.fetched_entries[i].f_pn)
          ? emu.cpu.lcd.context.sp2_colors[hi | lo]
          : emu.cpu.lcd.context.sp1_colors[hi | lo];

      if (hi | lo) {
        break;
      }
    }
  }
  return color;
}

void PPU::pipeline_load_sprite_tile()
{
  OAMLineEntry* line_entry = context.line_sprites;

  while (line_entry) {
    int sprite_x =
        (line_entry->entry.x - 8) + (emu.cpu.lcd.context.scroll_x % 8);

    if ((sprite_x >= context.pfc.fetch_x && sprite_x < context.pfc.fetch_x + 8)
        || ((sprite_x + 8) >= context.pfc.fetch_x
            && (sprite_x + 8) < context.pfc.fetch_x + 8))
    {
      context.fetched_entries[context.fetched_entry_count++] =
          line_entry->entry;
    }

    line_entry = line_entry->next;

    if (!line_entry || context.fetched_entry_count >= 3) {
      // Max 3 sprites on pixels
      break;
    }
  }
}

void PPU::pipeline_load_sprite_data(u8 offset)
{
  u8 cur_y = emu.cpu.lcd.context.ly;
  u8 sprite_height = LCDC_OBJ_HEIGHT(emu.cpu.lcd.context);

  for (int i = 0; i < context.fetched_entry_count; i++) {
    u8 ty = ((cur_y + 16) - context.fetched_entries[i].y) * 2;

    if (context.fetched_entries[i].f_y_flip) {
      ty = ((sprite_height * 2) - 2) - ty;
    }

    u8 tile_index = context.fetched_entries[i].tile;

    if (sprite_height == 16) {
      tile_index &= ~(1);  // removes last bit
    }

    context.pfc.fetch_entry_data[(i * 2) + offset] =
        emu.cpu.bus.bus_read(0x8000 + (tile_index * 16) + ty + offset);
  }
}

bool PPU::window_visible()
{
  return LCDC_WIN_ENABLE(emu.cpu.lcd.context) && emu.cpu.lcd.context.win_x >= 0
      && emu.cpu.lcd.context.win_x <= 166 && emu.cpu.lcd.context.win_y >= 0
      && emu.cpu.lcd.context.win_y < YRES;
}

u32 PPU::pixel_fifo_pop()
{
  if (context.pfc.pixel_fifo.size() <= 0) {
    fprintf(stderr, "ERR IN PIXEL FIFO!\n");
    exit(-8);
  }

  u32 val = context.pfc.pixel_fifo.front();
  context.pfc.pixel_fifo.pop();

  return val;
}
