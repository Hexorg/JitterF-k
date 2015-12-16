#include "BrainFuckCmp.h"

namespace Brainfuck {

/* All of the machine code this JIT may need
 *
 * b8 44 43 42 41       	mov    $0x41424344,%eax
 * bb 44 43 42 41       	mov    $0x41424344,%ebx
 * b9 44 43 42 41       	mov    $0x41424344,%ecx
 * ba 44 43 42 41       	mov    $0x41424344,%edx
 * 48 ff c0             	inc    %rax
 * 48 ff c3             	inc    %rbx
 * 48 ff c1             	inc    %rcx
 * 48 ff c2             	inc    %rdx
 * 48 ff c8             	dec    %rax
 * 48 ff cb             	dec    %rbx
 * 48 ff c9             	dec    %rcx
 * 48 ff ca             	dec    %rdx
 * fe 04 03             	incb   (%rbx,%rax,1)
 * fe 0c 03             	decb   (%rbx,%rax,1)
 * 48 8b 0c 03          	mov    (%rbx,%rax,1),%rcx
 * 48 8b 14 03          	mov    (%rbx,%rax,1),%rdx
 * 48 85 c9             	test   %rcx,%rcx
 * 48 85 d2             	test   %rdx,%rdx
 * 0f 84 40 43 42 41    	je     0x41424386
 * 50                   	push   %rax
 * 58                   	pop    %rax
 * 53                   	push   %rbx
 * 5b                   	pop    %rbx
 * e9 40 43 42 41       	jmpq   0x4142438f
 * e8 50 53 52 51       	callq  0x515253a4
 * c3                   	retq
 */

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
	 * ecx = contains [eax+ecx] before entering loop
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
			case '[': startLoop(); break;
			case ']': endLoop(); break;
			case '.': outputChar(); break;
			case ',': inputChar(); break;
		}
	}
	ret();
}

void BrainfuckCompiler::setOffset(unsigned int value) {
	program[program_ptr++] = 0xb8; // mov eax, value
	program[program_ptr++] = value & 0xFF;
	program[program_ptr++] = (value>>8) & 0xFF;
	program[program_ptr++] = (value>>8) & 0xFF;
	program[program_ptr++] = (value>>8) & 0xFF;
}

void BrainfuckCompiler::setMemStart(unsigned int value) {
	program[program_ptr++] = 0xbb; // mov ebx, value
	program[program_ptr++] = value & 0xFF;
	program[program_ptr++] = (value>>8) & 0xFF;
	program[program_ptr++] = (value>>8) & 0xFF;
	program[program_ptr++] = (value>>8) & 0xFF;
}

void BrainfuckCompiler::incCell(void) {	
	program[program_ptr++] = 0xfe; // inc byte [rax+rbx]
	program[program_ptr++] = 0x04;
	program[program_ptr++] = 0x03;
}

void BrainfuckCompiler::decCell(void) {	
	program[program_ptr++] = 0xfe; // dec byte [rax+rbx]
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

void BrainfuckCompiler::startLoop(void) {
	struct LoopData data;
	data.loop_start = program_ptr;
	// save current program_ptr
	// mov ecx, [ebx+eax]
	program[program_ptr++] = 0x48;
	program[program_ptr++] = 0x8b;
	program[program_ptr++] = 0x0c;
	program[program_ptr++] = 0x03;
	// test ecx, ecx
	program[program_ptr++] = 0x48;
	program[program_ptr++] = 0x85;
	program[program_ptr++] = 0xC9;
	// jz	end_loop ; to be calucalted in endLoop()
	program[program_ptr++] = 0x0f;
	program[program_ptr++] = 0x80;
	program[program_ptr++] = 0x00;
	program[program_ptr++] = 0x00;
	program[program_ptr++] = 0x00;
	program[program_ptr++] = 0x00;
	data.loop_end_label = program_ptr;
	loopStack.push(data);
}

void BrainfuckCompiler::endLoop(void) {
	struct LoopData data = loopStack.pop();
	// overwrite jz address in startLoop
	// jmp start_loop
	program[program_ptr++] = 0xe9;
	program[program_ptr++] = 0x00;
	program[program_ptr++] = 0x00;
	program[program_ptr++] = 0x00;
	program[program_ptr++] = 0x00;

}

void BrainfuckCompiler::callFunc(void *ptr) {
	unsigned int value = (unsigned int) ptr;

	program[program_ptr++] = 0xe8; // call ptr
	program[program_ptr++] = value & 0xff;
	program[program_ptr++] = (value>>8) & 0xff;
	program[program_ptr++] = (value>>8) & 0xff;
	program[program_ptr++] = (value>>8) & 0xff;
}

void BrainfuckCompiler::ret(void) {	
	program[program_ptr++] = 0xc3; // ret
}

}
