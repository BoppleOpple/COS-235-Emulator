#include "program.h"
#include "assembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PROGRAM *addProgram(const char *path, int *memory, LIST *programMemory) {

	LIST *machineCodeList = assembleFile(path);

	if (!machineCodeList) {
		printf("error assembling program\n");
		return NULL;
	}

	PROGRAM *programContainer = malloc(sizeof(PROGRAM));

	programContainer->name = malloc(sizeof(char) * strlen(path));
	strcpy(programContainer->name, path);

	if (programMemory->size > 0) {
		PROGRAM *lastProgram = (PROGRAM *) listGetElement(programMemory, programMemory->size-1);

		programContainer->entryPoint = lastProgram->entryPoint + lastProgram->size;
	} else {
		programContainer->entryPoint = memory;
	}
	programContainer->size = machineCodeList->size;
	listAppendItem(programMemory, programContainer);

	for (int i = 0; i < programContainer->size; i++) {
		*(programContainer->entryPoint + i) = *(int *) listGetElement(machineCodeList, i);
	}

	return programContainer;
}