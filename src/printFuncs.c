#include "printFuncs.h"
#include "assembler.h"
#include "program.h"
#include <stdio.h>

void printInstruction(void *instruction) {
	printMachineCode(*(int*) instruction);
}

void printIntPointer(void *i) {
	printf("%i\n", *(int*) i);
}

void printProgramName(void *i) {
	printf("%s\n", ((PROGRAM*) i)->name);
}
