#include "assembler.h"
#include "list.h"
#include "stringUtils.h"
#include <stdio.h>
#include <string.h>

#define WORD_SIZE_BYTES 4
#define MEMORY_SIZE_KB 16
#define MEMORY_SIZE_BYTES ( 1 << 10 ) * MEMORY_SIZE_KB
#define MEMORY_SIZE_WORDS ( MEMORY_SIZE_BYTES / WORD_SIZE_BYTES )

#define REGISTER_COUNT 32

int memory[MEMORY_SIZE_WORDS];
int registers[REGISTER_COUNT];

void resetMemory() {
	for (int i = 0; i < MEMORY_SIZE_WORDS; i++) memory[i] = 0;
}
void resetRegisters() {
	// set all registers to 0
	for (int i = 0; i < REGISTER_COUNT; i++) registers[i] = 0;

	// set the stack pointer to the end of memory
	registers[2] = MEMORY_SIZE_WORDS;
}

void printInstruction(void *instruction) {
	printMachineCode(*(int*) instruction);
}

int main() {
	resetMemory();

	printf("bytes of memory: %i\n", MEMORY_SIZE_BYTES);
	printf("words of memory: %i\n", MEMORY_SIZE_WORDS);

	LIST *machineCode = assembleFile("./res/testFile.txt");
	printMachineCode(*(unsigned int*) listGetElement(machineCode, 2));
	printMachineCode(*(unsigned int*) listGetElement(machineCode, 1));
	printMachineCode(*(unsigned int*) listGetElement(machineCode, 0));
	// listMapFunction(machineCode, *printInstruction);
	

	return 0;
}