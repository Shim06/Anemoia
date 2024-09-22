#pragma once
#include <array>
#include "Cartridge.h"
#include <cstdint>
#include "cpu6502.h"
#include "ppu2C02.h"
#include "apu2A03.h"

class Bus
{
public:
	Bus();
	~Bus();

public:
	// Devices on the Bus
	cpu6502 cpu;
	ppu2C02 ppu;
	apu2A03 apu;
	std::shared_ptr<Cartridge> cart;
	// RAM
	uint8_t cpu_ram[2048];
	// Controllers
	uint8_t controller[2];

public:
	void cpuWrite(uint16_t addr, uint8_t data);
	uint8_t cpuRead(uint16_t addr);

	void insertCartridge(const std::shared_ptr<Cartridge>& cartridge);
	void reset();
	void clock();

private:
	uint32_t system_clock_counter = 0;
	uint32_t next_cpu_clock = 0;
	uint8_t controller_state[2];

	uint8_t OAM_DMA_page = 0x00;
	uint8_t OAM_DMA_addr = 0x00;
	uint8_t OAM_DMA_data = 0x00;
	bool OAM_DMA_transfer = false;
	bool OAM_DMA_alignment = true;

};