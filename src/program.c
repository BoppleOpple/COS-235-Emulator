#include "program.h"
#include "assembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PROGRAM *addProgram(const char *path, int *memory, LIST *programMemory) {

	// first, assemble the file
	LIST *machineCodeList = assembleFile(path);

	// if any error occurs, exit before writing to memory
	if (!machineCodeList) {
		printf("error assembling program\n");
		return NULL;
	}

	// the new program object to store data associated with the block of code
	PROGRAM *programContainer = malloc(sizeof(PROGRAM));

	// set the program name to the path
	programContainer->name = malloc(sizeof(char) * strlen(path));
	strcpy(programContainer->name, path);

	// insert the program starting at the next available address
	if (programMemory->size > 0) {
		PROGRAM *lastProgram = (PROGRAM *) listGetElement(programMemory, programMemory->size-1);

		programContainer->startAddress = lastProgram->startAddress + lastProgram->size;
	} else {
		programContainer->startAddress = 0;
	}

	programContainer->size = machineCodeList->size;

	// once all fields are set, document the program data
	listAppendItem(programMemory, programContainer);

	// and copy its code to memory
	for (int i = 0; i < programContainer->size; i++) {
		*(memory + programContainer->startAddress + i) = *(int *) listGetElement(machineCodeList, i);
	}

	// return a redundant pointer to the newest program
	// TODO: make this a weak pointer or double pointer
	return programContainer;
}