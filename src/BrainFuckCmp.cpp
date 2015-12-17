#include "BrainFuckCmp.h"
#include <sys/mman.h>
#include <fstream>

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
	munmap(program, MEMORY_SIZE);
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
		instruction++;
	}
	if (loop_level != 0)
		return BAD_BRACKETS;
	// Step 2: aquire memory
	program = (char *) mmap(NULL, MEMORY_SIZE, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (program == MAP_FAILED) {
		out << "Can't allocate space for program" << std::endl;
		return -1;
	}
	bf_program = (int(*)(void)) program;
	program_ptr = 0;
	//Step 3: compile
	/* BF ASM
	 * ebx = memory offset
	 * ecx = memory start
	 * eax = contains [eax+ecx] before entering loop
	 */
	instruction = code;
	initFunction();
	setOffset(0);
	setMemStart(memory);
	while (*instruction != '\0') {
		switch (*instruction) {
			case '+': incCell(); break;
			case '-': decCell(); break;
			case '>': incOffset(); break;
			case '<': decOffset(); break;
			case '[': startLoop(); break;
			case ']': endLoop(); break;
			case '.': callOutputChar(); break;
			case ',': callInputChar(); break;
		}
		++instruction;
	}
	ret();
	// Step 4: mark the code executable
	std::ofstream fs("bf.bin", std::ios::binary);
	fs.write(program, program_ptr);
	fs.close();
	if (mprotect(program, MEMORY_SIZE, PROT_EXEC | PROT_READ) != 0)
		out << "mprotect returned != 0" << std::endl;
}

void BrainfuckCompiler::initFunction(void) {
	program[program_ptr++] = 0x55; // push rbp
	program[program_ptr++] = 0x48; // mov rbp, rsp
	program[program_ptr++] = 0x89; 
	program[program_ptr++] = 0xe5; 

}

void BrainfuckCompiler::setOffset(unsigned int value) {
	program[program_ptr++] = 0xBB; // mov ebx, value
	program[program_ptr++] = value & 0xFF;
	program[program_ptr++] = (value>>8) & 0xFF;
	program[program_ptr++] = (value>>16) & 0xFF;
	program[program_ptr++] = (value>>24) & 0xFF;
}

void BrainfuckCompiler::setMemStart(void *ptr) {
	unsigned long value = (unsigned long) ptr;
	program[program_ptr++] = 0x48; // mov rcx, ptr
	program[program_ptr++] = 0xB9;
	program[program_ptr++] = value & 0xFF;
	program[program_ptr++] = (value>>8) & 0xFF;
	program[program_ptr++] = (value>>16) & 0xFF;
	program[program_ptr++] = (value>>24) & 0xFF;
	program[program_ptr++] = (value>>32) & 0xFF;
	program[program_ptr++] = (value>>40) & 0xFF;
	program[program_ptr++] = (value>>48) & 0xFF;
	program[program_ptr++] = (value>>56) & 0xFF;
}

void BrainfuckCompiler::incCell(void) {	
	program[program_ptr++] = 0xfe; // inc byte [rbx+rcx]
	program[program_ptr++] = 0x04;
	program[program_ptr++] = 0x0B;
}

void BrainfuckCompiler::decCell(void) {	
	program[program_ptr++] = 0xfe; // dec byte [rbx+rcx]
	program[program_ptr++] = 0x0c;
	program[program_ptr++] = 0x0B;
}

void BrainfuckCompiler::incOffset(void) {	      
	program[program_ptr++] = 0xff; // inc ebx
	program[program_ptr++] = 0xc3;
}

void BrainfuckCompiler::decOffset(void) {	
	program[program_ptr++] = 0xff; // dec ebx
	program[program_ptr++] = 0xcB;
}

void BrainfuckCompiler::startLoop(void) {
	struct LoopData data;
	data.loop_start = program_ptr;
	// save current program_ptr
	// mov al, [rbx+rcx]
	program[program_ptr++] = 0x8a;
	program[program_ptr++] = 0x04;
	program[program_ptr++] = 0x0b;
	// test al, al
	program[program_ptr++] = 0x84;
	program[program_ptr++] = 0xC0;
	// jz	end_loop ; to be calucalted in endLoop()
	program[program_ptr++] = 0x0f;
	program[program_ptr++] = 0x84;
	data.loop_end_label = program_ptr;
	program[program_ptr++] = 0x00; // actual JMP address
	program[program_ptr++] = 0x00;
	program[program_ptr++] = 0x00;
	program[program_ptr++] = 0x00;
	loopStack.push(data);
}

void BrainfuckCompiler::endLoop(void) {
	struct LoopData data = loopStack.top();
	loopStack.pop();
	unsigned int value = data.loop_start - program_ptr - 5;
	// jmp start_loop
	program[program_ptr++] = 0xe9;
	program[program_ptr++] = (value) & 0xFF;
	program[program_ptr++] = (value>>8) & 0xFF;
	program[program_ptr++] = (value>>16) & 0xFF;
	program[program_ptr++] = (value>>24) & 0xFF;
	// overwrite jz address in startLoop
	value = program_ptr - data.loop_end_label-4;
	program[data.loop_end_label] = value & 0xFF;
	program[data.loop_end_label+1] = (value>>8) & 0xFF;
	program[data.loop_end_label+2] = (value>>16) & 0xFF;
	program[data.loop_end_label+3] = (value>>24) & 0xFF;

}

void BrainfuckCompiler::callOutputChar(void) {
	// xor rax, rax
	program[program_ptr++] = 0x48;
	program[program_ptr++] = 0x31;
	program[program_ptr++] = 0xC0;
	// mov al, [rbx+rcx]
	program[program_ptr++] = 0x8A;
	program[program_ptr++] = 0x04;
	program[program_ptr++] = 0x0b;
	// push rax
	program[program_ptr++] = 0x50;
	callFunc((void *) &outputChar);
	// pop rax	
	program[program_ptr++] = 0x58;
}

void BrainfuckCompiler::callInputChar(void) {
	callFunc((void *) &inputChar);
	// mov [ebx+ecx], al
	program[program_ptr++] = 0x67;
	program[program_ptr++] = 0x88;
	program[program_ptr++] = 0x04;
	program[program_ptr++] = 0x0B;
}

void BrainfuckCompiler::callFunc(void *ptr) {
	unsigned long value = (unsigned long) ptr;

	program[program_ptr++] = 0x48; // mov rax, value
	program[program_ptr++] = 0xB8; 
	program[program_ptr++] = value & 0xFF;
	program[program_ptr++] = (value>>8) & 0xFF;
	program[program_ptr++] = (value>>16) & 0xFF;
	program[program_ptr++] = (value>>24) & 0xFF;
	program[program_ptr++] = (value>>32) & 0xFF;
	program[program_ptr++] = (value>>40) & 0xFF;
	program[program_ptr++] = (value>>48) & 0xFF;
	program[program_ptr++] = (value>>54) & 0xFF;
	program[program_ptr++] = 0xff; // call [rax]
	program[program_ptr++] = 0xd0;
}

void BrainfuckCompiler::outputChar(char c) {	
	std::cout.put(c);
	std::cout.flush();
}

char BrainfuckCompiler::inputChar(void) {
	return std::cin.get();
}

void BrainfuckCompiler::ret(void) {	

	program[program_ptr++] = 0xc9; // ret
	program[program_ptr++] = 0xc3; // ret

}

}
