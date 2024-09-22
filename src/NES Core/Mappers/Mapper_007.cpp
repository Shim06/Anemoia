#include "Mapper_007.h"


Mapper_007::Mapper_007(uint8_t PRGBanks, uint8_t CHRBanks) : Mapper(PRGBanks, CHRBanks)
{
}

Mapper_007::~Mapper_007()
{
}

bool Mapper_007::cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data)
{
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		mapped_addr = (ptr_PRG_bank * 0x8000) + (addr & 0x7FFF);
		return true;
	}

	return false;
}

bool Mapper_007::cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data)
{
	if (addr >= 0x8000 && addr <= 0xFFFF)
	{
		ptr_PRG_bank = data & 0x07;

		switch ((data >> 4) & 0x01)
		{
		case 0:
			mirror = ONESCREEN_LOW;
			break;
		case 1:
			mirror = ONESCREEN_HIGH;
			break;
		}
	}

	return false;
}

bool Mapper_007::ppuMapRead(uint16_t addr, uint32_t& mapped_addr)
{
	if (addr >= 0x0000 && addr <= 0x1FFF)
	{
		mapped_addr = addr;
		return true;
	}

	return false;
}

bool Mapper_007::ppuMapWrite(uint16_t addr, uint32_t& mapped_addr)
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

void Mapper_007::reset()
{
	ptr_PRG_bank = 0;
	mirror = ONESCREEN_LOW;
}

uint8_t Mapper_007::getMirrorMode()
{
	return mirror;
}
