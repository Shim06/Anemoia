#include "Mapper_018.h"
#include <iostream>

// WIP ----------------------------------------------------------------------------------------

Mapper_018::Mapper_018(uint8_t PRGBanks, uint8_t CHRBanks) : Mapper(PRGBanks, CHRBanks)
{
	mapper_RAM.resize(8 * 1024);
	memset(ptr_PRG_bank_8KB, 0, sizeof(ptr_PRG_bank_8KB));
	memset(ptr_CHR_bank_1KB, 0, sizeof(ptr_CHR_bank_1KB));
}

Mapper_018::~Mapper_018()
{
}

bool Mapper_018::cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data)
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

bool Mapper_018::cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data)
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
		ptr_PRG_bank_8KB[0] = (ptr_PRG_bank_8KB[0] & 0x30) | (data & 0x0F);
		break;

	case 0x8001:
		ptr_PRG_bank_8KB[0] = (ptr_PRG_bank_8KB[0] & 0x0F) | (data & 0x03) << 4;
		break;

	case 0x8002:
		ptr_PRG_bank_8KB[1] = (ptr_PRG_bank_8KB[1] & 0x30) | (data & 0x0F);
		break;

	case 0x8003:
		ptr_PRG_bank_8KB[1] = (ptr_PRG_bank_8KB[1] & 0x0F) | (data & 0x03) << 4;
		break;

	case 0x9000:
		ptr_PRG_bank_8KB[2] = (ptr_PRG_bank_8KB[2] & 0x30) | (data & 0x0F);
		break;
	
	case 0x9001:
		ptr_PRG_bank_8KB[2] = (ptr_PRG_bank_8KB[2] & 0x0F) | (data & 0x03) << 4;
		break;

	case 0x9002:
		RAM_write_protection = (data >> 1) & 0x01;
		break;

	case 0xA000:
		ptr_CHR_bank_1KB[0] = (ptr_CHR_bank_1KB[0] & 0xF0) | (data & 0x0F);
		break;

	case 0xA001:
		ptr_CHR_bank_1KB[0] = (ptr_CHR_bank_1KB[0] & 0x0F) | (data & 0x0F) << 4;
		break;

	case 0xA002:
		ptr_CHR_bank_1KB[1] = (ptr_CHR_bank_1KB[1] & 0xF0) | (data & 0x0F);
		break;

	case 0xA003:
		ptr_CHR_bank_1KB[1] = (ptr_CHR_bank_1KB[1] & 0x0F) | (data & 0x0F) << 4;
		break;

	case 0xB000:
		ptr_CHR_bank_1KB[2] = (ptr_CHR_bank_1KB[2] & 0xF0) | (data & 0x0F);
		break;

	case 0xB001:
		ptr_CHR_bank_1KB[2] = (ptr_CHR_bank_1KB[2] & 0x0F) | (data & 0x0F) << 4;
		break;

	case 0xB002:
		ptr_CHR_bank_1KB[3] = (ptr_CHR_bank_1KB[3] & 0xF0) | (data & 0x0F);
		break;

	case 0xB003:
		ptr_CHR_bank_1KB[3] = (ptr_CHR_bank_1KB[3] & 0x0F) | (data & 0x0F) << 4;
		break;

	case 0xC000:
		ptr_CHR_bank_1KB[4] = (ptr_CHR_bank_1KB[4] & 0xF0) | (data & 0x0F);
		break;

	case 0xC001:
		ptr_CHR_bank_1KB[4] = (ptr_CHR_bank_1KB[4] & 0x0F) | (data & 0x0F) << 4;
		break;

	case 0xC002:
		ptr_CHR_bank_1KB[5] = (ptr_CHR_bank_1KB[5] & 0xF0) | (data & 0x0F);
		break;

	case 0xC003:
		ptr_CHR_bank_1KB[5] = (ptr_CHR_bank_1KB[5] & 0x0F) | (data & 0x0F) << 4;
		break;

	case 0xD000:
		ptr_CHR_bank_1KB[6] = (ptr_CHR_bank_1KB[6] & 0xF0) | (data & 0x0F);
		break;

	case 0xD001:
		ptr_CHR_bank_1KB[6] = (ptr_CHR_bank_1KB[6] & 0x0F) | (data & 0x0F) << 4;
		break;

	case 0xD002:
		ptr_CHR_bank_1KB[7] = (ptr_CHR_bank_1KB[7] & 0xF0) | (data & 0x0F);
		break;

	case 0xD003:
		ptr_CHR_bank_1KB[7] = (ptr_CHR_bank_1KB[7] & 0x0F) | (data & 0x0F) << 4;
		break;

	case 0xF000:
		IRQ_counter = IRQ_latch;
		IRQ = true;
		break;

	case 0xF001:
		data = data & 0x0F;
		IRQ_enable = data & 0x01;
		// IRQ counter size
		if (((data >> 1) & 0b000) == 0)
			IRQ_control = 0;

		if ((data >> 1) & 0b001)
			IRQ_control = 1;

		if ((data >> 1) & 0b010)
			IRQ_control = 2;

		if ((data >> 1) & 0b100)
			IRQ_control = 3;
		
		IRQ = true;
		break;

	case 0xF002:
		switch (data & 0x03)
		{
		case 0: mirror = HORIZONTAL; break;
		case 1: mirror = VERTICAL; break;
		case 2: mirror = ONESCREEN_LOW; break;
		case 3: mirror = ONESCREEN_HIGH; break;
		}
		break;
	}

	if (addr >= 0xE000 && addr <= 0xEFFF)
	{
		switch (addr % 4)
		{
		case 0:
			IRQ_latch = (IRQ_latch & 0xFFF0) | data & 0x0F;
			break;

		case 1:
			IRQ_latch = (IRQ_latch & 0xFF0F) | (data & 0x0F) << 4;
			break;

		case 2:
			IRQ_latch = (IRQ_latch & 0xF0FF) | (data & 0x0F) << 8;
			break;

		case 3:
			IRQ_latch = (IRQ_latch & 0x0FFF) | (data & 0x0F) << 12;
			break;
		}
	}

	return false;
}

bool Mapper_018::ppuMapRead(uint16_t addr, uint32_t& mapped_addr)
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

bool Mapper_018::ppuMapWrite(uint16_t addr, uint32_t& mapped_addr)
{
	return false;
}

uint8_t Mapper_018::getMirrorMode()
{
	return mirror;
}

bool Mapper_018::getIRQState()
{
	return IRQ;
}

void Mapper_018::clearIRQ()
{
	IRQ = false;
}

void Mapper_018::cpuCycle()
{
	if (IRQ_counter == 0)
	{
		switch (IRQ_control)
		{
		case 0:
			IRQ_counter = IRQ_latch;
			break;

		case 1:
			IRQ_counter = IRQ_latch & 0x0FFF;
			break;

		case 2:
			IRQ_counter = IRQ_latch & 0x00FF;
			break;

		case 3:
			IRQ_counter = IRQ_latch & 0x000F;
			break;
		}
	}
	else
	{
		if (IRQ_enable)
			IRQ_counter--;

		if (IRQ_counter == 0)
			IRQ = true;
	}


}

void Mapper_018::reset()
{
	mirror = HARDWARE;

	IRQ_enable = false;
	IRQ = false;
	IRQ_control = 0;
	IRQ_latch = 0x00;
	IRQ_counter = 0x00;

	memset(ptr_CHR_bank_1KB, 0, sizeof(ptr_CHR_bank_1KB));
	memset(ptr_PRG_bank_8KB, 0, sizeof(ptr_PRG_bank_8KB));

	ptr_PRG_bank_8KB[3] = (number_PRG_banks * 2) - 1;
}