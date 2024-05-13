#include <fstream>
#include <string>
#include <vector>

#include <cart.hpp>

const char* Cartridge::cart_lic_name()
{
  if (context.header->new_lic_code <= 0xA4
      && LIC_CODE.find(context.header->lic_code) != LIC_CODE.end())
  {
    return LIC_CODE[context.header->lic_code];
  }

  return "UNKNOWN";
}

u8 Cartridge::cart_read(u16 address)
{
  return context.rom_data[address];
}

void Cartridge::cart_write(u16 address, u8 value) {}

const char* Cartridge::cart_type_name()
{
  if (context.header->cart_type <= 0x22) {
    return ROM_TYPES[context.header->cart_type];
  }

  return "UNKNOWN";
}

bool Cartridge::cart_load(std::string cart)
{
  context.filename = cart.c_str();

  std::ifstream fp(cart.c_str(), std::ios::binary);

  if (!fp) {
    printf("Failed to open: %s\n", cart.c_str());

    return false;
  }

  printf("Opened: %s\n", context.filename.c_str());

  context.rom_data.insert(context.rom_data.begin(),
                          std::istreambuf_iterator<char>(fp),
                          std::istreambuf_iterator<char>());

  fp.close();

  context.rom_size = context.rom_data.size();

  context.header = (RomHeader*)(context.rom_data.data() + 0x100);
  context.header->title[15] = 0;

  printf("Cartridge Loaded:\n");
  printf("\t Title    : %s\n", context.header->title);
  printf("\t Type     : %2.2X (%s)\n",
         context.header->cart_type,
         cart_type_name());
  printf("\t ROM Size : %d KB\n", 32 << context.header->rom_size);
  printf("\t RAM Size : %2.2X\n", context.header->ram_size);
  printf(
      "\t LIC Code : %2.2X (%s)\n", context.header->lic_code, cart_lic_name());
  printf("\t ROM Vers : %2.2X\n", context.header->version);

  u16 x = 0;
  for (u16 i = 0x0134; i <= 0x014C; i++) {
    x = x - context.rom_data[i] - 1;
  }

  printf("\t Checksum : %2.2X (%s)\n",
         context.header->checksum,
         (x & 0xFF) ? "PASSED" : "FAILED");

  return true;
}
