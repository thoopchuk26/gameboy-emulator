#include <cpu.hpp>
#include <lcd.hpp>

void LCD::lcd_init()
{
  context.lcdc = 0x91;
  context.scroll_x = 0;
  context.scroll_y = 0;
  context.ly = 0;
  context.ly_compare = 0;
  context.bg_palette = 0xFC;
  context.obj_palette[0] = 0xFF;
  context.obj_palette[1] = 0xFF;
  context.win_y = 0;
  context.win_x = 0;

  for (int i = 0; i < 4; i++) {
    context.bg_colors[i] = colors_default[i];
    context.sp1_colors[i] = colors_default[i];
    context.sp2_colors[i] = colors_default[i];
  }
}

u8 LCD::lcd_read(u16 address)
{
  u8 offset = (address - 0xFF40);
  u8* p = static_cast<u8*>(static_cast<void*>(&context));

  return p[offset];
}

void LCD::lcd_write(u16 address, u8 value)
{
  u8 offset = (address - 0xFF40);
  u8* p = static_cast<u8*>(static_cast<void*>(&context));

  p[offset] = value;

  if (offset == 6) {
    // 0xFF46 = DMA
    cpu.dma.dma_start(value);
  }

  if (address == 0xFF47) {
    update_palette(value, 0);
  }
  if (address == 0xFF48) {
    update_palette(value & 0b11111100, 1);
  }
  if (address == 0xFF49) {
    update_palette(value & 0b11111100, 2);
  }
}

void LCD::update_palette(u8 palette_data, u8 palette)
{
  u32* p_colors = context.bg_colors;

  switch (palette) {
    case 1:
      p_colors = context.sp1_colors;
      break;
    case 2:
      p_colors = context.sp2_colors;
      break;
  }

  p_colors[0] = colors_default[palette_data & 0b11];
  p_colors[1] = colors_default[palette_data >> 2 & 0b11];
  p_colors[2] = colors_default[palette_data >> 4 & 0b11];
  p_colors[3] = colors_default[palette_data >> 6 & 0b11];
}
