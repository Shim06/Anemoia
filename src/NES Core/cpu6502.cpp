#include "cpu6502.h"
#include "Bus.h"
#include <stdio.h>

constexpr uint8_t cpu6502::zn_table[256] = {
    #define ENTRY(v) (((v) == 0 ? cpu6502::Z : 0) | ((v) & cpu6502::N))
    ENTRY(0x00), ENTRY(0x01), ENTRY(0x02), ENTRY(0x03), ENTRY(0x04), ENTRY(0x05), ENTRY(0x06), ENTRY(0x07), ENTRY(0x08), ENTRY(0x09), ENTRY(0x0A), ENTRY(0x0B), ENTRY(0x0C), ENTRY(0x0D), ENTRY(0x0E), ENTRY(0x0F),
    ENTRY(0x10), ENTRY(0x11), ENTRY(0x12), ENTRY(0x13), ENTRY(0x14), ENTRY(0x15), ENTRY(0x16), ENTRY(0x17), ENTRY(0x18), ENTRY(0x19), ENTRY(0x1A), ENTRY(0x1B), ENTRY(0x1C), ENTRY(0x1D), ENTRY(0x1E), ENTRY(0x1F),
    ENTRY(0x20), ENTRY(0x21), ENTRY(0x22), ENTRY(0x23), ENTRY(0x24), ENTRY(0x25), ENTRY(0x26), ENTRY(0x27), ENTRY(0x28), ENTRY(0x29), ENTRY(0x2A), ENTRY(0x2B), ENTRY(0x2C), ENTRY(0x2D), ENTRY(0x2E), ENTRY(0x2F),
    ENTRY(0x30), ENTRY(0x31), ENTRY(0x32), ENTRY(0x33), ENTRY(0x34), ENTRY(0x35), ENTRY(0x36), ENTRY(0x37), ENTRY(0x38), ENTRY(0x39), ENTRY(0x3A), ENTRY(0x3B), ENTRY(0x3C), ENTRY(0x3D), ENTRY(0x3E), ENTRY(0x3F),
    ENTRY(0x40), ENTRY(0x41), ENTRY(0x42), ENTRY(0x43), ENTRY(0x44), ENTRY(0x45), ENTRY(0x46), ENTRY(0x47), ENTRY(0x48), ENTRY(0x49), ENTRY(0x4A), ENTRY(0x4B), ENTRY(0x4C), ENTRY(0x4D), ENTRY(0x4E), ENTRY(0x4F),
    ENTRY(0x50), ENTRY(0x51), ENTRY(0x52), ENTRY(0x53), ENTRY(0x54), ENTRY(0x55), ENTRY(0x56), ENTRY(0x57), ENTRY(0x58), ENTRY(0x59), ENTRY(0x5A), ENTRY(0x5B), ENTRY(0x5C), ENTRY(0x5D), ENTRY(0x5E), ENTRY(0x5F),
    ENTRY(0x60), ENTRY(0x61), ENTRY(0x62), ENTRY(0x63), ENTRY(0x64), ENTRY(0x65), ENTRY(0x66), ENTRY(0x67), ENTRY(0x68), ENTRY(0x69), ENTRY(0x6A), ENTRY(0x6B), ENTRY(0x6C), ENTRY(0x6D), ENTRY(0x6E), ENTRY(0x6F),
    ENTRY(0x70), ENTRY(0x71), ENTRY(0x72), ENTRY(0x73), ENTRY(0x74), ENTRY(0x75), ENTRY(0x76), ENTRY(0x77), ENTRY(0x78), ENTRY(0x79), ENTRY(0x7A), ENTRY(0x7B), ENTRY(0x7C), ENTRY(0x7D), ENTRY(0x7E), ENTRY(0x7F),
    ENTRY(0x80), ENTRY(0x81), ENTRY(0x82), ENTRY(0x83), ENTRY(0x84), ENTRY(0x85), ENTRY(0x86), ENTRY(0x87), ENTRY(0x88), ENTRY(0x89), ENTRY(0x8A), ENTRY(0x8B), ENTRY(0x8C), ENTRY(0x8D), ENTRY(0x8E), ENTRY(0x8F),
    ENTRY(0x90), ENTRY(0x91), ENTRY(0x92), ENTRY(0x93), ENTRY(0x94), ENTRY(0x95), ENTRY(0x96), ENTRY(0x97), ENTRY(0x98), ENTRY(0x99), ENTRY(0x9A), ENTRY(0x9B), ENTRY(0x9C), ENTRY(0x9D), ENTRY(0x9E), ENTRY(0x9F),
    ENTRY(0xA0), ENTRY(0xA1), ENTRY(0xA2), ENTRY(0xA3), ENTRY(0xA4), ENTRY(0xA5), ENTRY(0xA6), ENTRY(0xA7), ENTRY(0xA8), ENTRY(0xA9), ENTRY(0xAA), ENTRY(0xAB), ENTRY(0xAC), ENTRY(0xAD), ENTRY(0xAE), ENTRY(0xAF),
    ENTRY(0xB0), ENTRY(0xB1), ENTRY(0xB2), ENTRY(0xB3), ENTRY(0xB4), ENTRY(0xB5), ENTRY(0xB6), ENTRY(0xB7), ENTRY(0xB8), ENTRY(0xB9), ENTRY(0xBA), ENTRY(0xBB), ENTRY(0xBC), ENTRY(0xBD), ENTRY(0xBE), ENTRY(0xBF),
    ENTRY(0xC0), ENTRY(0xC1), ENTRY(0xC2), ENTRY(0xC3), ENTRY(0xC4), ENTRY(0xC5), ENTRY(0xC6), ENTRY(0xC7), ENTRY(0xC8), ENTRY(0xC9), ENTRY(0xCA), ENTRY(0xCB), ENTRY(0xCC), ENTRY(0xCD), ENTRY(0xCE), ENTRY(0xCF),
    ENTRY(0xD0), ENTRY(0xD1), ENTRY(0xD2), ENTRY(0xD3), ENTRY(0xD4), ENTRY(0xD5), ENTRY(0xD6), ENTRY(0xD7), ENTRY(0xD8), ENTRY(0xD9), ENTRY(0xDA), ENTRY(0xDB), ENTRY(0xDC), ENTRY(0xDD), ENTRY(0xDE), ENTRY(0xDF),
    ENTRY(0xE0), ENTRY(0xE1), ENTRY(0xE2), ENTRY(0xE3), ENTRY(0xE4), ENTRY(0xE5), ENTRY(0xE6), ENTRY(0xE7), ENTRY(0xE8), ENTRY(0xE9), ENTRY(0xEA), ENTRY(0xEB), ENTRY(0xEC), ENTRY(0xED), ENTRY(0xEE), ENTRY(0xEF),
    ENTRY(0xF0), ENTRY(0xF1), ENTRY(0xF2), ENTRY(0xF3), ENTRY(0xF4), ENTRY(0xF5), ENTRY(0xF6), ENTRY(0xF7), ENTRY(0xF8), ENTRY(0xF9), ENTRY(0xFA), ENTRY(0xFB), ENTRY(0xFC), ENTRY(0xFD), ENTRY(0xFE), ENTRY(0xFF)
    #undef ENTRY
};

cpu6502::cpu6502()
{
	using a = cpu6502;
	lookup =
	{
		{ "BRK", &a::BRK, &a::IMM, 7 },{ "ORA", &a::ORA, &a::IDX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 3 },{ "ORA", &a::ORA, &a::ZPG, 3 },{ "ASL", &a::ASL, &a::ZPG, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PHP", &a::PHP, &a::IMP, 3 },{ "ORA", &a::ORA, &a::IMM, 2 },{ "ASL", &a::ASL, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::NOP, &a::IMP, 4 },{ "ORA", &a::ORA, &a::ABS, 4 },{ "ASL", &a::ASL, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BPL", &a::BPL, &a::REL, 2 },{ "ORA", &a::ORA, &a::IDY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "ORA", &a::ORA, &a::ZPX, 4 },{ "ASL", &a::ASL, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "CLC", &a::CLC, &a::IMP, 2 },{ "ORA", &a::ORA, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "ORA", &a::ORA, &a::ABX, 4 },{ "ASL", &a::ASL, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "JSR", &a::JSR, &a::ABS, 6 },{ "AND", &a::AND, &a::IDX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "BIT", &a::BIT, &a::ZPG, 3 },{ "AND", &a::AND, &a::ZPG, 3 },{ "ROL", &a::ROL, &a::ZPG, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PLP", &a::PLP, &a::IMP, 4 },{ "AND", &a::AND, &a::IMM, 2 },{ "ROL", &a::ROL, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "BIT", &a::BIT, &a::ABS, 4 },{ "AND", &a::AND, &a::ABS, 4 },{ "ROL", &a::ROL, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BMI", &a::BMI, &a::REL, 2 },{ "AND", &a::AND, &a::IDY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "AND", &a::AND, &a::ZPX, 4 },{ "ROL", &a::ROL, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "SEC", &a::SEC, &a::IMP, 2 },{ "AND", &a::AND, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "AND", &a::AND, &a::ABX, 4 },{ "ROL", &a::ROL, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "RTI", &a::RTI, &a::IMP, 6 },{ "EOR", &a::EOR, &a::IDX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 3 },{ "EOR", &a::EOR, &a::ZPG, 3 },{ "LSR", &a::LSR, &a::ZPG, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PHA", &a::PHA, &a::IMP, 3 },{ "EOR", &a::EOR, &a::IMM, 2 },{ "LSR", &a::LSR, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "JMP", &a::JMP, &a::ABS, 3 },{ "EOR", &a::EOR, &a::ABS, 4 },{ "LSR", &a::LSR, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BVC", &a::BVC, &a::REL, 2 },{ "EOR", &a::EOR, &a::IDY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "EOR", &a::EOR, &a::ZPX, 4 },{ "LSR", &a::LSR, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "CLI", &a::CLI, &a::IMP, 2 },{ "EOR", &a::EOR, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "EOR", &a::EOR, &a::ABX, 4 },{ "LSR", &a::LSR, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "RTS", &a::RTS, &a::IMP, 6 },{ "ADC", &a::ADC, &a::IDX, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 3 },{ "ADC", &a::ADC, &a::ZPG, 3 },{ "ROR", &a::ROR, &a::ZPG, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "PLA", &a::PLA, &a::IMP, 4 },{ "ADC", &a::ADC, &a::IMM, 2 },{ "ROR", &a::ROR, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "JMP", &a::JMP, &a::IND, 5 },{ "ADC", &a::ADC, &a::ABS, 4 },{ "ROR", &a::ROR, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BVS", &a::BVS, &a::REL, 2 },{ "ADC", &a::ADC, &a::IDY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "ADC", &a::ADC, &a::ZPX, 4 },{ "ROR", &a::ROR, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "SEI", &a::SEI, &a::IMP, 2 },{ "ADC", &a::ADC, &a::ABY, 4 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "ADC", &a::ADC, &a::ABX, 4 },{ "ROR", &a::ROR, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "???", &a::NOP, &a::IMP, 2 },{ "STA", &a::STA, &a::IDX, 6 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 6 },{ "STY", &a::STY, &a::ZPG, 3 },{ "STA", &a::STA, &a::ZPG, 3 },{ "STX", &a::STX, &a::ZPG, 3 },{ "???", &a::XXX, &a::IMP, 3 },{ "DEY", &a::DEY, &a::IMP, 2 },{ "???", &a::NOP, &a::IMP, 2 },{ "TXA", &a::TXA, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "STY", &a::STY, &a::ABS, 4 },{ "STA", &a::STA, &a::ABS, 4 },{ "STX", &a::STX, &a::ABS, 4 },{ "???", &a::XXX, &a::IMP, 4 },
		{ "BCC", &a::BCC, &a::REL, 2 },{ "STA", &a::STA, &a::IDY, 6 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 6 },{ "STY", &a::STY, &a::ZPX, 4 },{ "STA", &a::STA, &a::ZPX, 4 },{ "STX", &a::STX, &a::ZPY, 4 },{ "???", &a::XXX, &a::IMP, 4 },{ "TYA", &a::TYA, &a::IMP, 2 },{ "STA", &a::STA, &a::ABY, 5 },{ "TXS", &a::TXS, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 5 },{ "???", &a::NOP, &a::IMP, 5 },{ "STA", &a::STA, &a::ABX, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "???", &a::XXX, &a::IMP, 5 },
		{ "LDY", &a::LDY, &a::IMM, 2 },{ "LDA", &a::LDA, &a::IDX, 6 },{ "LDX", &a::LDX, &a::IMM, 2 },{ "???", &a::XXX, &a::IMP, 6 },{ "LDY", &a::LDY, &a::ZPG, 3 },{ "LDA", &a::LDA, &a::ZPG, 3 },{ "LDX", &a::LDX, &a::ZPG, 3 },{ "???", &a::XXX, &a::IMP, 3 },{ "TAY", &a::TAY, &a::IMP, 2 },{ "LDA", &a::LDA, &a::IMM, 2 },{ "TAX", &a::TAX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "LDY", &a::LDY, &a::ABS, 4 },{ "LDA", &a::LDA, &a::ABS, 4 },{ "LDX", &a::LDX, &a::ABS, 4 },{ "???", &a::XXX, &a::IMP, 4 },
		{ "BCS", &a::BCS, &a::REL, 2 },{ "LDA", &a::LDA, &a::IDY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 5 },{ "LDY", &a::LDY, &a::ZPX, 4 },{ "LDA", &a::LDA, &a::ZPX, 4 },{ "LDX", &a::LDX, &a::ZPY, 4 },{ "???", &a::XXX, &a::IMP, 4 },{ "CLV", &a::CLV, &a::IMP, 2 },{ "LDA", &a::LDA, &a::ABY, 4 },{ "TSX", &a::TSX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 4 },{ "LDY", &a::LDY, &a::ABX, 4 },{ "LDA", &a::LDA, &a::ABX, 4 },{ "LDX", &a::LDX, &a::ABY, 4 },{ "???", &a::XXX, &a::IMP, 4 },
		{ "CPY", &a::CPY, &a::IMM, 2 },{ "CMP", &a::CMP, &a::IDX, 6 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "CPY", &a::CPY, &a::ZPG, 3 },{ "CMP", &a::CMP, &a::ZPG, 3 },{ "DEC", &a::DEC, &a::ZPG, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "INY", &a::INY, &a::IMP, 2 },{ "CMP", &a::CMP, &a::IMM, 2 },{ "DEX", &a::DEX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 2 },{ "CPY", &a::CPY, &a::ABS, 4 },{ "CMP", &a::CMP, &a::ABS, 4 },{ "DEC", &a::DEC, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BNE", &a::BNE, &a::REL, 2 },{ "CMP", &a::CMP, &a::IDY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "CMP", &a::CMP, &a::ZPX, 4 },{ "DEC", &a::DEC, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "CLD", &a::CLD, &a::IMP, 2 },{ "CMP", &a::CMP, &a::ABY, 4 },{ "NOP", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "CMP", &a::CMP, &a::ABX, 4 },{ "DEC", &a::DEC, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
		{ "CPX", &a::CPX, &a::IMM, 2 },{ "SBC", &a::SBC, &a::IDX, 6 },{ "???", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "CPX", &a::CPX, &a::ZPG, 3 },{ "SBC", &a::SBC, &a::ZPG, 3 },{ "INC", &a::INC, &a::ZPG, 5 },{ "???", &a::XXX, &a::IMP, 5 },{ "INX", &a::INX, &a::IMP, 2 },{ "SBC", &a::SBC, &a::IMM, 2 },{ "NOP", &a::NOP, &a::IMP, 2 },{ "???", &a::SBC, &a::IMP, 2 },{ "CPX", &a::CPX, &a::ABS, 4 },{ "SBC", &a::SBC, &a::ABS, 4 },{ "INC", &a::INC, &a::ABS, 6 },{ "???", &a::XXX, &a::IMP, 6 },
		{ "BEQ", &a::BEQ, &a::REL, 2 },{ "SBC", &a::SBC, &a::IDY, 5 },{ "???", &a::XXX, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 8 },{ "???", &a::NOP, &a::IMP, 4 },{ "SBC", &a::SBC, &a::ZPX, 4 },{ "INC", &a::INC, &a::ZPX, 6 },{ "???", &a::XXX, &a::IMP, 6 },{ "SED", &a::SED, &a::IMP, 2 },{ "SBC", &a::SBC, &a::ABY, 4 },{ "NOP", &a::NOP, &a::IMP, 2 },{ "???", &a::XXX, &a::IMP, 7 },{ "???", &a::NOP, &a::IMP, 4 },{ "SBC", &a::SBC, &a::ABX, 4 },{ "INC", &a::INC, &a::ABX, 7 },{ "???", &a::XXX, &a::IMP, 7 },
	};
}

cpu6502::~cpu6502()
{
}

uint8_t cpu6502::read(uint16_t addr)
{
	return bus->cpuRead(addr);
}

void cpu6502::write(uint16_t addr, uint8_t data)
{
	//std::cout << std::hex << "addr: " << +addr << " " << +data << std::endl;
	bus->cpuWrite(addr, data);
}

void cpu6502::clock()
{
	if (cycles == 0)
	{
		//std::cout << std::hex << "PC: " << +PC << std::endl;
		opcode = read(PC);
		SetFlag(U, true);
		PC++;

		// Get number of cycles for opcode
		//std::cout << std::hex << lookup[opcode].name << " " << +(PC - 1) << std::endl;
		cycles = lookup[opcode].cycles;

		// Check if opcode needs additional cycles
		uint8_t additional_cycle1 = (this->*lookup[opcode].addrmode)();
		uint8_t additional_cycle2 = (this->*lookup[opcode].instruction)();

		cycles += (additional_cycle1 & additional_cycle2);

		SetFlag(U, true);
	}
	cycles--;
}

void cpu6502::reset()
{
	addr_abs = 0xFFFC;
	uint16_t low_byte = read(addr_abs);
	uint16_t high_byte = read(addr_abs + 1);

	PC = (high_byte << 8) | low_byte;
	A = 0;
	X = 0;
	Y = 0;
	SP = 0xFD;
	status = 0x00 | U;

	addr_rel = 0x0000;
	addr_abs = 0x0000;
	fetched = 0x00;

	cycles = 8;
}

uint8_t cpu6502::fetch()
{
	if (!(lookup[opcode].addrmode == &cpu6502::IMP))
		fetched = read(addr_abs);
	return fetched;
}

uint8_t cpu6502::GetFlag(FLAGS f)
{
	return ((status & f) > 0) ? 1 : 0;
}

void cpu6502::SetFlag(FLAGS f, bool v)
{
	if (v)
		status |= f;
	else
		status &= ~f;
}

std::vector<uint16_t>& cpu6502::dumpState()
{
	static std::vector<uint16_t> dump;
	dump.clear();

	dump.push_back(A);
	dump.push_back(X);
	dump.push_back(Y);
	dump.push_back(PC);
	dump.push_back(SP);
	dump.push_back(status);
	dump.push_back(fetched);
	dump.push_back(addr_abs);
	dump.push_back(addr_rel);
	dump.push_back(opcode);
	dump.push_back(cycles);
	dump.push_back(temp);
	return dump;
}

void cpu6502::loadState(const std::vector<uint16_t>& dump)
{
	size_t index = 0;

	A = static_cast<uint8_t>(dump[index++]);
	X = static_cast<uint8_t>(dump[index++]);
	Y = static_cast<uint8_t>(dump[index++]);
	PC = dump[index++];
	SP = static_cast<uint8_t>(dump[index++]);
	status = static_cast<uint8_t>(dump[index++]);
	fetched = static_cast<uint8_t>(dump[index++]);
	addr_abs = dump[index++];
	addr_rel = dump[index++];
	opcode = static_cast<uint8_t>(dump[index++]);
	cycles = static_cast<uint8_t>(dump[index++]);
	temp = dump[index++];
	return;
}


uint8_t cpu6502::ABS()
{
    uint8_t low_byte = read(PC++);
    uint8_t high_byte = read(PC++);

    addr_abs = (high_byte << 8) | low_byte;
    return 0;
}

uint8_t cpu6502::ABX()
{
    uint8_t low_byte = read(PC++);
    uint8_t high_byte = read(PC++);

    addr_abs = (high_byte << 8) | low_byte;
    addr_abs += X;

    if ((addr_abs & 0xFF00) != (high_byte << 8))
        return 1;
    else
        return 0;
}

uint8_t cpu6502::ABY()
{
    uint8_t low_byte = read(PC++);
    uint8_t high_byte = read(PC++);

    addr_abs = (high_byte << 8) | low_byte;
    addr_abs += Y;

    if ((addr_abs & 0xFF00) != (high_byte << 8))
        return 1;
    else
        return 0;

}

uint8_t cpu6502::IMM()
{
    addr_abs = PC++;
    return 0;
}

uint8_t cpu6502::IMP()
{
    fetched = A;
    return 0;
}

uint8_t cpu6502::IND()
{
    uint8_t low_byte = read(PC++);
    uint8_t high_byte = read(PC++);

    uint16_t ptr = (high_byte << 8) | low_byte;

    if (low_byte == 0xFF)
    {
        addr_abs = (read(ptr & 0xFF00) << 8) | read(ptr);
    }
    else
    {
        addr_abs = (read(ptr + 1) << 8) | read(ptr);
    }
    return 0;
}

uint8_t cpu6502::IDX()
{
    uint8_t temp = read(PC++);

    uint8_t low_byte = read((uint16_t)(temp + (uint16_t)X) & 0x00FF);
    uint8_t high_byte = read((uint16_t)(temp + (uint16_t)X + 1) & 0x00FF);

    addr_abs = (high_byte << 8) | low_byte;
    return 0;
}

uint8_t cpu6502::IDY()
{
    uint8_t temp = read(PC++);

    uint8_t low_byte = read(temp & 0x00FF);
    uint8_t high_byte = read((temp + 1) & 0x00FF);

    addr_abs = (high_byte << 8) | low_byte;
    addr_abs += Y;

    if ((addr_abs & 0xFF00) != (high_byte << 8))
        return 1;
    else
        return 0;
}

uint8_t cpu6502::REL()
{
    addr_rel = read(PC++);
    if (addr_rel & 0x80) addr_rel |= 0xFF00;
    return 0;
}

uint8_t cpu6502::ZPG()
{
    addr_abs = read(PC++);
    addr_abs &= 0x00FF;
    return 0;
}

uint8_t cpu6502::ZPX()
{
    addr_abs = read(PC++) + X;
    addr_abs &= 0x00FF;
    return 0;
}

uint8_t cpu6502::ZPY()
{
    addr_abs = read(PC++) + Y;
    addr_abs &= 0x00FF;
    return 0;
}

uint8_t cpu6502::LDA()
{
    fetch();
    A = fetched;
    SET_ZN(A);
    return 1;
}

uint8_t cpu6502::LDX()
{
    fetch();
    X = fetched;
    SET_ZN(X);
    return 1;
}

uint8_t cpu6502::LDY()
{
    fetch();
    Y = fetched;
    SET_ZN(Y);
    return 1;
}

uint8_t cpu6502::STA()
{
    write(addr_abs, A);
    return 0;
}

uint8_t cpu6502::STX()
{
    write(addr_abs, X);
    return 0;
}

uint8_t cpu6502::STY()
{
    write(addr_abs, Y);
    return 0;
}

uint8_t cpu6502::TAX()
{
    X = A;
    SET_ZN(X);
    return 0;
}

uint8_t cpu6502::TAY()
{
    Y = A;
    SET_ZN(Y);
    return 0;
}

uint8_t cpu6502::TSX()
{
    X = SP;
    SET_ZN(X);
    return 0;
}

uint8_t cpu6502::TXA()
{
    A = X;
    SET_ZN(A);
    return 0;
}

uint8_t cpu6502::TXS()
{
    SP = X;
    return 0;
}

uint8_t cpu6502::TYA()
{
    A = Y;
    SET_ZN(A);
    return 0;
}

uint8_t cpu6502::PHA()
{
    write(0x0100 + SP, A);
    SP--;
    return 0;
}

uint8_t cpu6502::PHP()
{
    write(0x0100 + SP, status | B | U);
    SET_FLAG(B, 0);
    SET_FLAG(U, 1);
    SP--;
    return 0;
}

uint8_t cpu6502::PLA()
{
    SP++;
    A = read(0x0100 + SP);
    SET_ZN(A);
    return 0;
}

uint8_t cpu6502::PLP()
{
    SP++;
    status = read(0x0100 + SP);
    SET_FLAG(B, 0);
    SET_FLAG(U, 1);
    return 0;
}

uint8_t cpu6502::DEC()
{
    fetch();
    temp = (fetched - 1) & 0x00FF;
    SET_ZN(temp);
    write(addr_abs, (uint8_t)temp);
    return 0;
}

uint8_t cpu6502::DEX()
{
    X--;
    SET_ZN(X);
    return 0;
}

uint8_t cpu6502::DEY()
{
    Y--;
    SET_ZN(Y);
    return 0;
}

uint8_t cpu6502::INC()
{
    fetch();
    temp = (fetched + 1) & 0x00FF;
    SET_ZN(temp);
    write(addr_abs, (uint8_t)temp);
    return 0;
}

uint8_t cpu6502::INX()
{
    X++;
    SET_ZN(X);
    return 0;
}

uint8_t cpu6502::INY()
{
    Y++;
    SET_ZN(Y);
    return 0;
}

uint8_t cpu6502::ADC()
{
    fetch();
    temp = (uint16_t)A + (uint16_t)fetched + (uint16_t)GET_FLAG(C);
    SET_FLAG(C, temp > 255);
    SET_FLAG(V, ((~((uint16_t)A ^ (uint16_t)fetched) & ((uint16_t)A ^ temp)) & 0x0080) != 0);
    A = temp & 0x00FF;
    SET_ZN(A);
    return 1;
}

uint8_t cpu6502::SBC()
{
    fetch();
    uint16_t value = ((uint16_t)fetched) ^ 0x00FF;

    temp = (uint16_t)A + value + (uint16_t)GET_FLAG(C);
    SET_FLAG(C, temp > 255);
    SET_FLAG(V, ((temp ^ (uint16_t)A) & (temp ^ value) & 0x0080) != 0);
    A = temp & 0x00FF;
    SET_ZN(A);
    return 1;
}

uint8_t cpu6502::AND()
{
    fetch();
    A = A & fetched;
    SET_ZN(A);
    return 1;
}

uint8_t cpu6502::EOR()
{
    fetch();
    A = A ^ fetched;
    SET_ZN(A);
    return 1;
}

uint8_t cpu6502::ORA()
{
    fetch();
    A = A | fetched;
    SET_ZN(A);
    return 1;
}

uint8_t cpu6502::ASL()
{
    fetch();
    temp = (uint16_t)fetched << 1;
    SET_FLAG(C, (temp & 0xFF00) > 0);
    SET_ZN(temp & 0x00FF);
    if (lookup[opcode].addrmode == &cpu6502::IMP) A = temp & 0x00FF;
    else write(addr_abs, temp & 0x00FF);

    return 0;
}

uint8_t cpu6502::LSR()
{
    fetch();
    SET_FLAG(C, (fetched & 0x0001) != 0);
    temp = fetched >> 1;
    SET_ZN(temp & 0x00FF);
    if (lookup[opcode].addrmode == &cpu6502::IMP) A = temp & 0x00FF;
    else write(addr_abs, temp & 0x00FF);

    return 0;
}

uint8_t cpu6502::ROL()
{
    fetch();
    temp = (uint16_t)(fetched << 1) | GET_FLAG(C);
    SET_FLAG(C, (temp & 0xFF00) != 0);
    SET_ZN(temp & 0x00FF);
    if (lookup[opcode].addrmode == &cpu6502::IMP) A = temp & 0x00FF;
    else write(addr_abs, temp & 0x00FF);
    return 0;
}

uint8_t cpu6502::ROR()
{
    fetch();
    temp = (uint16_t)(GET_FLAG(C) << 7) | (fetched >> 1);
    SET_FLAG(C, (fetched & 0x01) != 0);
    SET_ZN(temp & 0x00FF);
    if (lookup[opcode].addrmode == &cpu6502::IMP) A = temp & 0x00FF;
    else write(addr_abs, temp & 0x00FF);
    return 0;
}

uint8_t cpu6502::CLC()
{
    SET_FLAG(C, false);
    return 0;
}

uint8_t cpu6502::CLD()
{
    SET_FLAG(D, false);
    return 0;
}

uint8_t cpu6502::CLI()
{
    SET_FLAG(I, false);
    return 0;
}

uint8_t cpu6502::CLV()
{
    SET_FLAG(V, false);
    return 0;
}

uint8_t cpu6502::SEC()
{
    SET_FLAG(C, true);
    return 0;
}

uint8_t cpu6502::SED()
{
    SET_FLAG(D, true);
    return 0;
}

uint8_t cpu6502::SEI()
{
    SET_FLAG(I, true);
    return 0;
}

uint8_t cpu6502::CMP()
{
    fetch();
    temp = (uint16_t)A - (uint16_t)fetched;
    SET_FLAG(C, A >= fetched);
    SET_ZN(temp & 0x00FF);
    return 1;
}

uint8_t cpu6502::CPX()
{
    fetch();
    temp = (uint16_t)X - (uint16_t)fetched;
    SET_FLAG(C, X >= fetched);
    SET_ZN(temp & 0x00FF);
    return 0;
}

uint8_t cpu6502::CPY()
{
    fetch();
    temp = (uint16_t)Y - (uint16_t)fetched;
    SET_FLAG(C, Y >= fetched);
    SET_ZN(temp & 0x00FF);
    return 0;
}

uint8_t cpu6502::BCC()
{
    if (GET_FLAG(C) == 0)
    {
        cycles++;
        addr_abs = PC + addr_rel;

        if ((addr_abs & 0xFF00) != (PC & 0xFF00))
            cycles++;

        PC = addr_abs;
    }
    return 0;
}

uint8_t cpu6502::BCS()
{
    if (GET_FLAG(C) == 1)
    {
        cycles++;
        addr_abs = PC + addr_rel;

        if ((addr_abs & 0xFF00) != (PC & 0xFF00))
            cycles++;

        PC = addr_abs;
    }
    return 0;
}

uint8_t cpu6502::BEQ()
{
    if (GET_FLAG(Z) == 1)
    {
        cycles++;
        addr_abs = PC + addr_rel;

        if ((addr_abs & 0xFF00) != (PC & 0xFF00))
            cycles++;

        PC = addr_abs;
    }
    return 0;
}

uint8_t cpu6502::BMI()
{
    if (GET_FLAG(N) == 1)
    {
        cycles++;
        addr_abs = PC + addr_rel;

        if ((addr_abs & 0xFF00) != (PC & 0xFF00))
            cycles++;

        PC = addr_abs;
    }
    return 0;
}

uint8_t cpu6502::BNE()
{
    if (GET_FLAG(Z) == 0)
    {
        cycles++;
        addr_abs = PC + addr_rel;

        if ((addr_abs & 0xFF00) != (PC & 0xFF00))
            cycles++;

        PC = addr_abs;
    }
    return 0;
}

uint8_t cpu6502::BPL()
{
    if (GET_FLAG(N) == 0)
    {
        cycles++;
        addr_abs = PC + addr_rel;

        if ((addr_abs & 0xFF00) != (PC & 0xFF00))
            cycles++;

        PC = addr_abs;
    }
    return 0;
}

uint8_t cpu6502::BVC()
{
    if (GET_FLAG(V) == 0)
    {
        cycles++;
        addr_abs = PC + addr_rel;

        if ((addr_abs & 0xFF00) != (PC & 0xFF00))
            cycles++;

        PC = addr_abs;
    }
    return 0;
}

uint8_t cpu6502::BVS()
{
    if (GET_FLAG(V) == 1)
    {
        cycles++;
        addr_abs = PC + addr_rel;

        if ((addr_abs & 0xFF00) != (PC & 0xFF00))
            cycles++;

        PC = addr_abs;
    }
    return 0;
}

uint8_t cpu6502::JMP()
{
    PC = addr_abs;
    return 0;
}

uint8_t cpu6502::JSR()
{
    PC--;
    write(0x0100 + SP, (PC >> 8) & 0x00FF);
    write(0x0100 + (uint8_t)(SP - 1), PC & 0x00FF);

    SP -= 2;
    PC = addr_abs;
    return 0;
}

uint8_t cpu6502::RTS()
{
    PC = read(0x0100 | (uint8_t)(SP + 1));
    PC |= read(0x0100 | (uint8_t)(SP + 2)) << 8;

    SP += 2;
    PC++;
    return 0;
}

uint8_t cpu6502::BRK()
{
    write(0x0100 | SP, (PC >> 8) & 0x00FF);
    write(0x0100 | (uint8_t)(SP - 1), PC & 0x00FF);
    status |= (U | B);
    write(0x0100 | (uint8_t)(SP - 2), status);
    SET_FLAG(B, 0);
    SET_FLAG(I, 1);

    uint8_t low_byte = read(0xFFFE);
    uint8_t high_byte = read(0xFFFF);

    SP -= 3;
    PC = (high_byte << 8) | low_byte;
    return 0;
}

uint8_t cpu6502::RTI()
{
    status = read(0x0100 | (uint8_t)(SP + 1));
    SET_FLAG(B, 0);
    SET_FLAG(U, 1);

    PC = (uint16_t)read(0x0100 | (uint8_t)(SP + 2));
    PC |= (uint16_t)read(0x0100 | (uint8_t)(SP + 3)) << 8;
    SP += 3;
    return 0;
}

uint8_t cpu6502::BIT()
{
    fetch();
    temp = A & fetched;
    SET_FLAG(Z, temp == 0);
    SET_FLAG(N, fetched & 0x80);
    SET_FLAG(V, fetched & 0x40);
    return 0;
}

uint8_t cpu6502::NOP()
{
    return 0;
}

uint8_t cpu6502::XXX()
{
    return 0;
}

void cpu6502::IRQ()
{
    if (GET_FLAG(I) == 0)
    {
        write(0x0100 | SP, (PC >> 8) & 0x00FF);
        write(0x0100 | (uint8_t)(SP - 1), PC & 0x00FF);

        write(0x0100 | (uint8_t)(SP - 2), status);

        SET_FLAG(I, 1);

        uint8_t low_byte = read(0xFFFE);
        uint8_t high_byte = read(0xFFFF);

        PC = (high_byte << 8) | low_byte;

        SP -= 3;
        cycles = 7;
    }
}

void cpu6502::NMI()
{
    write(0x0100 | SP, (PC >> 8) & 0x00FF);
    write(0x0100 | (uint8_t)(SP - 1), PC & 0x00FF);

    write(0x0100 | (uint8_t)(SP - 2), status);

    SET_FLAG(I, 1);

    uint8_t low_byte = read(0xFFFA);
    uint8_t high_byte = read(0xFFFB);

    PC = (high_byte << 8) | low_byte;

    SP -= 3;
    cycles = 8;
}