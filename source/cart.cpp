#include <fstream>

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
  if (address < 0x4000 || !cart_mbc1()) {
    return context.rom_data[address];
  }

  if ((address & 0xE000) == 0xA000) {
    if (!context.ram_enabled) {
      return 0xFF;
    }
    if (context.ram_bank.empty()) {
      return 0xFF;
    }

    return context.ram_bank[address - 0xA000];
  }
  return context.rom_bank_x[address - 0x4000];
}

void Cartridge::cart_write(u16 address, u8 value)
{
  if (!cart_mbc1()) {
    return;
  }

  if (address < 0x2000) {
    context.ram_enabled = ((value & 0xF) == 0xA);
  }

  if ((address & 0xE000) == 0x2000) {
    if (value == 0) {
      value = 1;
    }

    value &= 0b11111;

    context.rom_bank_value = value;

    context.rom_bank_x.clear();
    context.rom_bank_x.insert(
        context.rom_bank_x.end(),
        context.rom_data.begin() + (0x4000 * context.rom_bank_value),
        context.rom_data.end());
  }

  if ((address & 0xE000) == 0x4000) {
    context.ram_bank_value = value & 0b11;

    if (context.ram_banking) {
      if (cart_need_save()) {
        cart_battery_save();
      }

      context.ram_bank = context.ram_banks[context.ram_bank_value];
    }
  }

  if ((address & 0xE000) == 0x6000) {
    context.banking_mode = value & 1;
    context.ram_banking = context.banking_mode;

    if (context.ram_banking) {
      if (cart_need_save()) {
        cart_battery_save();
      }

      context.ram_bank = context.ram_banks[context.ram_bank_value];
    }
  }

  if ((address & 0xE000) == 0xA000) {
    if (!context.ram_enabled) {
      return;
    }

    if (context.ram_bank.empty()) {
      return;
    }

    context.ram_bank[address - 0xA000] = value;

    if (context.battery) {
      context.need_save = true;
    }
  }
}

bool Cartridge::cart_need_save()
{
  return context.need_save;
}

bool Cartridge::cart_mbc1()
{
  return BETWEEN(context.header->cart_type, 1, 3);
}

bool Cartridge::cart_battery()
{
  return context.header->cart_type == 3;
}

void Cartridge::cart_set_banking()
{
  for (int i = 0; i < 16; i++) {
    context.ram_banks[i].clear();

    if ((context.header->ram_size == 2 && i == 0)
        || (context.header->ram_size == 3 && i < 4)
        || (context.header->ram_size == 4 && i < 16)
        || (context.header->ram_size == 5 && i < 8))
    {
      context.ram_banks[i].resize(0x2000);
    }
  }

  context.ram_bank = context.ram_banks[0];

  // MBC Changes the portion of rom data after 0x4000 to a bank so we change how
  // we hold the data
  context.rom_bank_x.insert(context.rom_bank_x.end(),
                            context.rom_data.begin() + 0x4000,
                            context.rom_data.end());

  // unsure if I should delete this data from rom_data
  // context.rom_data.resize(0x4000);
}

void Cartridge::cart_battery_load()
{
  if (context.ram_bank.empty()) {
    return;
  }

  // TODO: Make path relative
  std::string save_file =
      context.filename.substr(0, context.filename.size() - 3) + ".battery";
  std::size_t pos = save_file.find("roms");
  save_file.replace(pos, 4, "saves");

  std::fstream fp(save_file.c_str(), std::ios::binary | std::ios::in);

  if (!fp) {
    fprintf(stderr, "FAILED TO OPEN: %s\n", save_file.c_str());
    return;
  }

  context.ram_bank.clear();
  context.ram_bank.insert(context.ram_bank.begin(),
                          std::istreambuf_iterator<char>(fp),
                          std::istreambuf_iterator<char>());
  context.ram_bank.resize(0x2000);

  fp.close();
}

void Cartridge::cart_battery_save()
{
  if (context.ram_bank.empty()) {
    return;
  }

  // TODO: Make path relative
  std::string save_file =
      context.filename.substr(0, context.filename.size() - 3) + ".battery";
  std::size_t pos = save_file.find("roms");
  save_file.replace(pos, 4, "saves");

  std::ofstream fp(save_file.c_str(), std::ios::binary | std::ios::out);

  if (!fp) {
    fprintf(stderr, "FAILED TO OPEN: %s\n", save_file.c_str());
    return;
  }

  for (u8 val : context.ram_bank) {
    fp << val;
  }
  fp.close();
}

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
  context.battery = cart_battery();
  context.need_save = false;

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

  cart_set_banking();

  u16 x = 0;
  for (u16 i = 0x0134; i <= 0x014C; i++) {
    x = x - context.rom_data[i] - 1;
  }

  printf("\t Checksum : %2.2X (%s)\n",
         context.header->checksum,
         (x & 0xFF) ? "PASSED" : "FAILED");

  if (context.battery) {
    cart_battery_load();
  }

  return true;
}
