#pragma once
#include "Mapper.h"

class Mapper_206 : public Mapper
{
public:
	Mapper_206(uint8_t PRGBanks, uint8_t CHRBanks);
	~Mapper_206();

	bool cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data) override;
	bool cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data = 0) override;
	bool ppuMapRead(uint16_t addr, uint32_t& mapped_addr) override;
	bool ppuMapWrite(uint16_t addr, uint32_t& mapped_addr) override;

	void reset() override;

private:
	uint8_t bank_select = 0x00; // Bank select register

	uint8_t ptr_PRG_bank_8KB[2];
	uint8_t ptr_PRG_bank_16KB = 0x00;
	uint8_t ptr_CHR_bank_1KB[4];
	uint8_t ptr_CHR_bank_2KB[2];
};