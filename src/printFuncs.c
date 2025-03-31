#include "printFuncs.h"
#include "assembler.h"
#include "list.h"
#include "program.h"
#include <stdio.h>

void printInstruction(void *instruction) {
	printMachineCode(*(int*) instruction);
}

void printIntPointer(void *i) {
	printf("%i\n", *(int*) i);
}

void printProgramName(void *program) {
	printf("%s\n", ((PROGRAM*) program)->name);
}

void printLabelRefs(void *reference) {
	printf("  - %i\n", *(int*) reference);
}

void printLabelData(void *label) {
	LABEL_DATA *cast = (LABEL_DATA *) label;
	printf("Label \"%s\":\n- id: %i\n- address: %i\n- references:\n", cast->name, cast->id, cast->address);
	listMapFunction(&cast->references, *printLabelRefs);
}