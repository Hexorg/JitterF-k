#ifndef __BRAINFUCKCMP_H__
#define __BRAINFUCKCMP_H__

#include <iostream>
#include <cstdlib>
#include <cstring>

namespace Brainfuck {

	class BrainfuckCompiler {
		public:
			BrainfuckCompiler(std::istream &in, std::ostream &out);
			BrainfuckCompiler();
			~BrainfuckCompiler();
			int compile(const char *code);
			int run();
		private:
			void init(void);

			void initFunction(void);
			void setOffset(unsigned int value);
			void setMemStart(void *ptr);
			void incCell(void);
			void decCell(void);
			void incOffset(void);
			void decOffset(void);
			void startLoop(void);
			void endLoop(void);
			void callFunc(void *ptr);
			void callOutputChar(void);
			void callInputChar(void);
			static void outputChar(char c);
			static char inputChar(void);
			void ret(void); 
			int (*bf_program)(void);
			char *program;
			unsigned int program_ptr;
			unsigned int i;
			unsigned char *memory;
			unsigned int memory_ptr;
			std::istream &in;
			std::ostream &out;
			struct LoopData {unsigned int loop_start; unsigned int loop_end_label;};
			static const unsigned int MEMORY_SIZE = 4096;
			static const int BAD_BRACKETS = -1;

			unsigned int loop_stack[128]; // max of 128 nested loops
			int stack_ptr;

			char output_char_code[sizeof(OUTPUT_CHAR_CODE)];
			char input_char_code[sizeof(INPUT_CHAR_CODE)];

			// push rbp
			// mov ebx, 0
			// mov rcx, [insert 8 bytes here]
			static const char BF_START = {0x55, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x48, 0xb9};
			static const char OUTPUT_CHAR_CODE = {0x53, 0x51, 0x48, 0x8b, 0x3c, 
				0x0b, 0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0xff, 0xd0, 0x59, 0x5b};
			static const unsigned int OUTPUT_CHAR_INSERT_PTR = 8;
			static const char INPUT_CHAR_CODE = {0x48, 0xb8, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0xff, 0xd0, 0x67, 0x88, 0x04, 0x0b};
			static const unsigned int INPUT_CHAR_INSERT_PTR = 2;

			static const char START_LOOP_CODE = { 0x8a, 0x04, 0x0b, 0x84, 0xc0, 0x0f, 0x84,
				0x00, 0x00, 0x00, 0x00};
			static const unsigned int START_LOOP_INSERT_PTR = 7;

	};
}
#endif
