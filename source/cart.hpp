#pragma once
#include <common.hpp>
#include <vector>

struct RomHeader
{
  u8 entry[4]; 
  u8 logo[0X30];

  char title[16]; 
  u16 new_lic_code; 
  u8 sgb_flag;
  u8 sgb_flag;
  u8 cart_type;
  u8 rom_size;
  u8 ram_size;
  u8 dest_code;
  u8 lic_code;
  u8 version;
  u8 checksum;
  u16 global_checksum;
};

struct CartridgeContext
{
  char filename[1024];
  u32 rom_size;
  u8* rom_data;
  RomHeader* header;
};

class Cartridge
{
public:
	bool cart_load(char* cart);
    char* cart_type_name();
    char* cart_lic_name();

private:
    CartridgeContext context;
};