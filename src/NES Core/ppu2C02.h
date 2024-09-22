#pragma once
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Cartridge.h"

class ppu2C02
{
public:
	ppu2C02();
	~ppu2C02();

	void ConnectCartridge(const std::shared_ptr<Cartridge>& cartridge);
	void clock();
	bool nmi = false;

	uint8_t cpuRead(uint16_t addr);
	void cpuWrite(uint16_t addr, uint8_t data);
	uint8_t ppuRead(uint16_t addr);
	void ppuWrite(uint16_t addr, uint8_t data);
	
	void reset();

	std::shared_ptr<Cartridge> cart;
	uint8_t name_table[2][1024]; // VRAM
	uint8_t pattern_table[2][4096];
	uint8_t palette_table[32]; // PPU Color palletes
	std::unique_ptr<uint8_t[]> frame_buffer = std::make_unique<uint8_t[]>(256 * 240 * 3); // Frame buffer

	struct color
	{
		uint8_t R = 0;
		uint8_t G = 0;
		uint8_t B = 0;
	};
	color color_palette[64]; // Number of colors in the palette file

	int16_t cycle = 0;
	int16_t scanline = 0;
	bool odd_frame = false;
	bool frame_complete = false;

	uint8_t addr_latch = 0x00;
	uint8_t ppu_data_buffer = 0x00;

private:
	// PPU Registers
	union // Status Register
	{
		struct
		{
			uint8_t unused : 5;
			uint8_t sprite_overflow : 1;
			uint8_t sprite_zero_hit : 1;
			uint8_t vertical_blank : 1;
		};

		uint8_t reg = 0x00;
	} status;

	union // Mask Register
	{
		struct
		{
			uint8_t	grayscale : 1;
			uint8_t	show_background_left : 1;
			uint8_t	show_sprites_left : 1;
			uint8_t	show_background : 1;
			uint8_t	show_sprites : 1;
			uint8_t emphasize_red : 1;
			uint8_t emphasize_green : 1;
			uint8_t emphasize_blue : 1;
		};

		uint8_t reg = 0x00;
	} mask;

	union // Control Register
	{
		struct
		{
			uint8_t	nametable_x : 1;
			uint8_t	nametable_y : 1;
			uint8_t	VRAM_addr_increment : 1;
			uint8_t	sprite_table_addr : 1;
			uint8_t	background_table_addr : 1;
			uint8_t sprite_size : 1;
			uint8_t PPU_master : 1;
			uint8_t generate_NMI : 1;
		};

		uint8_t reg = 0x00;
	} control;

	union internal_register
	{
		struct
		{
			uint16_t coarse_x : 5;
			uint16_t coarse_y : 5;
			uint16_t nametable_x : 1;
			uint16_t nametable_y : 1;
			uint16_t fine_y : 3;
			uint16_t unused : 1;
		};
		uint16_t reg = 0x0000;
	};

	internal_register vram_addr;
	internal_register temp_vram_addr;
	uint8_t fine_x = 0x00;
	uint8_t write_toggle = 0;

	uint8_t bg_tile_id = 0x00;
	uint8_t bg_tile_attr = 0x00;
	uint8_t bg_tile_lsb = 0x00;
	uint8_t bg_tile_msb = 0x00;

	uint16_t bg_shifter_lsb = 0x0000;
	uint16_t bg_shifter_msb = 0x0000;
	uint16_t bg_shifter_attr_lsb = 0x0000;
	uint16_t bg_shifter_attr_msb = 0x0000;

	struct sprite
	{
		uint8_t y;
		uint8_t id;
		uint8_t attribute;
		uint8_t x;
	};
	
	sprite OAM[64];
	sprite scanline_sprites[8];
	uint8_t sprite_count = 0;
	uint8_t sprite_shifter_lsb[8];
	uint8_t sprite_shifter_msb[8];

	bool sprite_zero = 0;
	bool rendering_sprite_zero = 0;

	uint16_t cached_color_address = 0;

	struct pixel
	{
		int16_t x = 0;
		int16_t y = 0;
		color RGB;
	};
	pixel screen; // NES screen

	uint8_t OAM_addr = 0x00;
	uint8_t OAM_entry = 0;

	uint8_t sprite_bits_lsb = 0x00, sprite_bits_msb = 0x00;
	uint16_t sprite_addr_lsb = 0x0000, sprite_addr_msb = 0x0000;
	uint8_t bg_pixel = 0x00;
	uint8_t bg_palette = 0x00;
	uint8_t sprite_pixel = 0x00;
	uint8_t sprite_palette = 0x00;
	uint8_t pixel = 0x00;
	uint8_t palette = 0x00;
	bool sprite_priority = 0;

public:
	uint8_t* ptr_OAM = (uint8_t*)OAM;
};
