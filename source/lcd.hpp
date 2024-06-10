#pragma once

#include <common.hpp>

class CPU;

#define LCDC_BGW_ENABLE(context) (BIT(context.lcdc, 0))
#define LCDC_OBJ_ENABLE(context) (BIT(context.lcdc, 1))
#define LCDC_OBJ_HEIGHT(context) (BIT(context.lcdc, 2) ? 16 : 8)
#define LCDC_BG_MAP_AREA(context) (BIT(context.lcdc, 3) ? 0x9C00 : 0x9800)
#define LCDC_BGW_DATA_AREA(context) (BIT(context.lcdc, 4) ? 0x8000 : 0x8800)
#define LCDC_WIN_ENABLE(context) (BIT(context.lcdc, 5))
#define LCDC_WIN_MAP_AREA(context) (BIT(context.lcdc, 6) ? 0x9C00 : 0x9800)
#define LCDC_LCD_ENABLE_BIT(context) (BIT(context.lcdc, 7))

#define LCDS_MODE(context) ((LCDMode)(context.lcds & 0b11))
#define LCDS_MODE_SET(context, mode) \
  { \
    context.lcds &= ~0b11; \
    context.lcds |= mode; \
  }

#define LCDS_LYC(context) (BIT(context.lcds, 2))
#define LCDS_LYC_SET(context, b) (BIT_SET(context.lcds, 2, b))
#define LCDS_STAT_INT(context, src) (context.lcds & src)

static unsigned long colors_default[4] = {
    0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};

struct LCDContext
{
  u8 lcdc;
  u8 lcds;
  u8 scroll_y;
  u8 scroll_x;
  u8 ly;
  u8 ly_compare;
  u8 dma;
  u8 bg_palette;
  u8 obj_palette[2];
  u8 win_y;
  u8 win_x;

  // other data
  u32 bg_colors[4];
  u32 sp1_colors[4];
  u32 sp2_colors[4];
};

enum LCDMode
{
  MODE_HBLANK,
  MODE_VBLANK,
  MODE_OAM,
  MODE_TRANSFER
};

enum StatSrc
{
  SS_HBLANK = (1 << 3),
  SS_VBLANK = (1 << 4),
  SS_OAM = (1 << 5),
  SS_LYC = (1 << 6),
};

class LCD
{
public:
  LCD(CPU& c)
      : cpu(c) {};

  LCDContext context;

  void lcd_init();
  u8 lcd_read(u16 address);
  void lcd_write(u16 address, u8 value);
  void update_palette(u8 palette_data, u8 palette);

private:
  CPU& cpu;
};
