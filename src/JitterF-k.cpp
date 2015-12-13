#include "BrainFuckInt.h"

int main() {
	char code[] = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";
	Brainfuck::BrainfuckInterpreter bf;
	return bf.run(code);
}

