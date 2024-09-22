#include "Mapper_004.h"

Mapper_004::Mapper_004(uint8_t PRGBanks, uint8_t CHRBanks) : Mapper(PRGBanks, CHRBanks)
{
	mapper_RAM.resize(8 * 1024);
	memset(ptr_PRG_bank_8KB, 0, sizeof(ptr_PRG_bank_8KB));
	memset(ptr_CHR_bank_1KB, 0, sizeof(ptr_CHR_bank_1KB));
	memset(ptr_bank_register, 0, sizeof(ptr_bank_register));
}

Mapper_004::~Mapper_004()
{
}

bool Mapper_004::cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data)
{
	if (addr >= 0x6000 && addr <= 0x7FFF)
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

bool Mapper_004::cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data)
{
	if (addr >= 0x6000 && addr <= 0x7FFF)
	{
		mapped_addr = 0xFFFFFFFF;
		mapper_RAM[addr & 0x1FFF] = data;
		return true;
	}

	// Bank select (even address) | Bank data (odd address)
	if (addr >= 0x8000 && addr <= 0x9FFF)
	{
		if (addr % 2 == 0)
		{
			bank_select = data & 0x07;
			PRG_ROM_bank_mode = (data >> 6) & 0x01;
			CHR_ROM_bank_mode = (data >> 7) & 0x01;
		}
		else
		{
			ptr_bank_register[bank_select] = data;

			if (CHR_ROM_bank_mode)
			{
				ptr_CHR_bank_1KB[0] = ptr_bank_register[2];
				ptr_CHR_bank_1KB[1] = ptr_bank_register[3];
				ptr_CHR_bank_1KB[2] = ptr_bank_register[4];
				ptr_CHR_bank_1KB[3] = ptr_bank_register[5];
				ptr_CHR_bank_1KB[4] = (ptr_bank_register[0] & 0xFE);
				ptr_CHR_bank_1KB[5] = (ptr_bank_register[0] & 0xFE) + 1;
				ptr_CHR_bank_1KB[6] = (ptr_bank_register[1] & 0xFE);
				ptr_CHR_bank_1KB[7] = (ptr_bank_register[1] & 0xFE) + 1;
			}
			else
			{
				ptr_CHR_bank_1KB[0] = (ptr_bank_register[0] & 0xFE);
				ptr_CHR_bank_1KB[1] = (ptr_bank_register[0] & 0xFE) + 1;
				ptr_CHR_bank_1KB[2] = (ptr_bank_register[1] & 0xFE);
				ptr_CHR_bank_1KB[3] = (ptr_bank_register[1] & 0xFE) + 1;
				ptr_CHR_bank_1KB[4] = ptr_bank_register[2];
				ptr_CHR_bank_1KB[5] = ptr_bank_register[3];
				ptr_CHR_bank_1KB[6] = ptr_bank_register[4];
				ptr_CHR_bank_1KB[7] = ptr_bank_register[5];
			}

			if (PRG_ROM_bank_mode)
			{
				ptr_PRG_bank_8KB[0] = (number_PRG_banks * 2) - 2;
				ptr_PRG_bank_8KB[2] = ptr_bank_register[6] & 0x3F;
			}
			else
			{
				ptr_PRG_bank_8KB[0] = ptr_bank_register[6] & 0x3F;
				ptr_PRG_bank_8KB[2] = (number_PRG_banks * 2) - 2;
			}
			ptr_PRG_bank_8KB[1] = ptr_bank_register[7] & 0x3F;
			ptr_PRG_bank_8KB[3] = (number_PRG_banks * 2) - 1;

		}
	}

	// Mirroring (even address)
	else if (addr >= 0xA000 && addr <= 0xBFFF)
	{
		if (addr % 2 == 0)
		{
			switch (data & 0x01)
			{
			case 0: mirror = VERTICAL; break;
			case 1: mirror = HORIZONTAL; break;
			}
		}
	}

	// IRQ latch (even address) | IRQ reload (odd address)
	else if (addr >= 0xC000 && addr <= 0xDFFF)
	{
		if (addr % 2 == 0)
			IRQ_latch = data;
		else
			IRQ_counter = 0x00;
	}

	// IRQ disable (even address) | IRQ enable (odd address)
	else if (addr >= 0xE000 && addr <= 0xFFFF)
	{
		if (addr % 2 == 1)
			IRQ_enable = true;
		else
		{
			IRQ_enable = false;
			IRQ = false;
		}
	}

	return false;
}

bool Mapper_004::ppuMapRead(uint16_t addr, uint32_t& mapped_addr)
{

	if (addr >= 0x0000 && addr <= 0x03FF)
	{
		mapped_addr = (ptr_CHR_bank_1KB[0] * 0x0400) + (addr & 0x03FF);
		return true;
	}
	else if (addr >= 0x0400 && addr <= 0x07FF)
	{
		mapped_addr = (ptr_CHR_bank_1KB[1] * 0x0400) + (addr & 0x03FF);
		return true;
	}
	else if (addr >= 0x0800 && addr <= 0x0BFF)
	{
		mapped_addr = (ptr_CHR_bank_1KB[2] * 0x0400) + (addr & 0x03FF);
		return true;
	}
	else if (addr >= 0x0C00 && addr <= 0x0FFF)
	{
		mapped_addr = (ptr_CHR_bank_1KB[3] * 0x0400) + (addr & 0x03FF);
		return true;
	}
	else if (addr >= 0x1000 && addr <= 0x13FF)
	{
		mapped_addr = (ptr_CHR_bank_1KB[4] * 0x0400) + (addr & 0x03FF);
		return true;
	}
	else if (addr >= 0x1400 && addr <= 0x17FF)
	{
		mapped_addr = (ptr_CHR_bank_1KB[5] * 0x0400) + (addr & 0x03FF);
		return true;
	}
	else if (addr >= 0x1800 && addr <= 0x1BFF)
	{
		mapped_addr = (ptr_CHR_bank_1KB[6] * 0x0400) + (addr & 0x03FF);
		return true;
	}
	else if (addr >= 0x1C00 && addr <= 0x1FFF)
	{
		mapped_addr = (ptr_CHR_bank_1KB[7] * 0x0400) + (addr & 0x03FF);
		return true;
	}
	return false;
}

bool Mapper_004::ppuMapWrite(uint16_t addr, uint32_t& mapped_addr)
{
	return false;
}

uint8_t Mapper_004::getMirrorMode()
{
	return mirror;
}

bool Mapper_004::getIRQState()
{
	return IRQ;
}

void Mapper_004::clearIRQ()
{
	IRQ = false;
}

void Mapper_004::scanline()
{
	if (IRQ_counter == 0)
		IRQ_counter = IRQ_latch;
	else
	{
		IRQ_counter--;
		if (IRQ_counter == 0 && IRQ_enable)
		{
			IRQ = true;
		}
	}


}

void Mapper_004::reset()
{
	bank_select = 0x00;
	PRG_ROM_bank_mode = 0;
	CHR_ROM_bank_mode = 0;
	mirror = HARDWARE;

	IRQ_enable = false;
	IRQ = false;
	IRQ_latch = 0x00;
	IRQ_counter = 0x00;

	memset(ptr_CHR_bank_1KB, 0, sizeof(ptr_CHR_bank_1KB));

	ptr_PRG_bank_8KB[2] = (number_PRG_banks * 2) - 2;
	ptr_PRG_bank_8KB[3] = (number_PRG_banks * 2) - 1;
}