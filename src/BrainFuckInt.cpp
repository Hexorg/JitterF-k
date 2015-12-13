#include "BrainFuckInt.h"

namespace Brainfuck {

BrainfuckInterpreter::BrainfuckInterpreter(std::istream &in, std::ostream &out): in(in), out(out) { 
	memory = new unsigned char [MEMORY_SIZE];
}

BrainfuckInterpreter::BrainfuckInterpreter(): in(std::cin), out(std::cout) {
	memory = new unsigned char [MEMORY_SIZE];
}

BrainfuckInterpreter::~BrainfuckInterpreter() {
	delete[] memory;
}

int BrainfuckInterpreter::run(const char *code) {
	instruction_ptr = code;
	memory_ptr = 0;
	memset(memory, 0, MEMORY_SIZE);
	return bf();
}

int BrainfuckInterpreter::bf() {
	const char *loop_start;
	int result;
	unsigned int loop_counter;
	while (*instruction_ptr != '\0') {
		switch (*instruction_ptr) {
			case '+': ++memory[memory_ptr]; break;
			case '-': --memory[memory_ptr]; break;
			case '>': if (++memory_ptr >= MEMORY_SIZE) return OUT_OF_MEMORY; break;
			case '<': if (--memory_ptr >= MEMORY_SIZE) return OUT_OF_MEMORY; break;
			case '.': out.put(memory[memory_ptr]); break;
			case ',': memory[memory_ptr] = (unsigned char) in.get(); break;
			case '[': loop_start = ++instruction_ptr;
				  loop_counter = 0;
				  while (memory[memory_ptr] != 0) {
					instruction_ptr = loop_start;
					if ((result = bf()) != SUCCESS)
						return result;
					if (++loop_counter > LOOP_MAX_ITERATION)
						return INFINITE_LOOP; // this will mark some good code as broken, 
								      // but catches all infinite loops
				  }
				  break;
			case ']': return SUCCESS; break;
		}
		++instruction_ptr;
	}
}

}
