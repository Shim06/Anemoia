#include "cpu6502.h"
#include "Bus.h"
#include <stdio.h>

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

uint8_t cpu6502::ABS()
{
	uint16_t low_byte = read(PC);
	PC++;
	uint16_t high_byte = read(PC);
	PC++;

	addr_abs = (high_byte << 8) | low_byte;
	return 0;
}

uint8_t cpu6502::BRK()
{
	PC++;
	SetFlag(I, 1);
	write(0x0100 + SP, (PC >> 8) & 0x00FF);
	SP--;
	write(0x0100 + SP, PC & 0x00FF);
	SP--;

	SetFlag(B, 1);
	write(0x0100 + SP, status);
	SP--;
	SetFlag(B, 0);

	uint16_t low_byte = (uint16_t)read(0xFFFE);
	uint16_t high_byte = (uint16_t)read(0xFFFF);

	PC = (high_byte << 8) | low_byte;
	return 0;
}

uint8_t cpu6502::IDX()
{
	uint16_t temp = read(PC);
	PC++;

	uint16_t low_byte = read((uint16_t)(temp + (uint16_t)X) & 0x00FF);
	uint16_t high_byte = read((uint16_t)(temp + (uint16_t)X + 1) & 0x00FF);

	addr_abs = (high_byte << 8) | low_byte;
	return 0;
}

uint8_t cpu6502::ROR()
{
	fetch();
	temp = (uint16_t)(GetFlag(C) << 7) | (fetched >> 1);
	SetFlag(C, fetched & 0x01);
	SetFlag(Z, (temp & 0x00FF) == 0x00);
	SetFlag(N, temp & 0x80);
	if (lookup[opcode].addrmode == &cpu6502::IMP)
		A = temp & 0x00FF;
	else
		write(addr_abs, temp & 0x00FF);
	return 0;
}

uint8_t cpu6502::INX()
{
	X++;
	SetFlag(Z, X == 0x00);
	SetFlag(N, X & 0x80);
	return 0;
}

uint8_t cpu6502::TYA()
{
	A = Y;
	SetFlag(Z, A == 0x00);
	SetFlag(N, A & 0x80);
	return 0;
}

uint8_t cpu6502::ABX()
{
	uint16_t low_byte = read(PC);
	PC++;
	uint16_t high_byte = read(PC);
	PC++;

	addr_abs = (high_byte << 8) | low_byte;
	addr_abs += X;

	if ((addr_abs & 0xFF00) != (high_byte << 8))
		return 1;
	else
		return 0;
}

uint8_t cpu6502::BVC()
{
	if (GetFlag(V) == 0)
	{
		cycles++;
		addr_abs = PC + addr_rel;

		if ((addr_abs & 0xFF00) != (PC & 0xFF00))
			cycles++;

		PC = addr_abs;
	}
	return 0;
}

uint8_t cpu6502::IDY()
{
	uint16_t temp = read(PC);
	PC++;

	uint16_t low_byte = read(temp & 0x00FF);
	uint16_t high_byte = read((temp + 1) & 0x00FF);

	addr_abs = (high_byte << 8) | low_byte;
	addr_abs += Y;

	if ((addr_abs & 0xFF00) != (high_byte << 8))
		return 1;
	else
		return 0;
}

uint8_t cpu6502::RTI()
{
	SP++;
	status = read(0x0100 + SP);
	status &= ~B;
	status &= ~U;

	SP++;
	PC = (uint16_t)read(0x0100 + SP);
	SP++;
	PC |= (uint16_t)read(0x0100 + SP) << 8;
	return 0;
}

uint8_t cpu6502::INY()
{
	Y++;
	SetFlag(Z, Y == 0x00);
	SetFlag(N, Y & 0x80);
	return 0;
}

uint8_t cpu6502::CLD()
{
	SetFlag(D, false);
	return 0;
}

uint8_t cpu6502::BVS()
{
	if (GetFlag(V) == 1)
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

uint8_t cpu6502::RTS()
{
	SP++;
	PC = read(0x0100 + SP);
	SP++;
	PC |= read(0x0100 + SP) << 8;

	PC++;
	return 0;
}

uint8_t cpu6502::LDA()
{
	fetch();
	A = fetched;
	SetFlag(Z, A == 0x00);
	SetFlag(N, A & 0x80);
	return 1;
}

uint8_t cpu6502::CLC()
{
	SetFlag(C, false);
	return 0;
}

uint8_t cpu6502::JSR()
{
	PC--;
	write(0x0100 + SP, (PC >> 8) & 0x00FF);
	SP--;
	write(0x0100 + SP, PC & 0x00FF);
	SP--;

	PC = addr_abs;
	return 0;
}


uint8_t cpu6502::SBC()
{
	fetch();
	uint16_t value = ((uint16_t)fetched) ^ 0x00FF;

	temp = (uint16_t)A + value + (uint16_t)GetFlag(C);
	SetFlag(C, temp > 255);
	SetFlag(Z, ((temp & 0x00FF) == 0));
	SetFlag(N, temp & 0x80);
	SetFlag(V, (temp ^ (uint16_t)A) & (temp ^ value) & 0x0080);
	A = temp & 0x00FF;
	return 1;
}

uint8_t cpu6502::SEC()
{
	SetFlag(C, true);
	return 0;
}

uint8_t cpu6502::ABY()
{
	uint16_t low_byte = read(PC);
	PC++;
	uint16_t high_byte = read(PC);
	PC++;

	addr_abs = (high_byte << 8) | low_byte;
	addr_abs += Y;

	if ((addr_abs & 0xFF00) != (high_byte << 8))
		return 1;
	else
		return 0;

}

uint8_t cpu6502::REL()
{
	addr_rel = read(PC);
	PC++;
	if (addr_rel & 0x80)
		addr_rel |= 0xFF00;
	return 0;
}

uint8_t cpu6502::IMM()
{
	addr_abs = PC++;
	return 0;
}

uint8_t cpu6502::ZPG()
{
	addr_abs = read(PC);
	PC++;
	addr_abs &= 0x00FF;
	return 0;
}

uint8_t cpu6502::IMP()
{
	fetched = A;
	return 0;
}

uint8_t cpu6502::ZPX()
{
	addr_abs = read(PC) + X;
	PC++;
	addr_abs &= 0x00FF;
	return 0;
}

uint8_t cpu6502::IND()
{
	uint16_t low_byte = read(PC);
	PC++;
	uint16_t high_byte = read(PC);
	PC++;

	uint16_t ptr = (high_byte << 8) | low_byte;
	
	if (low_byte == 0x00FF)
	{
		addr_abs = (read(ptr & 0xFF00) << 8) | read(ptr);
	} 
	else
	{
		addr_abs = (read(ptr + 1) << 8) | read(ptr);
	}
	return 0;
}

uint8_t cpu6502::ZPY()
{
	addr_abs = read(PC) + Y;
	PC++;
	addr_abs &= 0x00FF;
	return 0;
}

uint8_t cpu6502::ADC()
{
	fetch();
	temp = (uint16_t)A + (uint16_t)fetched + (uint16_t)GetFlag(C);
	SetFlag(C, temp > 255);
	SetFlag(Z, (temp & 0x00FF) == 0);
	SetFlag(N, temp & 0x80);
	SetFlag(V, (~((uint16_t)A ^ (uint16_t)fetched) & ((uint16_t)A ^ temp)) & 0x0080);
	A = temp & 0x00FF;
	return 1;
}

uint8_t cpu6502::CLI()
{
	SetFlag(I, false);
	return 0;
}

uint8_t cpu6502::LDX()
{
	fetch();
	X = fetched;
	SetFlag(Z, X == 0x00);
	SetFlag(N, X & 0x80);
	return 1;
}

uint8_t cpu6502::SED()
{
	SetFlag(D, true);
	return 0;
}

uint8_t cpu6502::AND()
{
	fetch();
	A = A & fetched;
	SetFlag(Z, A == 0x00);
	SetFlag(N, A & 0x80);
	return 1;
}

uint8_t cpu6502::CLV()
{
	SetFlag(V, false);
	return 0;
}

uint8_t cpu6502::LDY()
{
	fetch();
	Y = fetched;
	SetFlag(Z, Y == 0x00);
	SetFlag(N, Y & 0x80);
	return 1;
}

uint8_t cpu6502::SEI()
{
	SetFlag(I, true);
	return 0;
}

uint8_t cpu6502::ASL()
{
	fetch();
	temp = (uint16_t)fetched << 1;
	SetFlag(C, (temp & 0xFF00) > 0);
	SetFlag(Z, (temp & 0x00FF) == 0x00);
	SetFlag(N, temp & 0x80);
	if (lookup[opcode].addrmode == &cpu6502::IMP)
		A = temp & 0x00FF;
	else
		write(addr_abs, temp & 0x00FF);

	return 0;
}

uint8_t cpu6502::CMP()
{
	fetch();
	temp = (uint16_t)A - (uint16_t)fetched;
	SetFlag(C, A >= fetched);
	SetFlag(Z, (temp & 0x00FF) == 0x00);
	SetFlag(N, temp & 0x0080);
	return 1;
}

uint8_t cpu6502::LSR()
{
	fetch();
	SetFlag(C, fetched & 0x0001);
	temp = fetched >> 1;
	SetFlag(Z, (temp & 0x00FF) == 0x00);
	SetFlag(N, temp & 0x80);
	if (lookup[opcode].addrmode == &cpu6502::IMP)
		A = temp & 0x00FF;
	else
		write(addr_abs, temp & 0x00FF);

	return 0;
}

uint8_t cpu6502::STA()
{
	write(addr_abs, A);
	return 0;
}

uint8_t cpu6502::BCC()
{
	if (GetFlag(C) == 0)
	{
		cycles++;
		addr_abs = PC + addr_rel;

		if ((addr_abs & 0xFF00) != (PC & 0xFF00))
			cycles++;

		PC = addr_abs;
	}
	return 0;
}

uint8_t cpu6502::CPX()
{
	fetch();
	temp = (uint16_t)X - (uint16_t)fetched;
	SetFlag(C, X >= fetched);
	SetFlag(Z, (temp & 0x00FF) == 0x00);
	SetFlag(N, temp & 0x80);
	return 0;
}

uint8_t cpu6502::NOP()
{
	return 0;
}

uint8_t cpu6502::STX()
{
	write(addr_abs, X);
	return 0;
}

uint8_t cpu6502::BCS()
{
	if (GetFlag(C) == 1)
	{
		cycles++;
		addr_abs = PC + addr_rel;

		if ((addr_abs & 0xFF00) != (PC & 0xFF00))
			cycles++;

		PC = addr_abs;
	}
	return 0;
}

uint8_t cpu6502::CPY()
{
	fetch();
	temp = (uint16_t)Y - (uint16_t)fetched;
	SetFlag(C, Y >= fetched);
	SetFlag(Z, (temp & 0x00FF) == 0x00);
	SetFlag(N, temp & 0x80);
	return 0;
}

uint8_t cpu6502::ORA()
{
	fetch();
	A = A | fetched;
	SetFlag(Z, A == 0x00);
	SetFlag(N, A & 0x80);
	return 1;
}

uint8_t cpu6502::STY()
{
	write(addr_abs, Y);
	return 0;
}

uint8_t cpu6502::BEQ()
{
	if (GetFlag(Z) == 1)
	{
		cycles++;
		addr_abs = PC + addr_rel;

		if ((addr_abs & 0xFF00) != (PC & 0xFF00))
			cycles++;

		PC = addr_abs;
	}
	return 0;
}

uint8_t cpu6502::DEC()
{
	fetch();
	temp = fetched - 1;
	SetFlag(Z, (temp & 0x00FF) == 0x00);
	SetFlag(N, temp & 0x80);
	write(addr_abs, temp & 0x00FF);
	return 0;
}

uint8_t cpu6502::PHA()
{
	write(0x0100 + SP, A);
	SP--;
	return 0;
}

uint8_t cpu6502::TAX()
{
	X = A;
	SetFlag(Z, X == 0x00);
	SetFlag(N, X & 0x80);
	return 0;
}

uint8_t cpu6502::BIT()
{
	fetch();
	temp = A & fetched;	
	SetFlag(Z, (temp & 0x00FF) == 0x00);
	SetFlag(V, fetched & 0x40);
	SetFlag(N, fetched & 0x80);
	return 0;
}

uint8_t cpu6502::DEX()
{
	X--;
	SetFlag(Z, X == 0x00);
	SetFlag(N, X & 0x80);
	return 0;
}

uint8_t cpu6502::PHP()
{
	write(0x0100 + SP, status | B | U);
	SetFlag(B, 0);
	SetFlag(U, 0);
	SP--;
	return 0;
}

uint8_t cpu6502::TAY()
{
	Y = A;
	SetFlag(Z, Y == 0x00);
	SetFlag(N, Y & 0x80);
	return 0;
}

uint8_t cpu6502::BMI()
{
	if (GetFlag(N) == 1)
	{
		cycles++;
		addr_abs = PC + addr_rel;
		
		if ((addr_abs & 0xFF00) != (PC & 0xFF00))
			cycles++;

		PC = addr_abs;
	}
	return 0;
}

uint8_t cpu6502::DEY()
{
	Y--;
	SetFlag(Z, Y == 0x00);
	SetFlag(N, Y & 0x80);
	return 0;
}

uint8_t cpu6502::PLA()
{
	SP++;
	A = read(0x0100 + SP);
	SetFlag(Z, A == 0x00);
	SetFlag(N, A & 0x80);
	return 0;
}

uint8_t cpu6502::TSX()
{
	X = SP;
	SetFlag(Z, X == 0x00);
	SetFlag(N, X & 0x80);
	return 0;
}

uint8_t cpu6502::BNE()
{
	if (GetFlag(Z) == 0)
	{
		cycles++;
		addr_abs = PC + addr_rel;

		if ((addr_abs & 0xFF00) != (PC & 0xFF00))
			cycles++;

		PC = addr_abs;
	}
	return 0;
}

uint8_t cpu6502::EOR()
{
	fetch();
	A = A ^ fetched;
	SetFlag(Z, A == 0x00);
	SetFlag(N, A & 0x80);
	return 1;
}

uint8_t cpu6502::PLP()
{
	SP++;
	status = read(0x0100 + SP);
	SetFlag(U, 1);
	return 0;
}

uint8_t cpu6502::TXA()
{
	A = X;
	SetFlag(Z, A == 0x00);
	SetFlag(N, A & 0x80);
	return 0;
}

uint8_t cpu6502::BPL()
{
	if (GetFlag(N) == 0)
	{
		cycles++;
		addr_abs = PC + addr_rel;

		if ((addr_abs & 0xFF00) != (PC & 0xFF00))
			cycles++;

		PC = addr_abs;
	}
	return 0;
}

uint8_t cpu6502::INC()
{
	fetch();
	temp = fetched + 1;
	SetFlag(Z, (temp & 0x00FF) == 0x00);
	SetFlag(N, temp & 0x80);
	write(addr_abs, temp & 0x00FF);
	return 0;
}

uint8_t cpu6502::ROL()
{
	fetch();
	temp = (uint16_t)(fetched << 1) | GetFlag(C);
	SetFlag(C, temp & 0xFF00);
	SetFlag(Z, (temp & 0x00FF) == 0x00);
	SetFlag(N, temp & 0x80);
	if (lookup[opcode].addrmode == &cpu6502::IMP)
		A = temp & 0x00FF;
	else
		write(addr_abs, temp & 0x00FF);
	return 0;
}

uint8_t cpu6502::TXS()
{
	SP = X;
	return 0;
}

uint8_t cpu6502::XXX()
{
	return 0;
}

void cpu6502::IRQ()
{
	if (GetFlag(I) == 0)
	{
		write(0x0100 + SP, (PC >> 8) & 0x00FF);
		SP--;
		write(0x0100 + SP, PC & 0x00FF);
		SP--;

		SetFlag(B, 0);
		SetFlag(U, 1);
		SetFlag(I, 1);
		write(0x0100 + SP, status);
		SP--;

		addr_abs = 0xFFFE;
		uint16_t low_byte = read(addr_abs);
		uint16_t high_byte = read(addr_abs + 1);

		PC = (high_byte << 8) | low_byte;

		cycles = 7;
	}
}

void cpu6502::NMI()
{
	write(0x0100 + SP, (PC >> 8) & 0x00FF);
	SP--;
	write(0x0100 + SP, PC & 0x00FF);
	SP--;

	SetFlag(B, 0);
	SetFlag(U, 1);
	SetFlag(I, 1);
	write(0x0100 + SP, status);
	SP--;

	addr_abs = 0xFFFA;
	uint16_t low_byte = read(addr_abs);
	uint16_t high_byte = read(addr_abs + 1);

	PC = (high_byte << 8) | low_byte;

	cycles = 8;
}