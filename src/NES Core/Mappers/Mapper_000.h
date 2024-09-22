#pragma once
#include "Mapper.h"

class Mapper_000 : public Mapper
{
public:
	Mapper_000(uint8_t PRGBanks, uint8_t CHRBanks);
	~Mapper_000();

	bool cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data) override;
	bool cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data = 0) override;
	bool ppuMapRead(uint16_t addr, uint32_t& mapped_addr) override;
	bool ppuMapWrite(uint16_t addr, uint32_t& mapped_addr) override;
};

