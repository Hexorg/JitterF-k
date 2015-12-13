#ifndef __BRAINFUCKINT_H__
#define __BRAINFUCKINT_H__

#include <iostream>
#include <cstdlib>
#include <cstring>

namespace Brainfuck {

#define SUCCESS 0
#define OUT_OF_MEMORY -1
#define INFINITE_LOOP -2

	class BrainfuckInterpreter {
		public:
			BrainfuckInterpreter(std::istream &in, std::ostream &out);
			BrainfuckInterpreter();
			~BrainfuckInterpreter();
			int run(const char *code);
		private:
			int bf();
			const char *instruction_ptr;
			unsigned char *memory;
			unsigned int memory_ptr;
			static const unsigned int MEMORY_SIZE = 4096;
			static const unsigned int LOOP_MAX_ITERATION = 256;
			std::istream &in;
			std::ostream &out;

	};
}
#endif
