#include "Mapper_002.h"


Mapper_002::Mapper_002(uint8_t PRGBanks, uint8_t CHRBanks) : Mapper(PRGBanks, CHRBanks)
{
}

Mapper_002::~Mapper_002()
{
}

bool Mapper_002::cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t &data)
{
	if (addr >= 0x8000 && addr <= 0xBFFF)
	{
		mapped_addr = (ptr_PRG_bank_low * 0x4000) + (addr & 0x3FFF);
		return true;
	}
	if (addr >= 0xC000 && addr <= 0xFFFF)
	{
		mapped_addr = (ptr_PRG_bank_high * 0x4000) + (addr & 0x3FFF);
		return true;
	}

	return false;
}

bool Mapper_002::cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data)
{
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		ptr_PRG_bank_low = data & 0x0F;
	}
		
	return false;
}

bool Mapper_002::ppuMapRead(uint16_t addr, uint32_t& mapped_addr)
{
	if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		mapped_addr = addr;
		return true;
	}

	return false;
}

bool Mapper_002::ppuMapWrite(uint16_t addr, uint32_t& mapped_addr)
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

void Mapper_002::reset()
{
	ptr_PRG_bank_low = 0x00;
	ptr_PRG_bank_high = number_PRG_banks - 1;
}