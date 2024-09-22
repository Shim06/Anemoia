#include "Mapper_000.h"


Mapper_000::Mapper_000(uint8_t PRGBanks, uint8_t CHRBanks) : Mapper(PRGBanks, CHRBanks)
{
}

Mapper_000::~Mapper_000()
{
}

bool Mapper_000::cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data)
{
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		mapped_addr = addr & (number_PRG_banks > 1 ? 0x7FFF : 0x3FFF);
		return true;
	}

	return false;
}

bool Mapper_000::cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data)
{
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		mapped_addr = addr & (number_PRG_banks > 1 ? 0x7FFF : 0x3FFF);
	}

	return false;
}

bool Mapper_000::ppuMapRead(uint16_t addr, uint32_t& mapped_addr)
{
	if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		mapped_addr = addr;
		return true;
	}

	return false;
}

bool Mapper_000::ppuMapWrite(uint16_t addr, uint32_t& mapped_addr)
{
	if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		if (number_CHR_banks == 0)
		{
			// Treat as RAM
			mapped_addr = addr;
			return true;
		}
	}

	return false;
}
