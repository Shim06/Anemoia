#pragma once
#include <cstdint>
#include <string>

enum MIRROR
{
	HARDWARE,
	HORIZONTAL,
	VERTICAL,
	ONESCREEN_LOW,
	ONESCREEN_HIGH,
};

class Mapper
{
public:
	Mapper(uint8_t PRGBanks, uint8_t CHRBanks);
	~Mapper();

	virtual bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) = 0;
	virtual bool cpuMapWrite(uint16_t addr, uint32_t& mapped_addr, uint8_t data = 0) = 0;
	virtual bool ppuMapRead(uint16_t addr, uint32_t& mapped_addr) = 0;
	virtual bool ppuMapWrite(uint16_t addr, uint32_t& mapped_addr) = 0;

	virtual void reset();
	virtual uint8_t getMirrorMode();
	virtual bool getIRQState();
	virtual void clearIRQ();
	virtual void scanline();
	virtual void cpuCycle();
	virtual void loadRAM(const std::string& path);
	virtual void dumpRAM(const std::string& path);

protected:

	uint8_t number_PRG_banks = 0;
	uint8_t number_CHR_banks = 0;
};