#include "Mapper_011.h"

Mapper_011::Mapper_011(uint8_t PRGBanks, uint8_t CHRBanks) : Mapper(PRGBanks, CHRBanks)
{
}

Mapper_011::~Mapper_011()
{
}

bool Mapper_011::cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data)
{
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		mapped_addr = (ptr_PRG_bank * 0x8000) + (addr & 0x7FFF);
		return true;
	}
	return false;
}

bool Mapper_011::cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data)
{
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		ptr_PRG_bank = data & 0x03;
		ptr_CHR_bank = (data >> 4) & 0x0F;
	}
	return false;
}

bool Mapper_011::ppuMapRead(uint16_t addr, uint32_t& mapped_addr)
{

	if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		mapped_addr = (ptr_CHR_bank * 0x2000) + (addr & 0x1FFF);
		return true;
	}
	return false;
}

bool Mapper_011::ppuMapWrite(uint16_t addr, uint32_t& mapped_addr)
{
	return false;
}

void Mapper_011::reset()
{
	ptr_PRG_bank = 0;
	ptr_CHR_bank = 0;
}

void Mapper_011::loadState(const std::vector<uint8_t>& dump)
{
	size_t index = 0;

	ptr_PRG_bank = dump[index++];
	ptr_CHR_bank = dump[index++];
	return;
}

std::vector<uint8_t>& Mapper_011::dumpState()
{
	static std::vector<uint8_t> dump;
	dump.clear();

	dump.push_back(ptr_PRG_bank);
	dump.push_back(ptr_CHR_bank);
	return dump;
}
