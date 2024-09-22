#pragma once
#include "Mapper.h"
#include <vector>

class Mapper_033 : public Mapper
{
public:
	Mapper_033(uint8_t PRGBanks, uint8_t CHRBanks);
	~Mapper_033();

	bool cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data) override;
	bool cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data = 0) override;
	bool ppuMapRead(uint16_t addr, uint32_t& mapped_addr) override;
	bool ppuMapWrite(uint16_t addr, uint32_t& mapped_addr) override;
	uint8_t getMirrorMode() override;

	void reset() override;

private:
	uint8_t mirror = 0;

	uint8_t ptr_PRG_bank_8KB[4];
	uint8_t ptr_CHR_bank_2KB[2];
	uint8_t ptr_CHR_bank_1KB[4];

	std::vector<uint8_t> mapper_RAM;
};