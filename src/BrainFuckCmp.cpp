#include "BrainFuckCmp.h"
#include <sys/mman.h>
#include <fstream>

namespace Brainfuck {

BrainfuckCompiler::BrainfuckCompiler(std::istream &in, std::ostream &out): in(in), out(out) { 
	
}

BrainfuckCompiler::BrainfuckCompiler(): in(std::cin), out(std::cout) {

}

void BrainfuckCompiler::init(void) {
	memory = new unsigned char [MEMORY_SIZE];

	program = (char *) mmap(NULL, MEMORY_SIZE, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (program == MAP_FAILED) {
		out << "Can't allocate space for program" << std::endl;
		return -1;
	}
	bf_program = (int(*)(void)) program;
	program_ptr = 0;
	stack_ptr = 0;

	// copy function init code
	for (i=0; i<sizeof(BF_START); ++i, ++program_ptr) {
		program[program_ptr] = BF_START[i];
	}
	// copy the memory address
	for (i=0; i<sizeof(&memory); ++i, ++program_ptr) {
		program[ptrogram_ptr] = ((char *)(&memory))[i];
	}
	// copy the code to call outputChar()
	for (i=0; i<sizeof(OUTPUT_CHAR_CODE); ++i) {
		output_char_code[i] = OUTPUT_CHAR_CODE[i];
	}
	// copy the location to outputChar()
	for (i=OUTPUT_CHAR_INSERT_PTR; i<sizeof(&outputChar); ++i) {
		output_char_code[i] = ((char *)(&outputChar))[i-OUTPUT_CHAR_INSERT_PTR];
	}
	// copy the code to call inputChar()
	for (i=0; i<sizeof(INPUT_CHAR_CODE); ++i) {
		input_char_code[i] = INPUT_CHAR_CODE[i];
	}
	// copy the location to inputChar()
	for (i=INPUT_CHAR_INSERT_PTR; i<sizeof(&inputChar); ++i) {
		input_char_code[i] = ((char *)(&outputChar))[i-INPUT_CHAR_INSERT_PTR];
	}

}

BrainfuckCompiler::~BrainfuckCompiler() {
	delete[] memory;
	munmap(program, MEMORY_SIZE);
}

int BrainfuckCompiler::run() {
	if (program_ptr != 0) {
		return (*bf_program)();
	}	
}

int BrainfuckCompiler::compile(const char *code) {
	const char *instruction = code;
	unsigned int tmp, tmp2;
	// compile
	/* BF ASM
	 * ebx = memory offset
	 * ecx = memory start
	 * eax = contains [eax+ecx] before entering loop
	 */
	//initFunction();
	//setOffset(0);
	//setMemStart(memory);
	while (*instruction != '\0') {
		switch (*instruction) {
			case '+': program[program_ptr++] = 0xfe;
					  program[program_ptr++] = 0x04;
					  program[program_ptr++] = 0x0b; break;
			case '-': program[program_ptr++] = 0xfe;
					  program[program_ptr++] = 0x0c;
					  program[program_ptr++] = 0x0b; break;
			case '>': program[program_ptr++] = 0xff;
					  program[program_ptr++] = 0xc3; break;
			case '<': program[program_ptr++] = 0xff;
					  program[program_ptr++] = 0xcb; break;
			case '[': loop_stack[stack_ptr++] = program_ptr;
					  for (i=0; i<sizeof(START_LOOP_CODE); ++i, ++program_ptr) {
						  program[program_ptr] = START_LOOP_CODE[i];
					  } break;
			case ']': stack_ptr--;
					  if (stack_ptr < 0)
						 return -1;
					  tmp = loop_stack[stack_ptr] - program_ptr - 5;
					  program[program_ptr++] = 0xe9; 
					  for (i=0; i<sizeof(tmp); ++i, ++program_ptr) {
						  program[program_ptr] = ((char *)(&tmp))[i];
					  }
					  tmp2 = program_ptr - loop_stack[stack_ptr] - START_LOOP_INSERT_PTR - 4;
					  for (i=0, tmp=loop_stack[stack_ptr]+START_LOOP_INSERT_PTR; i<sizeof(tmp); ++i, ++tmp) {
						  program[tmp] = ((char *)(&tmp))[i];
					  }
					  break;
			case '.': for (i=0; i<sizeof(output_char_code); ++i, ++program_ptr) {
					  	  program[program_ptr] = output_char_code[i];
					  } break;
			case ',': for (i=0; i<sizeof(input_char_code); ++i, ++program_ptr) {
						  program[program_ptr] = input_char_code[i];
					  } break;
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
	/**/ program[program_ptr++] = 0x55; // push rbp
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
	// push rbx
	/**/ program[program_ptr++] = 0x53;
	// push rcx
	/**/ program[program_ptr++] = 0x51;
	// mov rdi, [rbx+rcx]
	program[program_ptr++] = 0x48;
	program[program_ptr++] = 0x8b;
	program[program_ptr++] = 0x3c;
	program[program_ptr++] = 0x0b;

	callFunc((void *) &outputChar);
	// pop rcx	
	/**/ program[program_ptr++] = 0x59;
	// pop rbx	
	/**/ program[program_ptr++] = 0x5B;
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
}

char BrainfuckCompiler::inputChar(void) {
	return std::cin.get();
}

void BrainfuckCompiler::ret(void) {	

	/**/ program[program_ptr++] = 0x5d; // pop RBP
	program[program_ptr++] = 0xc3; // ret

}

}
