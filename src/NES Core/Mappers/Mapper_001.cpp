#include "Mapper_001.h"

Mapper_001::Mapper_001(uint8_t PRGBanks, uint8_t CHRBanks) : Mapper(PRGBanks, CHRBanks)
{
	mapper_RAM.resize(RAM_size);
}

Mapper_001::~Mapper_001()
{
}

bool Mapper_001::cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data)
{
	if (addr < 0x6000) return false;

	if (addr < 0x8000)
	{
		mapped_addr = 0xFFFFFFFF;
		data = mapper_RAM[addr & 0x1FFF];
		return true;
	}

	if (PRG_ROM_bank_mode < 2)
	{
		mapped_addr = (ptr_PRG_bank_32KB * 0x8000) + (addr & 0x7FFF);
		return true;
	}
	else
	{
		uint8_t bank = (addr >> 14) & 0x01;
		mapped_addr = (ptr_PRG_bank_16KB[bank] * 0x4000) + (addr & 0x3FFF);
		return true;
	}

	return false;
}

bool Mapper_001::cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data)
{
	if (addr < 0x6000) return false;
	if (addr < 0x8000)
	{
		mapped_addr = 0xFFFFFFFF;
		mapper_RAM[addr & 0x1FFF] = data;
		return true;
	}
	
	// If bit 7 is set clear load shift register
	if (!(data & 0x80))
	{
		load = (load >> 1) | ((data & 0x01) << 4);
		load_writes++;

		if (load_writes == 5)
		{
			// Write data to register
			switch ((addr >> 13) & 0x03)
			{
			// Control Register
			case 0:
				control = load & 0x1F;
				CHR_ROM_bank_mode = (control >> 4) & 0x01;
				PRG_ROM_bank_mode = (control >> 2) & 0x03;

				// Set mirror mode
				switch (control & 0x03)
				{
				case 0: mirror = ONESCREEN_LOW; break;
				case 1: mirror = ONESCREEN_HIGH; break;
				case 2: mirror = VERTICAL; break;
				case 3: mirror = HORIZONTAL; break;
				}

				break;

			// CHR bank 0 Register
			case 1:
				CHR_bank_0 = load & 0x1F;

				if (CHR_ROM_bank_mode == 0)
					ptr_CHR_bank_8KB = CHR_bank_0 & 0x1E;
				else
					ptr_CHR_bank_4KB[0] = CHR_bank_0;

				break;

			// CHR bank 1 Register
			case 2:
				CHR_bank_1 = load & 0x1F;

				if (CHR_ROM_bank_mode == 1)
					ptr_CHR_bank_4KB[1] = CHR_bank_1;

				break;

			// PRG bank Register 
			case 3:
				PRG_bank = load & 0x1F;

				switch (PRG_ROM_bank_mode)
				{
				case 0:
				case 1: 
					ptr_PRG_bank_32KB = (PRG_bank & 0x0E) >> 1; 
					break;
				case 2:
					ptr_PRG_bank_16KB[0] = 0;
					ptr_PRG_bank_16KB[1] = PRG_bank & 0x0F;
					break;
				case 3:
					ptr_PRG_bank_16KB[0] = PRG_bank & 0x0F;
					ptr_PRG_bank_16KB[1] = number_PRG_banks - 1;
					break;
				}
				break;
			}

			// Reset Load Register and counter
			load = 0x00;
			load_writes = 0;
		}
	}
	else
	{
		load = 0x00;
		load_writes = 0;
		control |= 0x0C;
	}

	return false;
}

bool Mapper_001::ppuMapRead(uint16_t addr, uint32_t& mapped_addr)
{
	if (addr > 0x1FFF) return false;

	if (CHR_ROM_bank_mode == 0)
	{
		mapped_addr = (ptr_CHR_bank_8KB * 0x2000) + (addr & 0x1FFF);
		return true;
	}
	else
	{
		uint8_t bank = (addr >> 12) & 0x01;
		mapped_addr = (ptr_CHR_bank_4KB[bank] * 0x1000) + (addr & 0x0FFF);
		return true;
	}

	return false;
}

bool Mapper_001::ppuMapWrite(uint16_t addr, uint32_t& mapped_addr)
{
	if (addr > 0x1FFF) return false;

	if (number_CHR_banks == 0)
	{
		mapped_addr = addr;
		return true;
	}

	return false;
}

uint8_t Mapper_001::getMirrorMode()
{
	return mirror;
}

void Mapper_001::loadRAM(const std::string& path)
{
	std::ifstream ifs(path, std::ios::binary);
	if (ifs.is_open())
	{
		ifs.read(reinterpret_cast<char*>(mapper_RAM.data()), mapper_RAM.size());
		ifs.close();
	}
}

void Mapper_001::dumpRAM(const std::string& path)
{
	std::ofstream ofs(path);
	if (ofs.is_open())
	{
		ofs.write(reinterpret_cast<char*>(mapper_RAM.data()), mapper_RAM.size());
		ofs.close();
	}
}

void Mapper_001::loadState(const std::vector<uint8_t>& dump)
{
	size_t index = 0;

	mirror = dump[index++];
	load = dump[index++];
	control = dump[index++];
	CHR_bank_0 = dump[index++];
	CHR_bank_1 = dump[index++];
	PRG_bank = dump[index++];
	load_writes = dump[index++];
	PRG_ROM_bank_mode = dump[index++];
	CHR_ROM_bank_mode = dump[index++];
	ptr_PRG_bank_32KB = dump[index++];
	ptr_PRG_bank_16KB[0] = dump[index++];
	ptr_PRG_bank_16KB[1] = dump[index++];
	ptr_CHR_bank_8KB = dump[index++];
	ptr_CHR_bank_4KB[0] = dump[index++];
	ptr_CHR_bank_4KB[1] = dump[index++];
	for (size_t i = 0, size = mapper_RAM.size(); i < size; i++)
		mapper_RAM[i] = dump[index++];
	return;
}

std::vector<uint8_t>& Mapper_001::dumpState()
{
	static std::vector<uint8_t> dump;
	dump.clear();

	dump.push_back(mirror);
	dump.push_back(load);
	dump.push_back(control);
	dump.push_back(CHR_bank_0);
	dump.push_back(CHR_bank_1);
	dump.push_back(PRG_bank);
	dump.push_back(load_writes);
	dump.push_back(PRG_ROM_bank_mode);
	dump.push_back(CHR_ROM_bank_mode);
	dump.push_back(ptr_PRG_bank_32KB);
	dump.push_back(ptr_PRG_bank_16KB[0]);
	dump.push_back(ptr_PRG_bank_16KB[1]);
	dump.push_back(ptr_CHR_bank_8KB);
	dump.push_back(ptr_CHR_bank_4KB[0]);
	dump.push_back(ptr_CHR_bank_4KB[1]);
	dump.insert(dump.end(), mapper_RAM.begin(), mapper_RAM.end());
	return dump;
}

void Mapper_001::reset()
{
	mirror = 0;
	load = 0x00;
	control = 0x1C;
	CHR_bank_0 = 0x00;
	CHR_bank_1 = 0x00;
	PRG_bank = 0x00;
	load_writes = 0;


	ptr_PRG_bank_32KB = 0x00;
	ptr_CHR_bank_8KB = 0x00;

	ptr_PRG_bank_16KB[0] = 0x00;
	ptr_PRG_bank_16KB[1] = number_PRG_banks - 1;

	ptr_CHR_bank_4KB[0] = 0x00;
	ptr_CHR_bank_4KB[1] = 0x00;

	PRG_ROM_bank_mode = 0x03;
	CHR_ROM_bank_mode = 0x00;
}