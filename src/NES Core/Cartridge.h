#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <fstream>

#include "Mapper_000.h"
#include "Mapper_001.h"
#include "Mapper_002.h"
#include "Mapper_003.h"
#include "Mapper_004.h"
#include "Mapper_007.h"
#include "Mapper_011.h"
#include "Mapper_018.h"
#include "Mapper_033.h"
#include "Mapper_048.h"
#include "Mapper_066.h"
#include "Mapper_185.h"
#include "Mapper_206.h"

class Cartridge
{
public:
	Cartridge(const std::string& file_name);
	~Cartridge();

	enum MIRROR
	{
		HORIZONTAL,
		VERTICAL,
		ONESCREEN_LOW,
		ONESCREEN_HIGH,
	};
	
	uint8_t hardware_mirror = HORIZONTAL;
	uint8_t mirror = HORIZONTAL;
	bool cpuRead(uint16_t addr, uint8_t& data);
	bool cpuWrite(uint16_t addr, uint8_t data);
	bool ppuRead(uint16_t addr, uint8_t& data);
	bool ppuWrite(uint16_t addr, uint8_t data);
	bool isValidMapper();
	void reset();
	void getMirrorMode();
	std::shared_ptr<Mapper> getMapper();

private:
	std::vector<uint8_t> PRG_memory;
	std::vector<uint8_t> CHR_memory;
	std::shared_ptr<Mapper> ptr_mapper;

	// iNES format header
	uint8_t mapper_ID = 0;
	uint8_t number_PRG_banks = 0;
	uint8_t number_CHR_banks = 0;

};