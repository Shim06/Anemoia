#pragma once
#include "Mapper.h"
#include <vector>

class Mapper_004 : public Mapper
{
public:
	Mapper_004(uint8_t PRGBanks, uint8_t CHRBanks);
	~Mapper_004();

	bool cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data) override;
	bool cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data = 0) override;
	bool ppuMapRead(uint16_t addr, uint32_t& mapped_addr) override;
	bool ppuMapWrite(uint16_t addr, uint32_t& mapped_addr) override;
	uint8_t getMirrorMode() override;
	bool getIRQState() override;
	void clearIRQ() override;
	void scanline() override;

	void reset() override;

private:
	uint8_t mirror = 0;
	uint8_t bank_select = 0x00; // Bank select register
	uint8_t bank_data = 0x00; // Bank data register
	uint16_t IRQ_latch = 0x0000; // IRQ latch register
	uint16_t IRQ_counter = 0x0000;
	bool IRQ_enable = false; // IRQ enable/disable register
	bool IRQ = false;

	uint8_t PRG_ROM_bank_mode = 0;
	uint8_t CHR_ROM_bank_mode = 0;
	uint8_t ptr_bank_register[8];
	uint8_t ptr_PRG_bank_8KB[4];
	uint8_t ptr_CHR_bank_1KB[8];

	std::vector<uint8_t> mapper_RAM;
};