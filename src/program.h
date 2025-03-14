#pragma once

#include "list.h"

typedef struct {
	char* name;
	int startAddress;
	int size;
} PROGRAM;

/**
 * @brief 
 * 
 * @param path a const c-string containing the path of the assembly code
 * @param memory a pointer to the int array representation of memory
 * @param programMemory a pointer to the list of currently loaded programs
 * @return PROGRAM* 
 */
PROGRAM *addProgram(const char *path, int *memory, LIST *programMemory);