#pragma once
#include "Mapper.h"
#include <vector>
#include <fstream>

class Mapper_001 : public Mapper
{
public:
	Mapper_001(uint8_t PRGBanks, uint8_t CHRBanks);
	~Mapper_001();

	bool cpuMapRead(uint16_t addr, uint32_t& mapped_addr, uint8_t& data) override;
	bool cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data = 0) override;
	bool ppuMapRead(uint16_t addr, uint32_t& mapped_addr) override;
	bool ppuMapWrite(uint16_t addr, uint32_t& mapped_addr) override;
	uint8_t getMirrorMode() override;

	void loadRAM(const std::string& path) override;
	void dumpRAM(const std::string& path) override;

	void reset() override;

private:
	uint8_t mirror = 0;
	uint8_t load = 0x00; // Load Register
	uint8_t control = 0x00; // Control register
	uint8_t CHR_bank_0 = 0x00; // CHR Bank 0 Register
	uint8_t CHR_bank_1 = 0x00; // CHR Bank 1 Register
	uint8_t PRG_bank = 0x00; // PRG Bank Register

	uint8_t load_writes = 0; // Keeps track of number of writes to load register
							 // 5 writes == move data to register

	uint8_t PRG_ROM_bank_mode = 0;
	uint8_t CHR_ROM_bank_mode = 0;
	uint8_t ptr_PRG_bank_32KB = 0x00;
	uint8_t ptr_PRG_bank_16KB_low = 0x00;
	uint8_t ptr_PRG_bank_16KB_high = 0x00;
	uint8_t ptr_CHR_bank_8KB = 0x00;
	uint8_t ptr_CHR_bank_4KB_low = 0x00;
	uint8_t ptr_CHR_bank_4KB_high = 0x00;

	std::vector<uint8_t> mapper_RAM;
	int RAM_size = 32 * 1024; // 8KB RAM
};
