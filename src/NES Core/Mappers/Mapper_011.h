#pragma once
#include "Mapper.h"

class Mapper_011 : public Mapper
{
public:
	Mapper_011(uint8_t PRGBanks, uint8_t CHRBanks);
	~Mapper_011();

	bool cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data) override;
	bool cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data = 0) override;
	bool ppuMapRead(uint16_t addr, uint32_t& mapped_addr) override;
	bool ppuMapWrite(uint16_t addr, uint32_t& mapped_addr) override;

	void reset() override;

	void loadState(const std::vector<uint8_t>& dump) override;
	std::vector<uint8_t>& dumpState() override;

private:
	uint8_t ptr_PRG_bank = 0x00;
	uint8_t ptr_CHR_bank = 0x00;
};