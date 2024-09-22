#include "Bus.h"
#include <stdio.h>


Bus::Bus()
{
	memset(cpu_ram, 0, sizeof(cpu_ram));
	memset(controller, 0, sizeof(controller));
	memset(controller_state, 0, sizeof(controller_state));
	cpu.connectBus(this);
}

Bus::~Bus()
{
}

void Bus::cpuWrite(uint16_t addr, uint8_t data)
{
	if (cart->cpuWrite(addr, data))
	{
	}
	else if (addr >= 0x0000 && addr <= 0x1FFF)
		cpu_ram[addr & 0x07FF] = data;
	else if (addr >= 0x2000 && addr <= 0x3FFF)
	{
		ppu.cpuWrite(addr & 0x0007, data);
	}
	else if ((addr >= 0x4000 && addr <= 0x4013) || addr == 0x4015 || addr == 0x4017)
	{
		apu.cpuWrite(addr, data);
	}
	else if (addr == 0x4014)
	{
		OAM_DMA_page = data;
		OAM_DMA_addr = 0x00;
		OAM_DMA_transfer = true;
	}
	else if (addr >= 0x4016 && addr <= 0x4017)
	{
		controller_state[addr & 0x01] = controller[addr & 0x01];
	}
}

uint8_t Bus::cpuRead(uint16_t addr)
{
	uint8_t data = 0x00;

	if (cart->cpuRead(addr, data))
	{
	}
	else if (addr >= 0x0000 && addr <= 0x1FFF)
		data = cpu_ram[addr & 0x07FF];
	else if (addr >= 0x2000 && addr <= 0x3FFF)
		data = ppu.cpuRead(addr & 0x0007);
	else if (addr >= 0x4016 && addr <= 0x4017)
	{
		data = (controller_state[addr & 0x01] & 0x80) > 0;
		controller_state[addr & 0x01] <<= 1;
	}

	return data;
}

void Bus::insertCartridge(const std::shared_ptr<Cartridge>& cartridge)
{
	cart = cartridge;
	ppu.ConnectCartridge(cartridge);
}

void Bus::reset()
{
	for (auto& i : cpu_ram) i = 0x00;
	cart->reset();
	cpu.reset();
	ppu.reset();
	apu.reset();
	system_clock_counter = 0;
	next_cpu_clock = 0;
}

void Bus::clock()
{
	ppu.clock();

	// Cpu Clock - 3 PPU Clock == 1 CPU Clock
	if (system_clock_counter == next_cpu_clock)
	{
		next_cpu_clock += 3;
		if (OAM_DMA_transfer || apu.DMC_DMA_load || apu.DMC_DMA_reload)
		{
			// OAM DMA - CPU halts on put cycles
			// DMC DMA - CPU halts on get cycles
			// Get Cycle - Even cycle | Put Cycle - Odd cycle
			if (OAM_DMA_transfer)
			{
				if (OAM_DMA_alignment)
				{
					if (system_clock_counter % 2 == 1)
						OAM_DMA_alignment = false;
				}
				else
				{
					if (system_clock_counter % 2 == 0)
					{
						OAM_DMA_data = cpuRead(OAM_DMA_page << 8 | OAM_DMA_addr);
					}
					else
					{
						ppu.ptr_OAM[OAM_DMA_addr] = OAM_DMA_data;
						OAM_DMA_addr++;
						if (OAM_DMA_addr == 0x00)
						{
							OAM_DMA_transfer = false;
							OAM_DMA_alignment = true;
						}
					}
				}
			}

			if (apu.DMC_DMA_load)
			{
				//std::cout << "LOAD\n";
				if (apu.DMC_DMA_dummy)
					apu.DMC_DMA_dummy = false;
				else
				{
					if (apu.DMC_DMA_alignment)
					{
						if (system_clock_counter % 2 == 1)
							apu.DMC_DMA_alignment = false;
					}
					else
					{
						apu.setDMCBuffer(cpuRead(apu.getDMCAddress()));
						apu.DMC_DMA_load = false;
					}
				}
			}

			if (apu.DMC_DMA_reload)
			{
				//std::cout << "RELOAD\n";
				if (apu.DMC_DMA_dummy)
					apu.DMC_DMA_dummy = false;
				else
				{
					if (apu.DMC_DMA_alignment)
					{
						if (system_clock_counter % 2 == 1)
							apu.DMC_DMA_alignment = false;
					}
					else
					{
						apu.setDMCBuffer(cpuRead(apu.getDMCAddress()));
						//std::cout << +apu.DMC_sample_byte << "\n";
						apu.DMC_DMA_reload = false;
					}
				}
			}
		}
		else
		{
			cpu.clock();
			cart->getMapper()->cpuCycle();
		}
		apu.clock();
	}

	if (apu.IRQ) cpu.IRQ();
	if (ppu.nmi)
	{
		ppu.nmi = false;
		cpu.NMI();
	}

	if (cart->getMapper()->getIRQState())
	{
		cart->getMapper()->clearIRQ();
		cpu.IRQ();
	}

	system_clock_counter++;
}
