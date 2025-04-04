#include "program.h"
#include "assembler.h"
#include "stringUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PROGRAM *addProgram(const char *path, int *memory, LIST *programMemory) {
	int startAddress = 0;
	// insert the program starting at the next available address
	if (programMemory->size > 0) {
		PROGRAM *lastProgram = (PROGRAM *) listGetElement(programMemory, programMemory->size-1);

		startAddress = lastProgram->startAddress + lastProgram->size;
	}

	// first, assemble the file
	LIST *machineCodeList = assembleFile(path, startAddress);

	// if any error occurs, exit before writing to memory
	if (!machineCodeList) {
		printf("error assembling program\n");
		return NULL;
	}

	// the new program object to store data associated with the block of code
	PROGRAM *programContainer = malloc(sizeof(PROGRAM));

	int filenameIndex = lastIndexInString(path, '/') + 1;
	int extensionIndex = filenameIndex + lastIndexInString(path + filenameIndex, '.');

	// set the program name to the path
	programContainer->name = slice(path, filenameIndex, extensionIndex);
	programContainer->size = machineCodeList->size;
	programContainer->startAddress = startAddress;

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