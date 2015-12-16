#ifndef __BRAINFUCKCMP_H__
#define __BRAINFUCKCMP_H__

#include <iostream>
#include <cstdlib>
#include <cstring>

namespace Brainfuck {

#define SUCCESS 0
#define BAD_BRACKETS -1
#define INFINITE_LOOP -2

	class BrainfuckCompiler {
		public:
			BrainfuckCompiler(std::istream &in, std::ostream &out);
			BrainfuckCompiler();
			~BrainfuckCompiler();
			int compile(const char *code);
			int run();
		private:
			void setOffset(unsigned int value);
			void setMemStart(char *value);
			void incCell(void);
			void decCell(void);
			void incOffset(void);
			void decOffset(void);
			void startLoop(void);
			void endLoop(void);
			void callFunc(void *ptr);
			void ret(void);
			void 
			int (*bf_program)();
			char *program;
			unsigned int program_ptr;
			unsigned char *memory;
			unsigned int memory_ptr;
			static const unsigned int MEMORY_SIZE = 4096;
			std::istream &in;
			std::ostream &out;

	};
}
#endif
