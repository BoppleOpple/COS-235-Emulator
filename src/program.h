#pragma once

#include "list.h"

typedef struct {
	char* name;
	int* entryPoint;
	int size;
} PROGRAM;

PROGRAM *addProgram(const char*path, int *memory, LIST *programMemory);