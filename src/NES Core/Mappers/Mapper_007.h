#pragma once
#include "Mapper.h"

class Mapper_007 : public Mapper
{
public:
	Mapper_007(uint8_t PRGBanks, uint8_t CHRBanks);
	~Mapper_007();

	bool cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data) override;
	bool cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data = 0) override;
	bool ppuMapRead(uint16_t addr, uint32_t& mapped_addr) override;
	bool ppuMapWrite(uint16_t addr, uint32_t& mapped_addr) override;

	void reset() override;
	uint8_t getMirrorMode() override;

private:
	uint8_t mirror = ONESCREEN_LOW;
	uint8_t ptr_PRG_bank = 0x00;
};
