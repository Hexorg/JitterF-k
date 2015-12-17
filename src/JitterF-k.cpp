#include "BrainFuckInt.h"
#include "BrainFuckCmp.h"

#include <iostream>
int main() {
	char code[] = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";
	
	//Brainfuck::BrainfuckInterpreter bf;
	//bf.run(code);

	//char code[] = ".";
	Brainfuck::BrainfuckCompiler bfc;
	bfc.compile(code);
	std::cout << "Result: '";
	bfc.run();
	std::cout << "'" << std::endl;
}

