#include "Mapper_066.h"


Mapper_066::Mapper_066(uint8_t PRGBanks, uint8_t CHRBanks) : Mapper(PRGBanks, CHRBanks)
{
}

Mapper_066::~Mapper_066()
{
}

bool Mapper_066::cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data)
{
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		mapped_addr = (ptr_PRG_bank * 0x8000) + (addr & 0x7FFF);
		return true;
	}

	return false;
}

bool Mapper_066::cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data)
{
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		ptr_PRG_bank = (data >> 4) & 0x03;
		ptr_CHR_bank = data & 0x03;
	}

	return false;
}

bool Mapper_066::ppuMapRead(uint16_t addr, uint32_t& mapped_addr)
{
	if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		mapped_addr = (ptr_CHR_bank * 0x2000) + addr;
		return true;
	}

	return false;
}

bool Mapper_066::ppuMapWrite(uint16_t addr, uint32_t& mapped_addr)
{
	return false;
}

void Mapper_066::reset()
{
	ptr_PRG_bank = 0x00;
	ptr_CHR_bank = 0x00;
}

void Mapper_066::loadState(const std::vector<uint8_t>& dump)
{
	size_t index = 0;

	ptr_PRG_bank = dump[index++];
	ptr_CHR_bank = dump[index++];
	return;
}

std::vector<uint8_t>& Mapper_066::dumpState()
{
	static std::vector<uint8_t> dump;
	dump.clear();

	dump.push_back(ptr_PRG_bank);
	dump.push_back(ptr_CHR_bank);
	return dump;
}
