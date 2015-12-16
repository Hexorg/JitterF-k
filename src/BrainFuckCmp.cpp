#include "BrainFuckCmp.h"

namespace Brainfuck {

BrainfuckCompiler::BrainfuckCompiler(std::istream &in, std::ostream &out): in(in), out(out) { 
	memory = new unsigned char [MEMORY_SIZE];
	bf_program = 0;
}

BrainfuckCompiler::BrainfuckCompiler(): in(std::cin), out(std::cout) {
	memory = new unsigned char [MEMORY_SIZE];
	bf_program = 0;
}

BrainfuckCompiler::~BrainfuckCompiler() {
	delete[] memory;
}

int BrainfuckCompiler::run() {
	if (bf_program != 0) {
		return (*bf_program)();
	}	
}

int BrainfuckCompiler::compile(const char *code) {
	const char *instruction;
	// Step 1: scan for open/close brackets
	int loop_level = 0;
	instruction = code;
	while (*instruction != '\0') {
		if (*instruction == '[')
			++loop_level;
		else if (*instruction == ']') {
			--loop_level;
			if (loop_level < 0)
				return BAD_BRACKETS;
		}
	}
	if (loop_level != 0)
		return BAD_BRACKETS;
	// Step 2: aquire memory
	program = new char [MEMORY_SIZE];
	program_ptr = 0;
	/* BF ASM
	 * eax = memory offset
	 * ebx = memory start
	 */
	instruction = code;
	setOffset(0);
	setMemStart(memory);
	while (*instruction != '\0') {
		switch (*instruction) {
			case '+': incCell(); break;
			case '-': decCell(); break;
			case '>': incOffset(); break;
			case '<': decOffset(); break;
		}
	}

}

void BrainfuckCompiler::setOffset(unsigned int value) {
	program[program_ptr++] = 0xb8; // mov eax
	program[program_ptr++] = value & 0xFF;
	program[program_ptr++] = (value>>8) & 0xFF;
	program[program_ptr++] = (value>>8) & 0xFF;
	program[program_ptr++] = (value>>8) & 0xFF;
}

void BrainfuckCompiler::setMemStart(char *value) {
	unsigned int ptr_value = (unsigned int ) value;

	program[program_ptr++] = 0xbb; // mov ebx
	program[program_ptr++] = ptr_value & 0xFF;
	program[program_ptr++] = (ptr_value>>8) & 0xFF;
	program[program_ptr++] = (ptr_value>>8) & 0xFF;
	program[program_ptr++] = (ptr_value>>8) & 0xFF;
}

void BrainfuckCompiler::incCell(void) {	
	program[program_ptr++] = 0x67; // inc dword [ebx+eax]
	program[program_ptr++] = 0xff;
	program[program_ptr++] = 0x04;
	program[program_ptr++] = 0x03;
}

void BrainfuckCompiler::decCell(void) {	
	program[program_ptr++] = 0x67; //dec dword [ebx+eax]
	program[program_ptr++] = 0xff;
	program[program_ptr++] = 0x0c;
	program[program_ptr++] = 0x03;
}

void BrainfuckCompiler::incOffset(void) {	      
	program[program_ptr++] = 0xff; // inc eax
	program[program_ptr++] = 0xc0;
}

void BrainfuckCompiler::decOffset(void) {	
	program[program_ptr++] = 0xff; // dec eax
	program[program_ptr++] = 0xc8;
}

void BrainfuckCompiler::callFunc(void *ptr) {
	unsigned int value = (unsigned int) ptr;

	program[program_ptr++] = 0xe8; // call
	program[program_ptr++] = value & 0xff;
	program[program_ptr++] = (value>>8) & 0xff;
	program[program_ptr++] = (value>>8) & 0xff;
	program[program_ptr++] = (value>>8) & 0xff;
}

void BrainfuckCompiler::ret(void) {	
	program[program_ptr++] = 0xc3; // ret
}

}
