#include "Mapper_004.h"

Mapper_004::Mapper_004(uint8_t PRGBanks, uint8_t CHRBanks) : Mapper(PRGBanks, CHRBanks)
{
	mapper_RAM.resize(RAM_size);
	memset(ptr_PRG_bank_8KB, 0, sizeof(ptr_PRG_bank_8KB));
	memset(ptr_CHR_bank_1KB, 0, sizeof(ptr_CHR_bank_1KB));
	memset(ptr_bank_register, 0, sizeof(ptr_bank_register));
}

Mapper_004::~Mapper_004()
{
}

bool Mapper_004::cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data)
{
	if (addr < 0x6000) return false;

	if (addr < 0x8000)
	{
		mapped_addr = 0xFFFFFFFF;
		data = mapper_RAM[addr & 0x1FFF];
		return true;
	}

	uint8_t bank = (addr >> 13) & 0x03;
	mapped_addr = (ptr_PRG_bank_8KB[bank] * 0x2000) + (addr & 0x1FFF);
	return true;
}

bool Mapper_004::cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data)
{
	if (addr < 0x6000) return false;

	if (addr < 0x8000)
	{
		mapped_addr = 0xFFFFFFFF;
		mapper_RAM[addr & 0x1FFF] = data;
		return true;
	}

	// Bank select (even address) | Bank data (odd address)
	switch (addr & 0xE001)
	{
	case 0x8000:
		bank_select = data & 0x07;
		PRG_ROM_bank_mode = (data >> 6) & 0x01;
		CHR_ROM_bank_mode = (data >> 7) & 0x01;
		break;

	case 0x8001:
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
		break;

	// Mirroring (even address)
	case 0xA000:
		switch (data & 0x01)
		{
		case 0: mirror = VERTICAL; break;
		case 1: mirror = HORIZONTAL; break;
		}
		break;

	// IRQ latch (even address) | IRQ reload (odd address)
	case 0xC000:
		IRQ_latch = data;
		break;
	
	case 0xC001:
		IRQ_counter = 0x00;
		break;

	// IRQ disable (even address) | IRQ enable (odd address)
	case 0xE000:
		IRQ_enable = false;
		IRQ = false;
		break;

	case 0xE001:
		IRQ_enable = true;
		break;
	}

	return false;
}

bool Mapper_004::ppuMapRead(uint16_t addr, uint32_t& mapped_addr)
{
	if (addr > 0x1FFF) return false;

	uint8_t bank = (addr >> 10) & 0x07;
	mapped_addr = (ptr_CHR_bank_1KB[bank] * 0x0400) + (addr & 0x03FF);
	return true;
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

void Mapper_004::loadState(const std::vector<uint8_t>& dump)
{
	size_t index = 0;

	mirror = dump[index++];
	bank_select = dump[index++];
	bank_data = dump[index++];
	IRQ_latch = dump[index++];
	IRQ_counter = dump[index++];
	IRQ_enable = dump[index++];
	IRQ = dump[index++];
	PRG_ROM_bank_mode = dump[index++];
	CHR_ROM_bank_mode = dump[index++];

	for (uint8_t i = 0; i < 8; i++)
		ptr_bank_register[i] = dump[index++];

	for (uint8_t i = 0; i < 4; i++)
		ptr_PRG_bank_8KB[i] = dump[index++];

	for (uint8_t i = 0; i < 8; i++)
		ptr_CHR_bank_1KB[i] = dump[index++];

	for (size_t i = 0; i < RAM_size; i++)
		mapper_RAM[i] = dump[index++];

	return;
}

std::vector<uint8_t>& Mapper_004::dumpState()
{
	// TODO: insert return statement here
	static std::vector<uint8_t> dump;
	dump.clear();

	dump.push_back(mirror);
	dump.push_back(bank_select);
	dump.push_back(bank_data);
	dump.push_back(IRQ_latch);
	dump.push_back(IRQ_counter);
	dump.push_back(IRQ_enable);
	dump.push_back(IRQ);
	dump.push_back(PRG_ROM_bank_mode);
	dump.push_back(CHR_ROM_bank_mode);
	dump.insert(dump.end(), ptr_bank_register, ptr_bank_register + 8);
	dump.insert(dump.end(), ptr_PRG_bank_8KB, ptr_PRG_bank_8KB + 4);
	dump.insert(dump.end(), ptr_CHR_bank_1KB, ptr_CHR_bank_1KB + 8);
	dump.insert(dump.end(), mapper_RAM.begin(), mapper_RAM.end());
	return dump;
}
