#include "ppu2C02.h"

ppu2C02::ppu2C02()
{
	// Initialize Color Palette
	std::ifstream ifs;
	ifs.open("system\\ntscpalette.pal", std::ifstream::binary);
	if (ifs.is_open())
	{
		uint8_t byte;
		for (uint8_t i = 0; i < 64; i++)
		{
			if (!ifs.read(reinterpret_cast<char*>(&byte), sizeof(byte)))
			{
				break;
			}
			color_palette[i].R = byte;

			if (!ifs.read(reinterpret_cast<char*>(&byte), sizeof(byte)))
			{
				break;
			}
			color_palette[i].G = byte;

			if (!ifs.read(reinterpret_cast<char*>(&byte), sizeof(byte)))
			{
				break;
			}
			color_palette[i].B = byte;
		}
	}

	// Initialize variables
	memset(name_table, 0, sizeof(name_table));
	memset(palette_table, 0, sizeof(palette_table));
	memset(OAM, 0, sizeof(OAM));
	memset(scanline_sprites, 0, sizeof(scanline_sprites));
	memset(sprite_shifter_lsb, 0, sizeof(sprite_shifter_lsb));
	memset(sprite_shifter_msb, 0, sizeof(sprite_shifter_msb));
}

ppu2C02::~ppu2C02()
{
}

uint8_t ppu2C02::cpuRead(uint16_t addr)
{
	uint8_t data = 0x00;
	switch (addr)
	{
	case 0x0000: // PPUCTRL
		break;
	case 0x0001: // PPUMASK
		break;
	case 0x0002: // PPUSTATUS
		data = (status.reg & 0xE0); //| (ppu_data_buffer & 0x1F);
		status.vertical_blank = 0;
		addr_latch = 0;
		break;
	case 0x0003: // OAMADDR
		break;
	case 0x0004: // OAMDATA
		break;
		data = ptr_OAM[OAM_addr];
	case 0x0005: // PPUSCROLL
		break;
	case 0x0006: // PPUDATA
		break;
	case 0x0007: // OAMDMA
		data = ppu_data_buffer;
		ppu_data_buffer = ppuRead(vram_addr.reg);

		if (vram_addr.reg >= 0x3F00) data = ppu_data_buffer;
		vram_addr.reg += (control.VRAM_addr_increment ? 32 : 1);
		break;
	}

	return data;
}

void ppu2C02::cpuWrite(uint16_t addr, uint8_t data)
{
	switch (addr)
	{
	case 0x0000: // PPUCTRL
		control.reg = data;
		temp_vram_addr.nametable_x = control.nametable_x;
		temp_vram_addr.nametable_y = control.nametable_y;
		break;
	case 0x0001: // PPUMASK
		mask.reg = data;
		break;
	case 0x0002: // PPUSTATUS
		break;
	case 0x0003: // OAMADDR
		OAM_addr = data;
		break;
	case 0x0004: // OAMDATA
		ptr_OAM[OAM_addr] = data;
		break;
	case 0x0005: // PPUSCROLL
		if (addr_latch == 0)
		{
			fine_x = data & 0x07;
			temp_vram_addr.coarse_x = data >> 3;
			addr_latch = 1;
		}
		else
		{
			temp_vram_addr.fine_y = data & 0x07;
			temp_vram_addr.coarse_y = data >> 3;
			addr_latch = 0;
		}
		break;
	case 0x0006: // PPUDATA
		if (addr_latch == 0)
		{
			temp_vram_addr.reg = (temp_vram_addr.reg & 0x00FF) | (uint16_t)((data & 0x3F) << 8);
			addr_latch = 1;
		}
		else
		{
			temp_vram_addr.reg = (temp_vram_addr.reg & 0xFF00) | data;
			vram_addr = temp_vram_addr;
			addr_latch = 0;
		}
		break;
	case 0x0007: // OAMDMA
		ppuWrite(vram_addr.reg, data);
		vram_addr.reg += (control.VRAM_addr_increment ? 32 : 1);
		break;
	}
}

uint8_t ppu2C02::ppuRead(uint16_t addr)
{
	uint8_t data = 0x00;
	addr &= 0x3FFF;
	if (cart->ppuRead(addr, data))
	{
	}
	else if (addr >= 0x2000 && addr <= 0x3EFF)
	{
		addr &= 0x0FFF;
		cart->getMirrorMode();
		if (cart->mirror == Cartridge::MIRROR::VERTICAL)
		{
			if (addr >= 0x0000 && addr <= 0x03FF)
				data = name_table[0][addr & 0x03FF];
			if (addr >= 0x0400 && addr <= 0x07FF)
				data = name_table[1][addr & 0x03FF];
			if (addr >= 0x0800 && addr <= 0x0BFF)
				data = name_table[0][addr & 0x03FF];
			if (addr >= 0x0C00 && addr <= 0x0FFF)
				data = name_table[1][addr & 0x03FF];
		}
		else if (cart->mirror == Cartridge::MIRROR::HORIZONTAL)
		{
			if (addr >= 0x0000 && addr <= 0x03FF)
				data = name_table[0][addr & 0x03FF];
			if (addr >= 0x0400 && addr <= 0x07FF)
				data = name_table[0][addr & 0x03FF];
			if (addr >= 0x0800 && addr <= 0x0BFF)
				data = name_table[1][addr & 0x03FF];
			if (addr >= 0x0C00 && addr <= 0x0FFF)
				data = name_table[1][addr & 0x03FF];
		}
		else if (cart->mirror == Cartridge::MIRROR::ONESCREEN_LOW)
		{
			if (addr >= 0x0000 && 0x0FFF)
				data = name_table[0][addr & 0x03FF];
		}
		else if (cart->mirror == Cartridge::MIRROR::ONESCREEN_HIGH)
		{
			if (addr >= 0x0000 && 0x0FFF)
				data = name_table[1][addr & 0x03FF];
		}
	}
	else if (addr >= 0x3F00 && addr <= 0x3FFF)
	{
		addr &= 0x001F;
		// Mirroring of address
		if (addr == 0x0010) addr = 0x0000;
		if (addr == 0x0014) addr = 0x0004;
		if (addr == 0x0018) addr = 0x0008;
		if (addr == 0x001C) addr = 0x000C;
		data = palette_table[addr] & (mask.grayscale ? 0x30 : 0x3F);
	}	

	return data;
}

void ppu2C02::ppuWrite(uint16_t addr, uint8_t data)
{
	// std::cout << std::hex << +addr << " " << +data << std::endl;
	addr &= 0x3FFF;
	if (cart->ppuWrite(addr, data))
	{
	}
	else if (addr >= 0x2000 && addr <= 0x3EFF)
	{
		addr &= 0x0FFF;
		cart->getMirrorMode();
		if (cart->mirror == Cartridge::MIRROR::VERTICAL)
		{
			if (addr >= 0x0000 && addr <= 0x03FF)
				name_table[0][addr & 0x03FF] = data;
			if (addr >= 0x0400 && addr <= 0x07FF)
				name_table[1][addr & 0x03FF] = data;
			if (addr >= 0x0800 && addr <= 0x0BFF)
				name_table[0][addr & 0x03FF] = data;
			if (addr >= 0x0C00 && addr <= 0x0FFF)
				name_table[1][addr & 0x03FF] = data;
		}
		else if (cart->mirror == Cartridge::MIRROR::HORIZONTAL)
		{
			if (addr >= 0x0000 && addr <= 0x03FF)
				name_table[0][addr & 0x03FF] = data;
			if (addr >= 0x0400 && addr <= 0x07FF)
				name_table[0][addr & 0x03FF] = data;
			if (addr >= 0x0800 && addr <= 0x0BFF)
				name_table[1][addr & 0x03FF] = data;
			if (addr >= 0x0C00 && addr <= 0x0FFF)
				name_table[1][addr & 0x03FF] = data;
		}
		else if (cart->mirror == Cartridge::MIRROR::ONESCREEN_LOW)
		{
			if (addr >= 0x0000 && 0x0FFF)
				name_table[0][addr & 0x03FF] = data;
		}
		else if (cart->mirror == Cartridge::MIRROR::ONESCREEN_HIGH)
		{
			if (addr >= 0x0000 && 0x0FFF)
				name_table[1][addr & 0x03FF] = data;
		}
	}
	else if (addr >= 0x3F00 && addr <= 0x3FFF)
	{
		addr &= 0x001F;
		// Mirroring of address
		if (addr == 0x0010) addr = 0x0000;
		if (addr == 0x0014) addr = 0x0004;
		if (addr == 0x0018) addr = 0x0008;
		if (addr == 0x001C) addr = 0x000C;
		palette_table[addr] = data;
	}
}

void ppu2C02::clock()
{
	if (scanline >= -1 && scanline < 240)
	{
		// Odd Frame cycle skip
		if (scanline == 0 && cycle == 0 && odd_frame && (mask.show_background || mask.show_sprites))
		{
			cycle = 1;
		}

		// Reset status register and bit shifter contents
		if (scanline == -1 && cycle == 1)
		{
			status.sprite_zero_hit = 0;
			status.vertical_blank = 0;
			status.sprite_overflow = 0;
			for (uint8_t i = 0; i < sprite_count; i++)
			{
				sprite_shifter_lsb[i] = 0;
				sprite_shifter_msb[i] = 0;

			}
		}

		// Background Rendering
		if ((cycle >= 2 && cycle < 258) || (cycle >= 321 && cycle < 338))
		{
			// Update shifters
			if (mask.show_background)
			{
				bg_shifter_lsb <<= 1;
				bg_shifter_msb <<= 1;
				bg_shifter_attr_lsb <<= 1;
				bg_shifter_attr_msb <<= 1;
			}

			if (mask.show_sprites && cycle >= 1 && cycle <= 258)
			{
				for (uint8_t i = 0; i < sprite_count; i++)
				{
					if (scanline_sprites[i].x > 0)
					{
						scanline_sprites[i].x--;
					}
					else
					{
						sprite_shifter_lsb[i] <<= 1;
						sprite_shifter_msb[i] <<= 1;
					}
				}
			}

			switch ((cycle - 1) % 8)
			{
			case 0:
				// Load shifters
				bg_shifter_lsb = (bg_shifter_lsb & 0xFF00) | bg_tile_lsb;
				bg_shifter_msb = (bg_shifter_msb & 0xFF00) | bg_tile_msb;
				bg_shifter_attr_lsb = (bg_shifter_attr_lsb & 0xFF00) | ((bg_tile_attr & 0b01) ? 0xFF : 0x00);
				bg_shifter_attr_msb = (bg_shifter_attr_msb & 0xFF00) | ((bg_tile_attr & 0b10) ? 0xFF : 0x00);

				// Obtain background tile ID
				bg_tile_id = ppuRead(0x2000 | (vram_addr.reg & 0x0FFF));
				break;
			case 2:
				// Obtain background tile attribute
				bg_tile_attr = ppuRead(0x23C0 | (vram_addr.nametable_y << 11)
												| (vram_addr.nametable_x << 10)
												| ((vram_addr.coarse_y >> 2) << 3)
												| (vram_addr.coarse_x >> 2));
				if (vram_addr.coarse_y & 0x02) bg_tile_attr >>= 4;
				if (vram_addr.coarse_x & 0x02) bg_tile_attr >>= 2;
				bg_tile_attr &= 0x03;
				break;
			case 4:
				// Obtain background tile LSB location
				bg_tile_lsb = ppuRead((control.background_table_addr << 12)
										+ ((uint16_t)bg_tile_id << 4)
										+ (vram_addr.fine_y));
				break;
			case 6:
				// Obtain background tile MSB location
				bg_tile_msb = ppuRead((control.background_table_addr << 12)
										+ ((uint16_t)bg_tile_id << 4)
										+ (vram_addr.fine_y) + 8);
				break;
			case 7:
				// Increment background tile x location
				if (mask.show_background || mask.show_sprites)
				{
					// Check if nametable boundary will be crossed
					if (vram_addr.coarse_x == 31)
					{
						vram_addr.coarse_x = 0;
						vram_addr.nametable_x = ~vram_addr.nametable_x;
					}
					else
					{
						vram_addr.coarse_x++;
					}
				}
				break;
			}
		}
		 
		if (cycle == 256)
		{
			// Increment background tile y location
			if (mask.show_background || mask.show_sprites)
			{
				if (vram_addr.fine_y < 7)
				{
					vram_addr.fine_y++;
				}
				else
				{
					vram_addr.fine_y = 0;
					// Check if nametable boundary is crossed
					if (vram_addr.coarse_y == 29)
					{
						vram_addr.coarse_y = 0;
						vram_addr.nametable_y = ~vram_addr.nametable_y;
					}
					// rows 31-32 are attribute tiles, so reset coarse y
					else if (vram_addr.coarse_y == 31)
					{
						vram_addr.coarse_y = 0;
					}
					else
					{
						vram_addr.coarse_y++;
					}
				}
			}
		}

		if (cycle == 257)
		{
			// Load shifters
			bg_shifter_lsb = (bg_shifter_lsb & 0xFF00) | bg_tile_lsb;
			bg_shifter_msb = (bg_shifter_msb & 0xFF00) | bg_tile_msb;
			bg_shifter_attr_lsb = (bg_shifter_attr_lsb & 0xFF00) | ((bg_tile_attr & 0b01) ? 0xFF : 0x00);
			bg_shifter_attr_msb = (bg_shifter_attr_msb & 0xFF00) | ((bg_tile_attr & 0b10) ? 0xFF : 0x00);

			// Reset background tile x location
			if (mask.show_background || mask.show_sprites)
			{
				vram_addr.coarse_x = temp_vram_addr.coarse_x;
				vram_addr.nametable_x = temp_vram_addr.nametable_x;
			}
		}

		if (scanline == -1 && cycle >= 280 && cycle < 305)
		{
			// Reset background tile y location
			if (mask.show_background || mask.show_sprites)
			{
				vram_addr.coarse_y = temp_vram_addr.coarse_y;
				vram_addr.nametable_y = temp_vram_addr.nametable_y;
				vram_addr.fine_y = temp_vram_addr.fine_y;	
			}
		}

		// Foreground Rendering
		if (cycle == 257 && scanline >= 0)
		{
			std::memset(scanline_sprites, 0xFF, 8 * sizeof(sprite));
			sprite_count = 0;

			sprite_zero = false;
			OAM_entry = 0;
			while (OAM_entry < 64 && sprite_count < 9)
			{
				diff = ((int16_t)scanline - (int16_t)OAM[OAM_entry].y);
				if (diff >= 0 && diff < (control.sprite_size ? 16 : 8))
				{
					if (sprite_count < 8)
					{
						if (OAM_entry == 0)
							sprite_zero = true;

						memcpy(&scanline_sprites[sprite_count], &OAM[OAM_entry], sizeof(sprite));
						sprite_count++;
					}
				}
				OAM_entry++;
			}
			status.sprite_overflow = (sprite_count > 8);
		}

		if (cycle == 340)
		{
			for (uint8_t i = 0; i < sprite_count; i++)
			{

				// 8x8 sprite mode
				if (!control.sprite_size)
				{
					// Check if sprite is not flipped vertically
					if (!(scanline_sprites[i].attribute & 0x80))
					{
						sprite_addr_lsb = (control.sprite_table_addr << 12)
							| (scanline_sprites[i].id << 4)
							| (scanline - scanline_sprites[i].y);
						
					}
					else
					{
						sprite_addr_lsb = (control.sprite_table_addr << 12)
							| (scanline_sprites[i].id << 4)
							| (7 - (scanline - scanline_sprites[i].y));
					}
				}
				// 8x16 sprite mode
				else
				{
					if (!(scanline_sprites[i].attribute & 0x80))
					{
						// Read first 8 pixels
						if (scanline - scanline_sprites[i].y < 8)
						{
							sprite_addr_lsb = ((scanline_sprites[i].id & 0x01) << 12)
												| ((scanline_sprites[i].id & 0xFE) << 4)
												| ((scanline - scanline_sprites[i].y) & 0x07);
						}
						// Read last 8 pixels
						else
						{
							sprite_addr_lsb = ((scanline_sprites[i].id & 0x01) << 12)
								| (((scanline_sprites[i].id & 0xFE) + 1) << 4)
								| ((scanline - scanline_sprites[i].y) & 0x07);
						}
					}
					else
					{
						// Read first 8 pixels
						if (scanline - scanline_sprites[i].y < 8)
						{
							sprite_addr_lsb = ((scanline_sprites[i].id & 0x01) << 12)
								| (((scanline_sprites[i].id & 0xFE) + 1) << 4)
								| (7 - (scanline - scanline_sprites[i].y) & 0x07);
						}
						// Read last 8 pixels
						else
						{
							sprite_addr_lsb = ((scanline_sprites[i].id & 0x01) << 12)
								| ((scanline_sprites[i].id & 0xFE) << 4)
								| (7 - (scanline - scanline_sprites[i].y) & 0x07);
						}
					}
				}

				sprite_addr_msb = sprite_addr_lsb + 8;
				sprite_bits_lsb = ppuRead(sprite_addr_lsb);
				sprite_bits_msb = ppuRead(sprite_addr_msb);


				// Check if sprite is flipped horizontally
				if (scanline_sprites[i].attribute & 0x40)
				{
					auto flipbyte = [](uint8_t b)
					{
						b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
						b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
						b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
						return b;
					};

					sprite_bits_lsb = flipbyte(sprite_bits_lsb);
					sprite_bits_msb = flipbyte(sprite_bits_msb);
				}
				sprite_shifter_lsb[i] = sprite_bits_lsb;
				sprite_shifter_msb[i] = sprite_bits_msb;
			}
		}
	}
	if (scanline == 241 && cycle == 1)
	{
		status.vertical_blank = 1;
		if (control.generate_NMI) nmi = true;
	}

	bg_pixel = 0x00;
	bg_palette = 0x00;

	if (mask.show_background)
	{
		if (mask.show_background_left || cycle >= 9)
		{
			bit_mux = 0x8000 >> fine_x;

			bg_pixel_lsb = (bg_shifter_lsb & bit_mux) > 0;
			bg_pixel_msb = (bg_shifter_msb & bit_mux) > 0;
			bg_pixel = (bg_pixel_msb << 1) | bg_pixel_lsb;

			bg_palette_lsb = (bg_shifter_attr_lsb & bit_mux) > 0;
			bg_palette_msb = (bg_shifter_attr_msb & bit_mux) > 0;
			bg_palette = (bg_palette_msb << 1) | bg_palette_lsb;
		}
	}

	sprite_pixel = 0x00;
	sprite_palette = 0x00;
	sprite_priority = 0;

	if (mask.show_sprites)
	{
		if (mask.show_sprites_left || cycle >= 9)
		{
			rendering_sprite_zero = false;
			for (uint8_t i = 0; i < sprite_count; i++)
			{
				if (scanline_sprites[i].x == 0)
				{

					pixel_lsb = (sprite_shifter_lsb[i] & 0x80) > 0;
					pixel_msb = (sprite_shifter_msb[i] & 0x80) > 0;
					sprite_pixel = (pixel_msb << 1) | pixel_lsb;

					sprite_palette = (scanline_sprites[i].attribute & 0x03) + 0x04;
					sprite_priority = (scanline_sprites[i].attribute & 0x20) == 0;

					if (sprite_pixel != 0)
					{
						if (i == 0)
							rendering_sprite_zero = true;
						break;
					}
				}
			}
		}
	}

	// Choose whether to render foreground or background pixels
	if ((cycle >= 1 && cycle < 256) && (scanline >= 0 && scanline < 240))
	{
		pixel = 0x00;
		palette = 0x00;
		if (bg_pixel > 0 && sprite_pixel == 0)
		{
			pixel = bg_pixel;
			palette = bg_palette;
		}
		else if (bg_pixel == 0 && sprite_pixel == 0)
		{
			pixel = 0x00;
			palette = 0x00;
		}
		else if (bg_pixel == 0 && sprite_pixel > 0)
		{
			pixel = sprite_pixel;
			palette = sprite_palette;
		}
		else if (bg_pixel > 0 && sprite_pixel > 0)
		{
			if (sprite_priority)
			{
				pixel = sprite_pixel;
				palette = sprite_palette;
			}
			else
			{
				pixel = bg_pixel;
				palette = bg_palette;
			}

			if (sprite_zero && rendering_sprite_zero)
			{
				if (mask.show_background && mask.show_sprites)
				{
					if (!(mask.show_background_left | mask.show_sprites_left))
					{
						if (cycle >= 9 && cycle < 258)
							status.sprite_zero_hit = 1;
					}
					else
					{
						if (cycle >= 1 && cycle < 258)
							status.sprite_zero_hit = 1;
					}
				}
			}
		}

		static uint16_t pixel_color;
		uint16_t address = (0x3F00 + (palette << 2) + pixel);
		if (address != cached_color_address)
		{
			cached_color_address = address;
			pixel_color = ppuRead(address) & 0x3F;
		}
		frame_buffer[((cycle - 1) + (scanline*256)) * 3 + 0] = color_palette[pixel_color].R;
		frame_buffer[((cycle - 1) + (scanline*256)) * 3 + 1] = color_palette[pixel_color].G;
		frame_buffer[((cycle - 1) + (scanline*256)) * 3 + 2] = color_palette[pixel_color].B;
	}

	cycle++;

	if (mask.show_background || mask.show_sprites)
	{
		// When using 8x8 sprites, if the BG uses $0000, and the sprites use $1000, 
		// the IRQ counter should decrement on PPU cycle 260.
		if (cycle == 260 && scanline < 240)
			cart->getMapper()->scanline();

		/*
		if (!control.sprite_size && cycle == 260 && scanline < 240)
		{
			if (control.background_table_addr == 0 && control.sprite_table_addr == 1)
				cart->getMapper()->scanline();
		}

		// When using 8x8 sprites, if the BG uses $1000, and the sprites use $0000,
		// the IRQ counter should decrement on PPU cycle 324 of the previous scanline
		if (!control.sprite_size && cycle == 324 && scanline < 240)
		{
			if (control.background_table_addr == 1 && control.sprite_table_addr == 0)
				cart->getMapper()->scanline();
		}

		if (control.sprite_size && sprite_count < 8 && scanline < 240)
		{
			if (control.background_table_addr == 0 && control.sprite_table_addr == 0)
				cart->getMapper()->scanline();
		}
		*/
	}

	if (cycle >= 341)
	{
		cycle = 0;
		scanline++;
		if (scanline >= 261)
		{
			scanline = -1;
			frame_complete = true;
			odd_frame = !odd_frame;
		}
	}
}

void ppu2C02::reset()
{
	status.reg = 0x00;
	mask.reg = 0x00;
	control.reg = 0x00;
	temp_vram_addr.reg = 0x00;
	vram_addr.reg = 0x00;
	fine_x = 0x00;
	write_toggle = 0;
	bg_tile_id = 0x00;
	bg_tile_attr = 0x00;
	bg_tile_lsb = 0x00;
	bg_tile_msb = 0x00;
	bg_shifter_lsb = 0x00;
	bg_shifter_msb = 0x00;
	bg_shifter_attr_lsb = 0x00;
	bg_shifter_attr_msb = 0x00;
	addr_latch = 0;
	ppu_data_buffer = 0x00;
	cycle = 0;
	scanline = 0;
	odd_frame = false;
	cached_color_address = 0;

	memset(frame_buffer.get(), 0, 256 * 240 * 3);
}

std::vector<uint16_t>& ppu2C02::dumpState()
{
	static std::vector<uint16_t> dump;
	dump.clear();

	dump.push_back(nmi);                          
	dump.push_back(cycle);
	dump.push_back(scanline);
	dump.push_back(odd_frame);
	dump.push_back(frame_complete);
	dump.push_back(addr_latch);
	dump.push_back(ppu_data_buffer);
	dump.push_back(status.reg);
	dump.push_back(mask.reg);
	dump.push_back(control.reg);
	dump.push_back(vram_addr.reg);
	dump.push_back(temp_vram_addr.reg);
	dump.push_back(fine_x);
	dump.push_back(write_toggle);
	dump.push_back(bg_tile_id);
	dump.push_back(bg_tile_attr);
	dump.push_back(bg_tile_lsb);
	dump.push_back(bg_tile_msb);
	dump.push_back(bg_shifter_attr_lsb);
	dump.push_back(bg_shifter_attr_msb);
	dump.push_back(sprite_count);
	dump.push_back(sprite_zero);
	dump.push_back(rendering_sprite_zero);
	dump.push_back(cached_color_address);
	dump.push_back(OAM_addr);
	dump.push_back(OAM_entry);
	dump.push_back(sprite_bits_lsb);
	dump.push_back(sprite_bits_msb);
	dump.push_back(sprite_addr_lsb);
	dump.push_back(sprite_addr_msb);
	dump.push_back(bg_pixel);
	dump.push_back(bg_palette);
	dump.push_back(sprite_pixel);
	dump.push_back(sprite_palette);
	dump.push_back(pixel);
	dump.push_back(palette);
	dump.push_back(sprite_priority);
	dump.push_back(diff);
	dump.push_back(bit_mux);
	dump.push_back(bg_pixel_lsb);
	dump.push_back(bg_pixel_msb);
	dump.push_back(bg_palette_lsb);
	dump.push_back(bg_palette_msb);
	dump.push_back(pixel_lsb);
	dump.push_back(pixel_msb);

	dump.insert(dump.end(), palette_table, palette_table + 32);
	dump.insert(dump.end(), sprite_shifter_lsb, sprite_shifter_lsb + 8);
	dump.insert(dump.end(), sprite_shifter_msb, sprite_shifter_msb + 8);

	for (uint16_t i = 0; i < 2; i++)
		dump.insert(dump.end(), name_table[i], name_table[i] + 1024);

	for (int i = 0; i < 64; i++) {
		dump.push_back(OAM[i].y);
		dump.push_back(OAM[i].id);
		dump.push_back(OAM[i].attribute); 
		dump.push_back(OAM[i].x);
	}

	for (int i = 0; i < 8; i++) {
		dump.push_back(scanline_sprites[i].y);
		dump.push_back(scanline_sprites[i].id);
		dump.push_back(scanline_sprites[i].attribute);
		dump.push_back(scanline_sprites[i].x);
	}

	return dump;
}

void ppu2C02::loadState(const std::vector<uint16_t>& dump)
{
	memset(frame_buffer.get(), 0, 256 * 240 * 3);
	size_t index = 0;

	nmi = static_cast<bool>(dump[index++]);
	cycle = static_cast<int16_t>(dump[index++]);
	scanline = static_cast<int16_t>(dump[index++]);
	odd_frame = static_cast<bool>(dump[index++]);
	frame_complete = static_cast<bool>(dump[index++]);
	addr_latch = static_cast<uint8_t>(dump[index++]);
	ppu_data_buffer = static_cast<uint8_t>(dump[index++]);
	status.reg = static_cast<uint8_t>(dump[index++]);
	mask.reg = static_cast<uint8_t>(dump[index++]);
	control.reg = static_cast<uint8_t>(dump[index++]);
	vram_addr.reg = dump[index++];
	temp_vram_addr.reg = dump[index++];
	fine_x = static_cast<uint8_t>(dump[index++]);
	write_toggle = static_cast<uint8_t>(dump[index++]);
	bg_tile_id = static_cast<uint8_t>(dump[index++]);
	bg_tile_attr = static_cast<uint8_t>(dump[index++]);
	bg_tile_lsb = static_cast<uint8_t>(dump[index++]);
	bg_tile_msb = static_cast<uint8_t>(dump[index++]);
	bg_shifter_attr_lsb = static_cast<uint8_t>(dump[index++]);
	bg_shifter_attr_msb = static_cast<uint8_t>(dump[index++]);
	sprite_count = static_cast<uint8_t>(dump[index++]);
	sprite_zero = static_cast<bool>(dump[index++]);;
	rendering_sprite_zero = static_cast<bool>(dump[index++]);
	cached_color_address = dump[index++];
	OAM_addr = static_cast<uint8_t>(dump[index++]);
	OAM_entry = static_cast<uint8_t>(dump[index++]);
	sprite_bits_lsb = static_cast<uint8_t>(dump[index++]);
	sprite_bits_msb = static_cast<uint8_t>(dump[index++]);
	sprite_addr_lsb = dump[index++];
	sprite_addr_msb = dump[index++];
	bg_pixel = static_cast<uint8_t>(dump[index++]);
	bg_palette = static_cast<uint8_t>(dump[index++]);
	sprite_pixel = static_cast<uint8_t>(dump[index++]);
	sprite_palette = static_cast<uint8_t>(dump[index++]);
	pixel = static_cast<uint8_t>(dump[index++]);
	palette = static_cast<uint8_t>(dump[index++]);
	sprite_priority = static_cast<bool>(dump[index++]);
	diff = static_cast<int16_t>(dump[index++]);
	bit_mux = dump[index++];
	bg_pixel_lsb = static_cast<uint8_t>(dump[index++]);
	bg_pixel_msb = static_cast<uint8_t>(dump[index++]);
	bg_palette_lsb = static_cast<uint8_t>(dump[index++]);
	bg_palette_msb = static_cast<uint8_t>(dump[index++]);
	pixel_lsb = static_cast<uint8_t>(dump[index++]);
	pixel_msb = static_cast<uint8_t>(dump[index++]);

	for (uint16_t i = 0; i < 32; i++)
		palette_table[i] = static_cast<uint8_t>(dump[index++]);

	for (uint16_t i = 0; i < 8; i++)
		sprite_shifter_lsb[i] = static_cast<uint8_t>(dump[index++]);

	for (uint16_t i = 0; i < 8; i++)
		sprite_shifter_msb[i] = static_cast<uint8_t>(dump[index++]);

	for (uint16_t i = 0; i < 2; i++)
	{
		for (uint16_t j = 0; j < 1024; j++)
		{
			name_table[i][j] = static_cast<uint8_t>(dump[index++]);
		}
	}

	for (int i = 0; i < 64; i++) {
		OAM[i].y = static_cast<uint8_t>(dump[index++]);
		OAM[i].id = static_cast<uint8_t>(dump[index++]);
		OAM[i].attribute = static_cast<uint8_t>(dump[index++]);
		OAM[i].x = static_cast<uint8_t>(dump[index++]);
	}

	for (int i = 0; i < 8; i++) {
		scanline_sprites[i].y = static_cast<uint8_t>(dump[index++]);
		scanline_sprites[i].id = static_cast<uint8_t>(dump[index++]);
		scanline_sprites[i].attribute = static_cast<uint8_t>(dump[index++]);
		scanline_sprites[i].x = static_cast<uint8_t>(dump[index++]);
	}
	return;
}

void ppu2C02::ConnectCartridge(const std::shared_ptr<Cartridge>& cartridge)
{
	cart = cartridge;
}