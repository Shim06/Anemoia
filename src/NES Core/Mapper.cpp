#include "Mapper.h"

Mapper::Mapper(uint8_t PRGBanks, uint8_t CHRBanks)
{
	number_PRG_banks = PRGBanks;
	number_CHR_banks = CHRBanks;
}

Mapper::~Mapper()
{
}

void Mapper::reset()
{
}

uint8_t Mapper::getMirrorMode()
{
	return HARDWARE;
}

bool Mapper::getIRQState()
{
	return false;
}

void Mapper::clearIRQ()
{
}

void Mapper::scanline()
{
}

void Mapper::cpuCycle()
{
}

void Mapper::loadRAM(const std::string& path)
{
}

void Mapper::dumpRAM(const std::string& path)
{
}
