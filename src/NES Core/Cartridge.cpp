#include "Cartridge.h"
#include <iostream>


Cartridge::Cartridge(const std::string& file_name)
{
	struct cartridge_header
	{
		char name[4];
		uint8_t PRG_ROM_chunks;
		uint8_t CHR_ROM_chunks;
		uint8_t mapper1;
		uint8_t mapper2;
		uint8_t PRG_RAM_size;
		uint8_t tv_system;
		uint8_t tv_system2;
		char unused[5];
	} header;

	// Load Cartridge ROM
	std::ifstream ifs;
	ifs.open(file_name, std::ifstream::binary);
	if (ifs.is_open())
	{
		// Read header
		ifs.read((char*)&header, sizeof(cartridge_header));

		if (header.mapper1 & 0x04)
			ifs.seekg(512, std::ios_base::cur);

		mapper_ID = ((header.mapper2 >> 4) << 4) | header.mapper1 >> 4;
		hardware_mirror = (header.mapper1 & 0x01) ? VERTICAL : HORIZONTAL;

		// Check File format
		uint8_t file_type = 1;
		if ((header.mapper2 & 0x0C) == 0x08) file_type = 2;
		if (file_type == 0)
		{

		}
		else if (file_type == 1)
		{
			number_PRG_banks = header.PRG_ROM_chunks;
			PRG_memory.resize(number_PRG_banks * 16384);
			ifs.read((char*)PRG_memory.data(), PRG_memory.size());


			number_CHR_banks = header.CHR_ROM_chunks;
			if (number_CHR_banks == 0)
			{
				// Create CHR RAM
				CHR_memory.resize(8192);
			}
			else
			{
				// Allocate for ROM
				CHR_memory.resize(number_CHR_banks * 8192);
			}
			ifs.read((char*)CHR_memory.data(), CHR_memory.size());
		}
		else if (file_type == 2)
		{
			number_PRG_banks = ((header.PRG_RAM_size & 0x07) << 8) | header.PRG_ROM_chunks;
			PRG_memory.resize(number_PRG_banks * 16384);
			ifs.read((char*)PRG_memory.data(), PRG_memory.size());

			number_CHR_banks = ((header.PRG_RAM_size & 0x38) << 8) | header.CHR_ROM_chunks;
			CHR_memory.resize(number_CHR_banks * 8192);
			if (number_CHR_banks == 0)
			{
				// Create CHR RAM
				CHR_memory.resize(8192);
			}
			else
			{
				// Allocate for ROM
				CHR_memory.resize(number_CHR_banks * 8192);
			}
			ifs.read((char*)CHR_memory.data(), CHR_memory.size());
		}

		switch (mapper_ID)
		{
		case 0: ptr_mapper = std::make_shared<Mapper_000>(number_PRG_banks, number_CHR_banks); break;
		case 1: ptr_mapper = std::make_shared<Mapper_001>(number_PRG_banks, number_CHR_banks); break;
		case 2: ptr_mapper = std::make_shared<Mapper_002>(number_PRG_banks, number_CHR_banks); break;
		case 3: ptr_mapper = std::make_shared<Mapper_003>(number_PRG_banks, number_CHR_banks); break;
		case 4: ptr_mapper = std::make_shared<Mapper_004>(number_PRG_banks, number_CHR_banks); break;
		case 7: ptr_mapper = std::make_shared<Mapper_007>(number_PRG_banks, number_CHR_banks); break;
		case 11: ptr_mapper = std::make_shared<Mapper_011>(number_PRG_banks, number_CHR_banks); break;
		case 66: ptr_mapper = std::make_shared<Mapper_066>(number_PRG_banks, number_CHR_banks); break;
		case 206: ptr_mapper = std::make_shared<Mapper_206>(number_PRG_banks, number_CHR_banks); break;
		}
		ifs.close();
	}
}

Cartridge::~Cartridge()
{
}

bool Cartridge::cpuRead(uint16_t addr, uint8_t& data)
{
	uint32_t mapped_addr = 0;
	if (ptr_mapper->cpuMapRead(addr, mapped_addr, data))
	{
		if (mapped_addr == 0xFFFFFFFF)
		{
			return true;
		}
		else
		{
			data = PRG_memory[mapped_addr];
			return true;
		}
	}
	else
		return false;
}

bool Cartridge::cpuWrite(uint16_t addr, uint8_t data)
{
	uint32_t mapped_addr = 0;
	if (ptr_mapper->cpuMapWrite(addr, mapped_addr, data))
	{
		if (mapped_addr == 0xFFFFFFFF)
		{
			return true;
		}
		else
		{
			PRG_memory[mapped_addr] = data;
			return true;
		}
	}
	else
		return false;
}

bool Cartridge::ppuRead(uint16_t addr, uint8_t& data)
{
	uint32_t mapped_addr = 0;
	if (ptr_mapper->ppuMapRead(addr, mapped_addr))
	{
		data = CHR_memory[mapped_addr];
		return true;
	}
	else
		return false;
}

bool Cartridge::ppuWrite(uint16_t addr, uint8_t data)
{
	uint32_t mapped_addr = 0;
	if (ptr_mapper->ppuMapWrite(addr, mapped_addr))
	{
		CHR_memory[mapped_addr] = data;
		return true;
	}
	else
		return false;
}

void Cartridge::reset()
{
	if (ptr_mapper != NULL)
		ptr_mapper->reset();
}


bool Cartridge::isValidMapper()
{
	if (ptr_mapper != NULL)
	{
		return true;
	}
	return false;
}

void Cartridge::getMirrorMode()
{
	uint8_t mirror_mode = ptr_mapper->getMirrorMode();
	if (mirror_mode == HARDWARE)
		mirror = hardware_mirror;
	else
		mirror = mirror_mode - 1;
}

std::shared_ptr<Mapper> Cartridge::getMapper()
{
	if (ptr_mapper != NULL)
		return ptr_mapper;
	return nullptr;
}


