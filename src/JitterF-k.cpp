#include "BrainFuckInt.h"
#include "BrainFuckCmp.h"

#include <iostream>
int main() {
	char code[] = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";
	//Brainfuck::BrainfuckInterpreter bf;
	//bf.run(code);

	Brainfuck::BrainfuckCompiler bfc;
	bfc.compile(code);
	std::cout << "it compiled!" << std::endl;
	bfc.run();
}

