#include "Mapper_003.h"


Mapper_003::Mapper_003(uint8_t PRGBanks, uint8_t CHRBanks) : Mapper(PRGBanks, CHRBanks)
{
}

Mapper_003::~Mapper_003()
{
}

bool Mapper_003::cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data)
{
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		mapped_addr = addr & (number_PRG_banks > 1 ? 0x7FFF : 0x3FFF);
		return true;
	}

	return false;
}

bool Mapper_003::cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data)
{
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		ptr_CHR_bank = data & 0x03;
	}

	return false;
}

bool Mapper_003::ppuMapRead(uint16_t addr, uint32_t& mapped_addr)
{
	if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		mapped_addr = (ptr_CHR_bank * 0x2000) + addr;
		return true;
	}

	return false;
}

bool Mapper_003::ppuMapWrite(uint16_t addr, uint32_t& mapped_addr)
{
	return false;
}

void Mapper_003::reset()
{
	ptr_CHR_bank = 0x00;
}