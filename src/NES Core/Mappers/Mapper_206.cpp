#include "Mapper_206.h"

Mapper_206::Mapper_206(uint8_t PRGBanks, uint8_t CHRBanks) : Mapper(PRGBanks, CHRBanks)
{
	memset(ptr_PRG_bank_8KB, 0, sizeof(ptr_PRG_bank_8KB));
	memset(ptr_CHR_bank_1KB, 0, sizeof(ptr_CHR_bank_1KB));
	memset(ptr_CHR_bank_2KB, 0, sizeof(ptr_CHR_bank_2KB));
}

Mapper_206::~Mapper_206()
{
}

bool Mapper_206::cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data)
{
	if (addr >= 0x8000 && addr <= 0x9FFF)
	{
		mapped_addr = (ptr_PRG_bank_8KB[0] * 0x2000) + (addr & 0x1FFF);
		return true;
	}
	else if (addr >= 0xA000 && addr <= 0xBFFF)
	{
		mapped_addr = (ptr_PRG_bank_8KB[1] * 0x2000) + (addr & 0x1FFF);
		return true;
	}
	else if (addr >= 0xC000 && addr <= 0xFFFF)
	{
		mapped_addr = (ptr_PRG_bank_16KB * 0x4000) + (addr & 0x3FFF);
		return true;
	}


	return false;
}

bool Mapper_206::cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data)
{
	// Bank select (even address) | Bank data (odd address)
	if (addr >= 0x8000 && addr <= 0x9FFF)
	{
		if (addr % 2 == 0)
			bank_select = data & 0x07;

		else
		{
			switch (bank_select)
			{
			case 0: 
				ptr_CHR_bank_2KB[0] = (data >> 1) & 0x1F;
				break;

			case 1:
				ptr_CHR_bank_2KB[1] = (data >> 1) & 0x1F;
				break;

			case 2:
				ptr_CHR_bank_1KB[0] = data & 0x3F;
				break;

			case 3:
				ptr_CHR_bank_1KB[1] = data & 0x3F;
				break;

			case 4:
				ptr_CHR_bank_1KB[2] = data & 0x3F;
				break;

			case 5:
				ptr_CHR_bank_1KB[3] = data & 0x3F;
				break;

			case 6:
				ptr_PRG_bank_8KB[0] = data & 0x0F;
				break;

			case 7:
				ptr_PRG_bank_8KB[1] = data & 0x0F;
				break;
			}
		}
	}
	return false;
}

bool Mapper_206::ppuMapRead(uint16_t addr, uint32_t& mapped_addr)
{

	if (addr >= 0x0000 && addr <= 0x07FF)
	{
		mapped_addr = (ptr_CHR_bank_2KB[0] * 0x0800) + (addr & 0x07FF);
		return true;
	}
	else if (addr >= 0x0800 && addr <= 0x0FFF)
	{
		mapped_addr = (ptr_CHR_bank_2KB[1] * 0x0800) + (addr & 0x07FF);
		return true;
	}
	else if (addr >= 0x1000 && addr <= 0x13FF)
	{
		mapped_addr = (ptr_CHR_bank_1KB[0] * 0x0400) + (addr & 0x03FF);
		return true;
	}
	else if (addr >= 0x1400 && addr <= 0x17FF)
	{
		mapped_addr = (ptr_CHR_bank_1KB[1] * 0x0400) + (addr & 0x03FF);
		return true;
	}
	else if (addr >= 0x1800 && addr <= 0x1BFF)
	{
		mapped_addr = (ptr_CHR_bank_1KB[2] * 0x0400) + (addr & 0x03FF);
		return true;
	}
	else if (addr >= 0x1C00 && addr <= 0x1FFF)
	{
		mapped_addr = (ptr_CHR_bank_1KB[3] * 0x0400) + (addr & 0x03FF);
		return true;
	}
	return false;
}

bool Mapper_206::ppuMapWrite(uint16_t addr, uint32_t& mapped_addr)
{
	return false;
}

void Mapper_206::reset()
{
	bank_select = 0x00;

	memset(ptr_PRG_bank_8KB, 0, sizeof(ptr_PRG_bank_8KB));
	memset(ptr_CHR_bank_1KB, 0, sizeof(ptr_CHR_bank_1KB));
	memset(ptr_CHR_bank_2KB, 0, sizeof(ptr_CHR_bank_2KB));

	ptr_PRG_bank_16KB = number_PRG_banks - 1;
}

void Mapper_206::loadState(const std::vector<uint8_t>& dump)
{
	size_t index = 0;
	
	bank_select = dump[index++];
	ptr_PRG_bank_16KB = dump[index++];

	for (uint8_t i = 0; i < 2; i++)
		ptr_PRG_bank_8KB[i] = dump[index++];

	for (uint8_t i = 0; i < 4; i++)
		ptr_CHR_bank_1KB[i] = dump[index++];

	for (uint8_t i = 0; i < 2; i++)
		ptr_CHR_bank_2KB[i] = dump[index++];

	return;
}

std::vector<uint8_t>& Mapper_206::dumpState()
{
	static std::vector<uint8_t> dump;
	dump.clear();

	dump.push_back(bank_select);
	dump.push_back(ptr_PRG_bank_16KB);
	dump.insert(dump.end(), ptr_PRG_bank_8KB, ptr_PRG_bank_8KB + 2);
	dump.insert(dump.end(), ptr_CHR_bank_1KB, ptr_CHR_bank_1KB + 4);
	dump.insert(dump.end(), ptr_CHR_bank_2KB, ptr_CHR_bank_2KB + 2);
	return dump;
}
