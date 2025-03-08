#pragma once
#include "list.h"

#define NUM_OPCODES 10
#define ASSEMBLER_BUFFER_SIZE 1024

typedef struct {
	const int encoded;
	const char *decoded;
} OpCodeKVPair;

unsigned int assembleLine(const char *assembly);

LIST *assembleFile(const char *path);

void printBinary(unsigned int n);
void printMachineCode(unsigned int n);