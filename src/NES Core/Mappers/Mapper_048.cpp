#include "Mapper_048.h"

// WIP ----------------------------------------------------------------------------------------

Mapper_048::Mapper_048(uint8_t PRGBanks, uint8_t CHRBanks) : Mapper(PRGBanks, CHRBanks)
{
	mapper_RAM.resize(8 * 1024);
	memset(ptr_PRG_bank_8KB, 0, sizeof(ptr_PRG_bank_8KB));
	memset(ptr_CHR_bank_1KB, 0, sizeof(ptr_CHR_bank_1KB));
	memset(ptr_CHR_bank_2KB, 0, sizeof(ptr_CHR_bank_2KB));
}

Mapper_048::~Mapper_048()
{
}

bool Mapper_048::cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data)
{
	if (addr >= 0x6000 && addr <= 0x7FFF && RAM_write_protection)
	{
		mapped_addr = 0xFFFFFFFF;
		data = mapper_RAM[addr & 0x1FFF];
		return true;
	}

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
	else if (addr >= 0xC000 && addr <= 0xDFFF)
	{
		mapped_addr = (ptr_PRG_bank_8KB[2] * 0x2000) + (addr & 0x1FFF);
		return true;
	}
	else if (addr >= 0xE000 && addr <= 0xFFFF)
	{
		mapped_addr = (ptr_PRG_bank_8KB[3] * 0x2000) + (addr & 0x1FFF);
		return true;
	}


	return false;
}

bool Mapper_048::cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data)
{
	if (addr >= 0x6000 && addr <= 0x7FFF && RAM_write_protection)
	{
		mapped_addr = 0xFFFFFFFF;
		mapper_RAM[addr & 0x1FFF] = data;
		return true;
	}

	switch (addr)
	{
	case 0x8000:
		ptr_PRG_bank_8KB[0] = data & 0x3F;
		break;

	case 0x8001:
		ptr_PRG_bank_8KB[1] = data & 0x3F;
		break;

	case 0x8002:
		ptr_CHR_bank_2KB[0] = data;
		break;

	case 0x8003:
		ptr_CHR_bank_2KB[1] = data;
		break;

	case 0xA000:
		ptr_CHR_bank_1KB[0] = data & 0xFE;
		break;

	case 0xA001:
		ptr_CHR_bank_1KB[1] = data & 0xFE;
		break;

	case 0xA002:
		ptr_CHR_bank_1KB[2] = data & 0xFE;
		break;

	case 0xA003:
		ptr_CHR_bank_1KB[3] = data & 0xFE;
		break;

	case 0xC000:
		IRQ_latch = data ^ 0xFF;
		break;

	case 0xC001:
		IRQ_counter = 0;
		break;

	case 0xC002:
		IRQ_enable = true;
		break;

	case 0xC003:
		IRQ = true;
		break;

	case 0xE000:
		switch ((data >> 6) & 0x01)
		{
		case 0: mirror = VERTICAL; break;
		case 1: mirror = HORIZONTAL; break;
		}
		break;
	}
	return false;
}

bool Mapper_048::ppuMapRead(uint16_t addr, uint32_t& mapped_addr)
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

bool Mapper_048::ppuMapWrite(uint16_t addr, uint32_t& mapped_addr)
{
	return false;
}

uint8_t Mapper_048::getMirrorMode()
{
	return mirror;
}

bool Mapper_048::getIRQState()
{
	return IRQ;
}

void Mapper_048::clearIRQ()
{
	IRQ = false;
}

void Mapper_048::scanline()
{
	if (IRQ_counter == 0)
		IRQ_counter = IRQ_latch;

	else
	{
		IRQ_counter--;
		if (IRQ_counter == 0 && IRQ_enable)
			IRQ = true;
	}
}

void Mapper_048::reset()
{
	mirror = HARDWARE;

	IRQ_enable = false;
	IRQ = false;
	IRQ_control = 0;
	IRQ_latch = 0x00;
	IRQ_counter = 0x00;

	memset(ptr_CHR_bank_1KB, 0, sizeof(ptr_CHR_bank_1KB));
	memset(ptr_CHR_bank_2KB, 0, sizeof(ptr_CHR_bank_2KB));
	memset(ptr_PRG_bank_8KB, 0, sizeof(ptr_PRG_bank_8KB));

	ptr_PRG_bank_8KB[3] = (number_PRG_banks * 2) - 2;
	ptr_PRG_bank_8KB[3] = (number_PRG_banks * 2) - 1;
}